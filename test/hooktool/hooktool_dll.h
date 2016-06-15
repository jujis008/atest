#pragma once
#ifndef HOOKDRIVER
#define _import __declspec(dllimport);
#else
#define _import __declspec(dllexport);
#endif // !HOOKDRIVER

extern "C" {
	_import void WINAPI installHook(void);
	_import void WINAPI setResultFileBase(const WCHAR* fileBase);
	_import void WINAPI setTestTimeOut(DWORD timeout);
	_import void WINAPI setEnableUserSetTimeout(bool enableUserSetTimeOut);
	_import void WINAPI setCurrentRun(DWORD run);
	_import void WINAPI setDebugLevel(int level, const WCHAR* logFile);
	_import void WINAPI ResetTestResult();
	_import int  WINAPI GetTestResult();
	_import void WINAPI SetBrowserExe(const WCHAR * exe);
	_import DWORD WINAPI GetBrowserProcessId();
}