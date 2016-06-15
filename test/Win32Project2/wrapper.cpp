// Win32Project2.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "wrapper.h"
#include "shared_mem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWinApp theApp;

using namespace std;
wrapper::wrapper(const TCHAR* file_base, const TCHAR* browser):
test_state_(results_, screen_capture_)
,results_(test_state_, screen_capture_) {
	if (file_base) {
		lstrcpy(shared_results_file_base, file_base);
	} else {
		lstrcpy(shared_results_file_base, "");
	}
	if (browser) {
		lstrcpy(shared_browser_exe, browser);
	} else {
		lstrcpy(shared_browser_exe, "");
	}
	MessageBox(NULL, shared_results_file_base, shared_browser_exe, MB_OK);
}

wrapper::~wrapper() {

}

void wrapper::init() {
	test_state_.init();
}

void wrapper::start() {
	results_.save();
	test_state_.start();
}

void wrapper::stop() {
	results_.save();
	test_state_.done(true);
}

//void wrapper::setFileBase(const TCHAR* fileBase) {
//	SetResultsFileBase(fileBase);
//}
//
//void wrapper::setBrowserEx(const TCHAR* browser) {
//	SetBrowserExe(browser);
//}