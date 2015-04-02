// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <Windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>

#include <map>

#include <locale>

const size_t cycleStrBufLen = 1024;
PSTR cycleStrBufferA();
PWSTR cycleStrBufferW();
PTSTR cycleStrBuffer();
PCTSTR GetErrorText(DWORD nErrorCode);

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
