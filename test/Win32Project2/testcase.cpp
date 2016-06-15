#include "stdafx.h"
#include "testcase.h"
#include "hook.h"


TestCase::TestCase() {
	InitializeCriticalSection(&cs);
}

TestCase::~TestCase() {
	DeleteCriticalSection(&cs);
}

void TestCase::init() {
	reset();
}

void TestCase::reset() {
	EnterCriticalSection(&cs);
	SetBrowserExe(_browser_ex);
	SetResultsFileBase(_file_base);
	LeaveCriticalSection(&cs);
}