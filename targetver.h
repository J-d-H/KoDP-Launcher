﻿#pragma once

// Durch Einbeziehen von"SDKDDKVer.h" wird die höchste verfügbare Windows-Plattform definiert.

// Wenn Sie die Anwendung für eine frühere Windows-Plattform erstellen möchten, schließen Sie "WinSDKVer.h" ein, und
// legen Sie das _WIN32_WINNT-Makro auf die zu unterstützende Plattform fest, bevor Sie "SDKDDKVer.h" einschließen.

#define _WIN32_WINNT 0x0600 // _WIN32_WINNT_VISTA
#include <SDKDDKVer.h>
