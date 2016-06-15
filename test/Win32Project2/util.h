#pragma once

#include <TlHelp32.h>

namespace loglevel {
	const int kError = 1;
	const int kWarning = 2;
	const int kProcess = 3;
	const int kRareEvent = 4;
	const int kFrequentEvent = 7;
	const int kFunction = 8;
	const int kTrace = 9;
};


// Utility routines shared by all of the code

bool LaunchProcess(CString command_line, HANDLE * process_handle = NULL,
	const TCHAR *dir = NULL);
void DeleteDirectory(LPCTSTR directory, bool remove = true);
void DeleteOldDirectoryEntries(CString directory, int seconds);
void DeleteRegKey(HKEY hParent, LPCTSTR key, bool remove = true);
void CopyDirectoryTree(CString source, CString destination);
bool FindBrowserWindow(DWORD process_id, HWND& frame_window);
void WptTrace(int level, LPCTSTR format, ...);

typedef CAtlList<CStringA> HookSymbolNames;
typedef CAtlMap<CStringA, DWORD64> HookOffsets;
CString CreateAppDataDir();
bool GetModuleByName(HANDLE process, LPCTSTR module_name,
	MODULEENTRY32 * module);
DWORD FindProcessIds(TCHAR * exe, CAtlList<DWORD> &pids);
DWORD GetParentProcessId(DWORD pid);
void TerminateProcessAndChildren(DWORD pid);
void TerminateProcessById(DWORD pid);
void TerminateProcessesByName(TCHAR * exe);
void WaitForChildProcesses(DWORD pid, DWORD timeout = 300000);
void WaitForProcessesByName(TCHAR * exe, DWORD timeout = 300000);
bool IsBrowserDocument(HWND wnd, bool recurse = true);
CString HttpGetText(CString url);
DWORD   HttpSaveFile(CString url, CString file);
CString HashFileMD5(CString file);
bool FileExists(CString file);
bool  RegexMatch(CStringA str, CStringA regex);
CStringA JSONEscape(CString src);
CStringA JSONEscapeA(CStringA src);
void QueryPerfCounter(__int64 &counter);
void QueryPerfFrequency(__int64 &freq);
int ElapsedFileTimeSeconds(FILETIME& check, FILETIME& now);
void Reboot();