
#include "stdafx.h"

#include "WCT.h"

// Using parts of WCT Sample Code =>

// Copyright (C) Microsoft. All rights reserved.

/*
* Sample code for the Wait Chain Traversal (WCT) API.
*
* This program enumerates all threads in the system and prints the
* wait chain for each of them.  It should be run from an elevated
* command prompt to get results for services.
*
*/

#include <wct.h>
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Advapi32.lib")


// Global variable to store the WCT session handle
HWCT g_WctHandle = NULL;

// Global variable to store OLE32.DLL module handle.
HMODULE g_Ole32Hnd = NULL;


BOOL
GrantDebugPrivilege()
/*++

Routine Description:

Enables the debug privilege (SE_DEBUG_NAME) for this process.
This is necessary if we want to retrieve wait chains for processes
not owned by the current user.

Arguments:

None.

Return Value:

TRUE if this privilege could be enabled; FALSE otherwise.

--*/
{
	BOOL             fSuccess    = FALSE;
	HANDLE           TokenHandle = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&TokenHandle))
	{
		printf("Could not get the process token");
		goto Cleanup;
	}

	TokenPrivileges.PrivilegeCount = 1;

	if (!LookupPrivilegeValue(NULL,
		SE_DEBUG_NAME,
		&TokenPrivileges.Privileges[0].Luid))
	{
		printf("Couldn't lookup SeDebugPrivilege name");
		goto Cleanup;
	}

	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(TokenHandle,
		FALSE,
		&TokenPrivileges,
		sizeof(TokenPrivileges),
		NULL,
		NULL))
	{
		printf("Could not revoke the debug privilege");
		goto Cleanup;
	}

	fSuccess = TRUE;

Cleanup:

	if (TokenHandle)
	{
		CloseHandle(TokenHandle);
	}

	return fSuccess;
}

BOOL
InitCOMAccess()
/*++

Routine Description:

Register COM interfaces with WCT. This enables WCT to provide wait
information if a thread is blocked on a COM call.

--*/
{
	PCOGETCALLSTATE       CallStateCallback;
	PCOGETACTIVATIONSTATE ActivationStateCallback;

	// Get a handle to OLE32.DLL. You must keep this handle around
	// for the life time for any WCT session.
	g_Ole32Hnd = LoadLibrary(_T("ole32.dll"));
	if (!g_Ole32Hnd)
	{
		printf("ERROR: GetModuleHandle failed: 0x%X\n", GetLastError());
		return FALSE;
	}

	// Retrieve the function addresses for the COM helper APIs.
	CallStateCallback = (PCOGETCALLSTATE)
		GetProcAddress(g_Ole32Hnd, "CoGetCallState");
	if (!CallStateCallback)
	{
		printf("ERROR: GetProcAddress failed: 0x%X\n", GetLastError());
		return FALSE;
	}

	ActivationStateCallback = (PCOGETACTIVATIONSTATE)
		GetProcAddress(g_Ole32Hnd, "CoGetActivationState");
	if (!ActivationStateCallback)
	{
		printf("ERROR: GetProcAddress failed: 0x%X\n", GetLastError());
		return FALSE;
	}

	// Register these functions with WCT.
	RegisterWaitChainCOMCallback(CallStateCallback,
								 ActivationStateCallback);
	return TRUE;
}

// <= End of sample code


// get full executable path
PCTSTR getProcessFile(DWORD processId) {
	PTSTR file;

	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (process)
	{
		// get a string buffer
		file = cycleStrBuffer();

		if (GetProcessImageFileName(process, file, cycleStrBufLen) == 0)
		{
			file = TEXT("<UNKNOWN>");
		}

		CloseHandle(process);
	} else {
		file = TEXT("<UNKNOWN>");
	}

	return file;
}

PCSTR getProcessFileA(DWORD processId) {
	PSTR file;

	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (process)
	{
		// get a string buffer
		file = cycleStrBufferA();

		if (GetProcessImageFileNameA(process, file, cycleStrBufLen) == 0)
		{
			file = "<UNKNOWN>";
		}

		CloseHandle(process);
	} else {
		file = "<UNKNOWN>";
	}

	return file;
}

PCWSTR getProcessFileW(DWORD processId) {

	PWSTR file;

	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
	if (process)
	{
		// get a string buffer
		file = cycleStrBufferW();

		if (GetProcessImageFileNameW(process, file, cycleStrBufLen) == 0)
		{
			file = L"<UNKNOWN>";
		}

		CloseHandle(process);
	} else {
		file = L"<UNKNOWN>";
	}

	return file;
}

