#pragma once

BOOL GrantDebugPrivilege();

// Prints the thread wait chains for one or all processes in the system.
int listWCT(DWORD procId);

PCTSTR getProcessName(DWORD processId);
PCTSTR getProcessFile(DWORD processId);
PCSTR getProcessFileA(DWORD processId);
PCWSTR getProcessFileW(DWORD processId);
std::set<DWORD> getPossibleBlockingProcessIds(DWORD processId);
