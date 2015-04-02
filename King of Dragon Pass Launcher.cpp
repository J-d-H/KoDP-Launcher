// King of Dragon Pass Launcher.cpp
//

#include "stdafx.h"

DWORD WINAPI hookMessageLoopWithFancyWindowMovingToFixMouseClicksNotDoAnythingEvenThoughTheyToggleButtonDownEffectsInGame(LPVOID);

HINSTANCE g_hInstance;
PROCESS_INFORMATION g_processInfo;
HWND g_hwndMain = NULL;
HWND g_hwndChild = NULL;
HWND g_hwndRender = NULL;
HWND g_hwndDesktop = NULL;
HDC  g_hdcDesktop = NULL;
HDC  g_hdcRender = NULL;


struct MouseScale {
	int offX;
	int offY;
	double scale;
};
MouseScale scaleGame(HWND hSource, HWND hDest);
LRESULT CALLBACK MainWndProc(
	HWND hwnd,        // handle to window
	UINT uMsg,        // message identifier
	WPARAM wParam,    // first message parameter
	LPARAM lParam     // second message parameter
);

BOOL CALLBACK ecwCallback(HWND hwnd, LPARAM lParam) {
	auto cn = cycleStrBuffer();
	GetClassName(hwnd, cn, cycleStrBufLen);
	auto text = cycleStrBuffer();
	GetWindowText(hwnd, text, cycleStrBufLen);

	if (_tcscmp(text, TEXT("mPlayer")) == 0) {
		g_hwndChild = hwnd;
		auto style = GetWindowLongW(hwnd, GWL_STYLE);
		bool caption = (style & WS_CAPTION) == WS_CAPTION;
		bool child = (style & WS_CHILDWINDOW) == WS_CHILDWINDOW;
		bool overlapped = (style & WS_OVERLAPPED) == WS_OVERLAPPED;
		bool overlappedWnd = (style & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW;
		style = style & ~(WS_OVERLAPPEDWINDOW | WS_CAPTION);
		caption = (style & WS_CAPTION) == WS_CAPTION;
		child = (style & WS_CHILDWINDOW) == WS_CHILDWINDOW;
		overlapped = (style & WS_OVERLAPPED) == WS_OVERLAPPED;
		overlappedWnd = (style & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW;
		SetWindowLong(hwnd, GWL_STYLE, style & ~WS_CAPTION);
		//SetWindowPos(hwnd, NULL, 0, 0, 640, 480, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		MoveWindow(hwnd, 0, 0, 800, 600, 1);
	}
	else if (_tcscmp(cn, TEXT("AfxFrameOrView40s")) == 0) {
		g_hwndRender = hwnd;
		MoveWindow(hwnd, 0, 0, 640, 480, 1);
		//return false;
	}
	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	DWORD processId = 0;
	DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);
	if (processId == g_processInfo.dwProcessId) {
		auto text = cycleStrBuffer();
		if (GetWindowText(hwnd, text, cycleStrBufLen)) {
			if (_tcscmp(text, TEXT("mTropolis Windows Player - mPlayer")) == 0) {
				g_hwndMain = hwnd;
				return false;
			}
		}
	}
	return true;
}

bool searchGameWindow() {
	std::wcout << "searching game window..." << std::endl;

	EnumWindows(EnumWindowsProc, NULL);

	return g_hwndMain != NULL;
}



HWND CreateFullscreenWindow(HWND hwnd, bool asChild = false) {
	WNDCLASS wc;
	auto wndClassName = TEXT("KoDP-ScalingWnd");

	// Register the main window class. 
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hInstance;
	//wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIcon = NULL; //LoadIcon(NULL, "gfw_high.ico");
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = wndClassName;

	if (!RegisterClass(&wc)) {
		auto err = GetLastError();
		std::wcout << "ERROR: Couldn't register Window Class: 0x" << std::hex << err << std::endl;
		std::wcout << GetErrorText(err) << std::endl;
		return NULL;
	}

	HMONITOR hmon = MonitorFromWindow(hwnd,
									  MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi ={sizeof(mi)};
	if (!GetMonitorInfo(hmon, &mi)) return NULL;
	return CreateWindow(wndClassName,
						wndClassName,
						WS_POPUP | WS_VISIBLE,
						mi.rcMonitor.left,
						mi.rcMonitor.top,
						mi.rcMonitor.right - mi.rcMonitor.left,
						mi.rcMonitor.bottom - mi.rcMonitor.top,
						(asChild ? hwnd : NULL), NULL, g_hInstance, 0);
}

struct SavedWndStyle {
	DWORD style;
	RECT pos;
};
void toggleFullScreen(HWND hwnd, bool isChild = false) {
	static std::map<HWND, SavedWndStyle> saved;

	auto text = cycleStrBuffer();
	int bText = GetWindowText(hwnd, text, cycleStrBufLen);

	auto s = saved.find(hwnd);
	if (s == saved.end()) {
		// make fullscreen:
		SavedWndStyle sws;
		sws.style = GetWindowLong(hwnd, GWL_STYLE);
		GetWindowRect(hwnd, &sws.pos);
				
		HMONITOR hmon = MonitorFromWindow(hwnd,
										  MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi ={sizeof(mi)};
		if (!GetMonitorInfo(hmon, &mi)) return;

		SetWindowLong(hwnd, GWL_STYLE, (isChild ? WS_CHILD : WS_POPUP) | WS_VISIBLE);
		SetWindowPos(hwnd, HWND_TOP,
					 mi.rcMonitor.left,
					 mi.rcMonitor.top,
					 mi.rcMonitor.right - mi.rcMonitor.left,
					 mi.rcMonitor.bottom - mi.rcMonitor.top, 
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		saved[hwnd] = sws;
	} else {
		SetWindowLong(hwnd, GWL_STYLE, s->second.style);
		SetWindowPos(hwnd, HWND_TOP,
					 s->second.pos.left,
					 s->second.pos.top,
					 s->second.pos.right - s->second.pos.left,
					 s->second.pos.bottom - s->second.pos.top,
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		saved.erase(s);
	}

	if (bText) {
		SetWindowText(hwnd, text);
	}
}


int __cdecl _tmain(int argc, _TCHAR* argv[])
{
	DWORD idThread = 0; // Message Loop Thread id
	HANDLE hThread = NULL; // Message Loop Thread handle

	g_hInstance = GetModuleHandle(NULL);
	STARTUPINFOA info ={sizeof(STARTUPINFOA)};

	{
		// setting codepage
		if (!SetConsoleCP(1252) || !SetConsoleOutputCP(1252)) {
			auto err = GetLastError();
			std::wcout << "Error setting CP1252: 0x" << std::hex << err << "\n" << GetErrorText(err) << std::endl;
		}

		std::locale::global(std::locale("english_England.1252"));
		std::wcout.imbue(std::locale());
		

		// Seting console font
		auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
		//GetCurrentConsoleFontEx(hStdout, false, &cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 0;
		cfi.dwFontSize.Y = 16;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Consolas");
		//wcscpy_s(cfi.FaceName, L"Arial");
		if (!SetCurrentConsoleFontEx(hStdout, false, &cfi)) {
			auto err = GetLastError();
			std::wcout << "Error settig console font: 0x" << std::hex << err << "\n" << GetErrorText(err) << std::endl;
		}
	}

	std::wcout << "Launching King of Dragon Path..." << std::endl;

	if (CreateProcessA(NULL, "KoDP.exe -w", NULL, NULL, TRUE, 0, NULL, NULL, &info, &g_processInfo))
	{
		Sleep(500);
		while (!searchGameWindow()) {
			std::wcout << "Window not found. This version does not search for blocking processes ..." << std::endl;

			Sleep(1000);
		}

		auto windowTitle = cycleStrBuffer();
		GetWindowText(g_hwndMain, windowTitle, cycleStrBufLen);
		std::wcout << "\nFound window (" << windowTitle << "). Fixing borders..." << std::endl;

		// Remove pointless menu
		SetMenu(g_hwndMain, NULL);
		// Add window decoration
		auto style = GetWindowLong(g_hwndMain, GWL_STYLE);
		SetWindowLong(g_hwndMain, GWL_STYLE, style | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		SetWindowPos(g_hwndMain, HWND_TOP, 16, 12, 670, 530, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		// Find the innermost window which actually draws the game
		EnumChildWindows(g_hwndMain, ecwCallback, 0);
		// Fix Window Title
		SetWindowText(g_hwndMain, TEXT("King of Dragon Pass"));

		toggleFullScreen(g_hwndMain);
		toggleFullScreen(g_hwndChild, true);
		//toggleFullScreen(g_hwndRender, true);
		//MoveWindow(g_hwndRender, 0, 0, 640, 480, FALSE);

		// Scaling init
		hThread = CreateThread(
			NULL,        // default security attributes
			0,           // use default stack size  
			hookMessageLoopWithFancyWindowMovingToFixMouseClicksNotDoAnythingEvenThoughTheyToggleButtonDownEffectsInGame,
			NULL,        // argument to thread function 
			0,           // use default creation flags 
			&idThread);  // returns the thread identifier

		DWORD waitResult;
		if (hThread) {
			waitResult = ::WaitForSingleObject(hThread, 2500);
		}
		if (hThread && waitResult == WAIT_FAILED) {
			ShowWindow(GetConsoleWindow(), SW_SHOW);
			auto err = GetLastError();
			std::wcout << "Error in hook message thread: " << err << std::endl;
			std::wcout << GetErrorText(err) << std::endl;
			std::wcout << "Terminating ... ";

			TerminateProcess(g_processInfo.hProcess, 0);
		}

		::WaitForSingleObject(g_processInfo.hProcess, INFINITE);
		std::wcout << "finished." << std::endl;

		ShowWindow(GetConsoleWindow(), SW_SHOW);

		if (hThread) {
			CloseHandle(hThread);
			hThread = NULL;
		}

		if (g_hdcRender) {
			ReleaseDC(g_hwndRender, g_hdcRender);
		}
		if (g_hdcDesktop) {
			ReleaseDC(g_hwndDesktop, g_hdcDesktop);
		}

		CloseHandle(g_processInfo.hThread);
		CloseHandle(g_processInfo.hProcess);
	} else {
		auto err = GetLastError();
		std::wcout << "stating King of Dragon Path failed: " << std::hex << err << std::endl;
		std::wcout << GetErrorText(err);
	}

	Sleep(5000);
	return 0;
}

DWORD WINAPI hookMessageLoopWithFancyWindowMovingToFixMouseClicksNotDoAnythingEvenThoughTheyToggleButtonDownEffectsInGame(LPVOID lpThreadParameter) {
	std::wcout << "Initializing scaler..." << std::endl;

	HWND hDest = CreateFullscreenWindow(g_hwndChild, true);
	//MoveWindow(hDest, 700, 200, 640, 480, TRUE);
	//ShowWindow(hDest, SW_SHOW);
	//UpdateWindow(hDest);
	std::wcout << "\nWaiting for King of Dragon Pass to finish... ";
	DWORD exitCode;
	// Message LOOP
	MSG msg;
	MouseScale ms = scaleGame(g_hwndRender, hDest);
	while (true)
	{
		BOOL bRet;

		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
			if (!GetExitCodeProcess(g_processInfo.hProcess, &exitCode) || (exitCode != STILL_ACTIVE))
			{
				// well the game was quit (or crashed?)
				break;
			}
			else
			{
				if (WM_KEYFIRST <= msg.message && msg.message <= WM_KEYLAST) {
					//SendMessage(g_hwndMain, msg.message, msg.wParam, msg.lParam);
					//SendMessage(g_hwndChild, msg.message, msg.wParam, msg.lParam);
					SendMessage(g_hwndRender, msg.message, msg.wParam, msg.lParam);
				} else if (WM_MOUSEFIRST <= msg.message && msg.message <= WM_MOUSELAST) {
					int mPosX = GET_X_LPARAM(msg.lParam);
					int mPosY = GET_Y_LPARAM(msg.lParam);
					int xPos = (int)((mPosX - ms.offX) / ms.scale);
					int yPos = (int)((mPosY - ms.offY) / ms.scale);
					// Move the game window, because the mouse click doesn't work otherwise...
					MoveWindow(g_hwndRender, mPosX - xPos, mPosY - yPos, 640, 480, TRUE);
					SendMessage(g_hwndRender, msg.message, msg.wParam, MAKELPARAM(xPos, yPos));//*/
					//SendMessage(g_hwndChild, msg.message, msg.wParam, MAKELPARAM(xPos, yPos));
				} else if (msg.message == WM_PAINT) {
					//PostMessage(g_hwndMain, msg.message, msg.wParam, msg.lParam);
					ms = scaleGame(g_hwndRender, hDest);
				} else {
					SendMessage(g_hwndMain, msg.message, msg.wParam, msg.lParam);
					//SendMessage(g_hwndRender, msg.message, msg.wParam, msg.lParam);
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Sleep(1);
		if (!GetExitCodeProcess(g_processInfo.hProcess, &exitCode) || (exitCode != STILL_ACTIVE))
		{
			// well the game was quit (or crashed?)
			break;
		}
		//ms = scaleGame(g_hwndRender, hDest);
	}
	DestroyWindow(hDest);

	return 0;
}

LRESULT CALLBACK MainWndProc(
	HWND hwnd,        // handle to window
	UINT uMsg,        // message identifier
	WPARAM wParam,    // first message parameter
	LPARAM lParam)    // second message parameter
{
	static MouseScale ms;
	switch (uMsg)
	{
	case WM_CREATE:
		// Initialize the window. 
		return 0;

	case WM_DESTROY:
		// Clean up window-specific data objects. 
		return 0;

	case WM_PAINT:
		return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

MouseScale scaleGame(HWND hSource, HWND hDest)
{
	HDC hdcSource = GetDC(hSource);
	HDC hdcDest = GetDC(hDest);

	// Get the client area for size calculation
	RECT dstRect;
	RECT srcRect;
	GetClientRect(hSource, &srcRect);
	GetClientRect(hDest, &dstRect);

	const int sourceWidth = srcRect.right - srcRect.left;
	const int sourceHeight = srcRect.bottom - srcRect.top;
	const int destWidth = dstRect.right - dstRect.left - 10;
	const int destHeight = dstRect.bottom - dstRect.top - 10;

	const double aw = sourceWidth / (double)sourceHeight;
	const double as = destWidth / (double)destHeight;

	int dtop;
	int dleft;
	int dwidth;
	int dheight;

	MouseScale r;

	if (as > aw) {
		r.scale = destHeight / (double)sourceHeight;
		dtop = 0;
		dheight = destHeight;
		dwidth = (int)(r.scale * sourceWidth);
		dleft = (int)(0.5 * (destWidth - dwidth));
	} else {
		r.scale = destWidth / (double)sourceWidth;
		dleft = 0;
		dwidth = destWidth;
		dheight = (int)(r.scale * sourceHeight);
		dtop = (int)(0.5 * (destHeight - dheight));
	}
	r.offX = dleft;
	r.offY = dtop;

	//This is the best stretch mode
	SetStretchBltMode(hdcDest, HALFTONE);

	//The source DC is the entire screen and the destination DC is the current window (HWND)
	StretchBlt(hdcDest,
			   dleft, dtop,
			   dwidth, dheight,
			   //hdcWindow,
			   hdcSource,
			   srcRect.left, srcRect.top,
			   sourceWidth, sourceHeight,
			   SRCCOPY);

	//BitBlt(hdcScreen, dleft, dtop, dwidth, dheight, hdcMemDC, dleft, dtop, SRCCOPY);

	//DeleteObject(hdcMemDC);
	ReleaseDC(hSource, hdcSource);
	ReleaseDC(hDest, hdcDest);

	return r;
}