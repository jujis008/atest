#pragma once
#include "TlHelp32.h"
namespace loglevel {
	const int kError = 1;
	const int kWarning = 2;
	const int kProcess = 3;
	const int kRareEvent = 4;
	const int kFrequentEvent = 7;
	const int kFunction = 8;
	const int kTrace = 9;
}

bool lauchProcess(CString commandLine, HANDLE* processHandle = NULL, const TCHAR *dir = NULL);
void deleteDirectory(LPCTSTR directory, bool remove = true);
void deleteOldDirectoryEntries(CString directory, int seconds);
void deleteRegKey(HKEY hParent, LPCTSTR key, bool remove = true);
void copyDirectoryTree(CString source, CString destination);
bool findBrowserWindow(DWORD process_id, HWND& frameWindow);
void wptTrace(int level, LPCTSTR format, ...);
typedef CAtlList<CStringA> hookSymbolNames;
typedef CAtlMap<CStringA, DWORD64> hookOffsets;
CString createAppDataDir();
bool getModuleByName(HANDLE process, LPCTSTR moduleName, MODULEENTRY32 * module);
DWORD findProcessIds(TCHAR * exe, CAtlList<DWORD> &pids);
DWORD getParentProcessId(DWORD pid);
void terminateProcessAndChildren(DWORD pid);
void terminateProcessById(DWORD pid);
void terminateProcessesByName(TCHAR * exe);
void waitForChildProcesses(DWORD pid, DWORD timeout = 300000);
void waitForProcessesByName(TCHAR * exe, DWORD timeout = 300000);
bool isBrowserDocument(HWND wnd, bool recurse = true);
CString httpGetText(CString url);
DWORD   httpSaveFile(CString url, CString file);
CString hashFileMD5(CString file);
bool fileExists(CString file);
bool regexMatch(CStringA str, CStringA regex);
void queryPerfCounter(__int64 &counter);
void queryPerfFrequency(__int64 &freq);
int elapsedFileTimeSeconds(FILETIME& check, FILETIME& now);
void reboot();