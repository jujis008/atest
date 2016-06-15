#include "stdafx.h"
#include "util.h"
#include <Wincrypt.h>
#include <TlHelp32.h>
#include <Wtsapi32.h>
#include <WinInet.h>
#include <regex>
#include <string>
#include <sstream>

/*-----------------------------------------------------------------------------
Recursively find the highest visible window for the given process
-----------------------------------------------------------------------------*/
static HWND findDocumentWindow(DWORD processId, HWND parent) {
	HWND documentWindow = NULL;
	HWND hwnd = ::GetWindow(parent, GW_CHILD);
	while (hwnd && !documentWindow) {
		if (IsWindowVisible(hwnd)) {
			DWORD pid;
			GetWindowThreadProcessId(hwnd, &pid);
			if (pid == processId && isBrowserDocument(hwnd)) {
				documentWindow = hwnd;
			}
			else {
				documentWindow = findDocumentWindow(processId, hwnd);
			}
		}
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}
	return documentWindow;
}

/*-----------------------------------------------------------------------------
Find the top-level and document windows for the browser
-----------------------------------------------------------------------------*/
bool findBrowserWindow(DWORD processId, HWND& frameWindow) {
	bool found = false;
	// find a known document window that belongs to this process
	HWND documentWindow = ::findDocumentWindow(processId, ::GetDesktopWindow());
	if (documentWindow) {
		found = true;
		frameWindow = GetAncestor(documentWindow, GA_ROOTOWNER);
	}
	return found;
}

bool lanchProcess(CString commandLine, HANDLE* processHandle, const TCHAR* dir) {
	bool ret = false;
	if (commandLine.GetLength()) {
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)commandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, dir, &si, &pi)) {
			if (processHandle) {
				*processHandle = pi.hProcess;
				ret = true;
				CloseHandle(pi.hThread);
			}
			else {
				WaitForSingleObject(pi.hProcess, 60 * 60 * 1000);
				DWORD code;
				if (GetExitCodeProcess(pi.hProcess, &code) && code == 0) {
					ret = true;
				}
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
		}
	} else {
		ret = true;
	}
	return ret;
}