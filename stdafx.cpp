// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// King of Dragon Pass Launcher.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

PSTR cycleStrBufferA() {
	const size_t alen = 3;
	static char aStrBufs[alen][cycleStrBufLen];
	static size_t cur = 0;
	PSTR strbuffer = aStrBufs[cur];
	cur = (cur + 1) % alen;

	strbuffer[0] = '\0';
	return strbuffer;
}

PWSTR cycleStrBufferW() {
	const size_t alen = 3;
	static wchar_t aStrBufs[alen][cycleStrBufLen];
	static size_t cur = 0;
	PWSTR strbuffer = aStrBufs[cur];
	cur = (cur + 1) % alen;

	strbuffer[0] = L'\0';
	return strbuffer;
}

PTSTR cycleStrBuffer() {
	const size_t alen = 3;
	static TCHAR aStrBufs[alen][cycleStrBufLen];
	static size_t cur = 0;
	PTSTR strbuffer = aStrBufs[cur];
	cur = (cur + 1) % alen;

	strbuffer[0] = TEXT('\0');
	return strbuffer;
}

PCTSTR GetErrorText(DWORD nErrorCode)
{
	PTSTR msg = cycleStrBuffer();
	// Ask Windows to prepare a standard message for a GetLastError() code:
	auto count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, cycleStrBufLen, NULL);
	// Return the message
	if (!count)
		return TEXT("Unknown error");
	else
		return msg;
}
