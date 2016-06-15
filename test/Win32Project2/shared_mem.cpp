#include "stdafx.h"
#include "shared_mem.h"
//#include "hook.h"


#pragma once
#pragma data_seg(".shared")
TCHAR shared_results_file_base[MAX_PATH] = { NULL };
DWORD shared_test_timeout = 120000;
TCHAR shared_log_file[MAX_PATH] = { NULL };
int   shared_debug_level = 0;
TCHAR shared_browser_exe[MAX_PATH] = { NULL };
DWORD shared_browser_process_id = 0;
#pragma data_seg()

#pragma comment(linker, "/SECTION:.shared,RWS")

///*-----------------------------------------------------------------------------
//Set the base file name to use for results files
//-----------------------------------------------------------------------------*/
//void WINAPI SetResultsFileBase(const TCHAR * file_base) {
//	lstrcpy(shared_results_file_base, file_base);
//}
//
//void WINAPI SetDebugLevel(int level, const TCHAR * log_file) {
//	shared_debug_level = level;
//	lstrcpy(shared_log_file, log_file);
//}
//
///*-----------------------------------------------------------------------------
//Set the exe name for the browser we are currently using
//-----------------------------------------------------------------------------*/
//void WINAPI SetBrowserExe(const TCHAR * exe) {
//	if (exe)
//		lstrcpy(shared_browser_exe, exe);
//	else
//		lstrcpy(shared_browser_exe, "");
//	shared_browser_process_id = 0;
//}
//
///*-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------*/
//DWORD WINAPI GetBrowserProcessId() {
//	return shared_browser_process_id;
//}
