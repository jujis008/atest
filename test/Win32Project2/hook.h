#pragma once

#ifndef SCREENSHOT
#define _import __declspec(dllimport)
#else
#define _import __declspec(dllexport)
#endif // !SCREENSHOT

extern "C" {
	//_import void WINAPI InstallHook(void);
	_import void WINAPI SetResultsFileBase(const TCHAR * file_base);
	_import void WINAPI SetDebugLevel(int level, const TCHAR * log_file);
	_import void WINAPI SetBrowserExe(const TCHAR * exe);
	_import DWORD WINAPI GetBrowserProcessId();
}