// just get the executable
PCTSTR getProcessName(DWORD processId) {
	PCTSTR file = getProcessFile(processId);

	PCTSTR filePart = _tcsrchr(file, TEXT('\\'));
	if (filePart)
	{
		file = filePart + 1;
	}

	return file;
}

// Fill the blickingIds vector with process IDs that may be blocking processId (the process we don't want to be blocked anymore)
// uses parts of WCT sample code as template
void getPossibleBlockingProcessIdsFromThread(DWORD processId, DWORD threadId, std::set<DWORD>& blockingIds)
{
	WAITCHAIN_NODE_INFO NodeInfoArray[WCT_MAX_NODE_COUNT];
	DWORD               Count, i;
	BOOL                IsCycle;
	
	Count = WCT_MAX_NODE_COUNT;

	// Make a synchronous WCT call to retrieve the wait chain.
	if (!GetThreadWaitChain(g_WctHandle,
		NULL,
		WCTP_GETINFO_ALL_FLAGS,
		threadId,
		&Count,
		NodeInfoArray,
		&IsCycle))
	{
		std::wcout << "Error (0x" << GetLastError() << std::hex << ")" << std::endl;
		return;
	}

	// Check if the wait chain is too big for the array we passed in.
	if (Count > WCT_MAX_NODE_COUNT)
	{
		std::wcout << "Found additional nodes " << Count << std::endl;
		Count = WCT_MAX_NODE_COUNT;
	}


	bool process = false;
	bool blocked = false;

	// Loop over all the nodes returned and print useful information.
	for (i = 0; i < Count; i++)
	{
		switch (NodeInfoArray[i].ObjectType)
		{
		case WctThreadType:
			// A thread node contains process and thread ID.
			if (NodeInfoArray[i].ThreadObject.ProcessId == processId) {
				process = true;
			} else if (process && blocked) {
				blockingIds.insert( NodeInfoArray[i].ThreadObject.ProcessId );
			}
			blocked = NodeInfoArray[i].ObjectStatus == WctStatusBlocked;
			break;
		default:
			break;
		}
	}
}

// Search all threads of the process with processId for blocking processes
// uses parts of WCT sample code as a template
std::set<DWORD> getPossibleBlockingProcessIds(DWORD processId) {
	std::set<DWORD> blockingIds;

	// Initialize the WCT interface to COM. Continue if this
	// fails--there just will not be COM information.
	if (!InitCOMAccess())
	{
		printf("Could not enable COM access\n");
	}

	// Try to enable the SE_DEBUG_NAME privilege for this process. 
	if (!GrantDebugPrivilege())
	{
		printf("Could not enable the debug privilege");
		goto Cleanup;
	}

	// Open a synchronous WCT session.
	g_WctHandle = OpenThreadWaitChainSession(0, NULL);
	if (NULL == g_WctHandle)
	{
		printf("ERROR: OpenThreadWaitChainSession failed\n");
		goto Cleanup;
	}

	// Get a snapshot of this process. This enables us to
	// enumerate its threads.
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
	if (snapshot)
	{
		THREADENTRY32 thread;
		thread.dwSize = sizeof(thread);

		// Walk the thread list and print each wait chain
		if (Thread32First(snapshot, &thread))
		{
			do
			{
				if (thread.th32OwnerProcessID == processId)
				{
					// Open a handle to this specific thread
					HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS,
														FALSE,
														thread.th32ThreadID);
					if (threadHandle)
					{
						// Check whether the thread is still running
						DWORD exitCode;
						GetExitCodeThread(threadHandle, &exitCode);

						if (exitCode == STILL_ACTIVE)
						{
							// Print the wait chain.
							getPossibleBlockingProcessIdsFromThread(processId, thread.th32ThreadID, blockingIds);
						}

						CloseHandle(threadHandle);
					}
				}
			} while (Thread32Next(snapshot, &thread));
		}
		CloseHandle(snapshot);
	}

	// Close the WCT session.
	CloseThreadWaitChainSession(g_WctHandle);

Cleanup:
	if (NULL != g_Ole32Hnd)
	{
		FreeLibrary(g_Ole32Hnd);
	}

	return blockingIds;
}