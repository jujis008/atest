// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define PSAPI_VERSION 1
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>					//windows api base
#include "stdlib.h"						//standard lib
#include "malloc.h"						//
#include "memory.h"
#include "Psapi.h"
#include "tchar.h"
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#include <Ws2tcpip.h>
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#include <Wincrypt.h>

#include <shlobj.h>
#include <atlstr.h>
#include <atlcoll.h>
// TODO: reference additional headers your program requires here
