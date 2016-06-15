#include "stdafx.h"
#include "test_state.h"
#include "result.h"
#include "screen_capture.h"
#include "ximage.h"
#include "util.h"
#include "shared_mem.h"
#include <Mmsystem.h>

static const DWORD SCREEN_CAPTURE_INCREMENTS = 200;
static const DWORD DATA_COLLECTION_INTERVAL = 100;
static const DWORD START_RENDER_MARGIN = 80;
static const DWORD MS_IN_SEC = 1000;
static const DWORD SCRIPT_TIMEOUT_MULTIPLIER = 10;
static const DWORD RESPONSIVE_BROWSER_WIDTH = 480;
static const DWORD BROWSER_WIDTH = 1024;
static const DWORD BROWSER_HEIGHT = 768;

TestState::TestState(Results& results, ScreenCapture& screen_capture) :
	_results(results)
	, _screen_capture(screen_capture)
	, _frame_window(NULL)
	, no_gdi_(false)
	, gdi_only_(false)
	, _exit(false)
	, _data_timer(NULL)
	, _started(false) {
	QueryPerformanceFrequency(&_ms_frequency); 
	_ms_frequency.QuadPart = _ms_frequency.QuadPart / 1000;
	InitializeCriticalSection(&_data_cs);
	findBrowserNameAndVersion();
	_browser_name = shared_browser_exe;
	tmp_file_base = shared_results_file_base;
	_timeout_start_time.QuadPart = 0;
}

TestState::~TestState(void) {
	done(true);
	DeleteCriticalSection(&_data_cs);
}

void TestState::init() {
	reset();
}

void TestState::reset() {
	EnterCriticalSection(&_data_cs);
	_step_start.QuadPart = 0;
	_fixed_viewport = -1;
	_viewport_specified = -1;

	_active = false;
	_video_capture_count = 0;
	_start.QuadPart = 0;
	_last_video_time.QuadPart = 0;
	_title_time.QuadPart = 0;
	_title.Empty();
	_last_data.QuadPart = 0;
	//_user_agent = _T("Screenshot");
	GetSystemTime(&_start_time);
	if (shared_browser_exe) {
		lstrcpy((LPSTR)(LPCSTR)_browser_name, shared_browser_exe);
	}
	//_browser_name = shared_browser_exe;
	tmp_file_base = shared_results_file_base;
	LeaveCriticalSection(&_data_cs);
}

void __stdcall collectDelta(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
	if (lpParameter) {
		((TestState *)lpParameter)->collectData();
	}
}

void TestState::start() {
	MessageBox(NULL, _browser_name, tmp_file_base, 0);
	reset();
	QueryPerformanceCounter(&_step_start);
	GetSystemTime(&_start_time);
	if (!_start.QuadPart) {
		_start.QuadPart = _step_start.QuadPart;
	}
	if (!_timeout_start_time.QuadPart) {
		_timeout_start_time.QuadPart = _step_start.QuadPart;
	}

	_active = true;
	if (!_started) {
		MessageBox(NULL, shared_browser_exe, "started", 0);
		updateBrowserWindow(shared_browser_exe); // the document window may not be available yet
		findViewport(true);
		_started = true;
	}
	if (!_data_timer) {
		timeBeginPeriod(1);
		CreateTimerQueueTimer(&_data_timer, NULL, (WAITORTIMERCALLBACK)::collectDelta, this, DATA_COLLECTION_INTERVAL, DATA_COLLECTION_INTERVAL, WT_EXECUTEDEFAULT);
	}	
	grabVideoFrame(true);
	collectData();
}

void TestState::collectData() {
	EnterCriticalSection(&_data_cs);
	if (_active) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);		
		if ((now.QuadPart > _last_data.QuadPart) || !_last_data.QuadPart) {
			checkTitle();
			grabVideoFrame(true);
			_last_data.QuadPart = now.QuadPart;
		}
	}
	LeaveCriticalSection(&_data_cs);
}

BOOL CALLBACK MakeTopmost(HWND hwnd, LPARAM lParam) {
	TCHAR class_name[1024];
	if (IsWindowVisible(hwnd) &&
		GetClassName(hwnd, class_name, _countof(class_name))) {
		_tcslwr(class_name);
		if (_tcsstr(class_name, _T("chrome")) ||
			_tcsstr(class_name, _T("mozilla"))) {
			//HWND_TOP HWND_BOTTOM HWND_TOPMOST HWND_NOTOPMOST
			::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			::UpdateWindow(hwnd);
		}
	}
	return TRUE;
}

void TestState::updateBrowserWindow(TCHAR* browserName) {
	if (!_started) {
		CAtlList<DWORD> processes;
		FindProcessIds(browserName, processes);
		if (!processes.IsEmpty()) {
			POSITION pos = processes.GetHeadPosition();
			HWND _old_frame = _frame_window;
			while (pos) { //this list should be one element since every time will terminiate browser while starting a new test case.
				DWORD pid = processes.GetNext(pos);
				if (FindBrowserWindow(pid, _frame_window)) {
					shared_browser_process_id = pid;
					WptTrace(loglevel::kFunction, _T("[wpthook] - Frame Window: %08X\n"), _frame_window);
					break;
				}
			}
			//position the browser window
			if (_frame_window && (_old_frame != _frame_window)) {
				DWORD browser_width = BROWSER_WIDTH;//_test._browser_width;
				DWORD browser_height = BROWSER_HEIGHT;//_test._browser_height;
				DWORD _viewport_width = 0; //need to read from file or passed from java end
				DWORD _viewport_height = 0;
				::ShowWindow(_frame_window, SW_RESTORE);
				if (_viewport_width && _viewport_height) {
					::UpdateWindow(_frame_window);
					findViewport();
					RECT browser;
					GetWindowRect(_frame_window, &browser);
					RECT viewport = { 0,0,0,0 };
					if (_screen_capture.IsViewportSet()) {
						memcpy(&viewport, &_screen_capture._viewport, sizeof(RECT));
					}
					int vp_width = abs(viewport.right - viewport.left);
					int vp_height = abs(viewport.top - viewport.bottom);
					int br_width = abs(browser.right - browser.left);
					int br_height = abs(browser.top - browser.bottom);
					if (vp_width && vp_height && br_width && br_height && br_width >= vp_width && br_height >= vp_height) {
						browser_width = _viewport_width + (br_width - vp_width);
						browser_height = _viewport_height + (br_height - vp_height);
					}
					_screen_capture.ClearViewport();
					::SetWindowPos(_frame_window, HWND_TOPMOST, 0, 0, browser_width, browser_height, SWP_NOACTIVATE);
					::UpdateWindow(_frame_window);
					EnumWindows(::MakeTopmost, (LPARAM)this);
					findViewport();
				}
			}
		}
	}
}


/*-----------------------------------------------------------------------------
Grab a video frame if it is appropriate
-----------------------------------------------------------------------------*/
void TestState::grabVideoFrame(bool force) {
	if (_active && _frame_window /*&& force*/) {
		// use a falloff on the resolution with which we capture video
		bool grab_video = false;
		LARGE_INTEGER now;//record time
		QueryPerformanceCounter(&now);
		if (!_last_video_time.QuadPart) {
			grab_video = true;
		} else {
			DWORD interval = DATA_COLLECTION_INTERVAL;
			if (_video_capture_count > SCREEN_CAPTURE_INCREMENTS * 2) {
				interval *= 20;
			} else if (_video_capture_count > SCREEN_CAPTURE_INCREMENTS) {
				interval *= 5;
			}
			LARGE_INTEGER min_time;
			min_time.QuadPart = _last_video_time.QuadPart + (interval * _ms_frequency.QuadPart);
			if (now.QuadPart >= min_time.QuadPart) {
				grab_video = true;
			}

		}
		if (grab_video) {
			_last_video_time.QuadPart = now.QuadPart;
			_video_capture_count++;
			_screen_capture.Capture(_frame_window);
		}
	}
}

void TestState::checkTitle() {
	if (_active && _frame_window) {
		TCHAR title[4096];
		if (GetWindowText(_frame_window, title, _countof(title))) {
			if (last_title_.Compare(title)) {
				last_title_ = title;
				if (last_title_.Left(5).Compare(_T("Blank"))) {
					titleSet(title);
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
Keep track of the page title and when it was first set (first title only)
-----------------------------------------------------------------------------*/
void TestState::titleSet(CString title) {
	if (_active) {
		CString new_title = title.Trim();
		// trim the browser off of the title ( - Chrome, etc)
		int pos = new_title.ReverseFind(_T('-'));
		if (pos > 0) {
			new_title = new_title.Left(pos).Trim();
		}
		if (!_title_time.QuadPart || new_title.Compare(_title)) {
			QueryPerformanceCounter(&_title_time);
			_title = new_title;
			WptTrace(loglevel::kFunction, _T("[wpthook] TestState::TitleSet(%s)\n"), _title);
		}
	}
}

/*-----------------------------------------------------------------------------
Find the portion of the document window that represents the document
-----------------------------------------------------------------------------*/
void TestState::findViewport(bool force) {
	if (_frame_window && (force || !_screen_capture.IsViewportSet())) {
		_screen_capture.ClearViewport();
		CapturedImage captured = _screen_capture.CaptureImage(_frame_window);
		CxImage image;
		if (captured.Get(image)) {
			// start in the middle of the image and go in each direction 
			// until we get a pixel of a different color
			DWORD width = image.GetWidth();
			DWORD height = image.GetHeight();
			if (width > 100 && height > 100) {
				DWORD x = width / 2;
				DWORD y = height / 2;
				RECT viewport = { 0,0,0,0 };
				DWORD row_bytes = image.GetEffWidth();
				DWORD pixel_bytes = row_bytes / width;
				unsigned char * middle = image.GetBits(y);
				if (middle) {
					middle += x * pixel_bytes;
					unsigned char background[3];
					memcpy(background, middle, 3);
					// find the top
					unsigned char * pixel = middle;
					while (y < height - 1 && !viewport.top) {
						if (memcmp(background, pixel, 3)) {
							viewport.top = height - y;
						}
						pixel += row_bytes;
						y++;
					}
					// find the bottom
					y = height / 2;
					pixel = middle;
					while (y && !viewport.bottom) {
						if (memcmp(background, pixel, 3)) {
							viewport.bottom = height - y;
						}
						pixel -= row_bytes;
						y--;
					}
					if (!viewport.bottom) {
						viewport.bottom = height - 1;
					}
					// find the left
					pixel = middle;
					while (x && !viewport.left) {
						if (memcmp(background, pixel, 3)) {
							viewport.left = x + 1;
						}
						pixel -= pixel_bytes;
						x--;
					}
					// find the right
					x = width / 2;
					pixel = middle;
					while (x < width && !viewport.right) {
						if (memcmp(background, pixel, 3)) {
							viewport.right = x - 1;
						}
						pixel += pixel_bytes;
						x++;
					}
					if (!viewport.right) {
						viewport.right = width - 1;
					}
				}
				if (viewport.right - viewport.left > (long)width / 2 && viewport.bottom - viewport.top > (long)height / 2) {
					_screen_capture.SetViewport(viewport);
				}
			}
		}
		captured.Free();
	}
}

/*-----------------------------------------------------------------------------
Convert |time| to the number of milliseconds since the start.
-----------------------------------------------------------------------------*/
DWORD TestState::elapsedMsFromStart(LARGE_INTEGER end) const {
	return elapsedMs(_start, end);
}

DWORD TestState::elapsedMs(LARGE_INTEGER start, LARGE_INTEGER end) const {
	DWORD elapsed_ms = 0;
	if (start.QuadPart && end.QuadPart > start.QuadPart) {
		elapsed_ms = static_cast<DWORD>((end.QuadPart - start.QuadPart) / _ms_frequency.QuadPart);
	}
	return elapsed_ms;
}

/*-----------------------------------------------------------------------------
Find the browser name and version.
-----------------------------------------------------------------------------*/
void TestState::findBrowserNameAndVersion() {
	TCHAR file_name[MAX_PATH];
	if (GetModuleFileName(NULL, file_name, _countof(file_name))) {
		CString exe(file_name);
		exe.MakeLower();
		if (exe.Find(_T("webkit2webprocess.exe")) >= 0) {
			no_gdi_ = true;
			_browser_name = _T("Safari");
		} else if (exe.Find(_T("safari.exe")) >= 0) {
			gdi_only_ = true;
		}
		DWORD unused;
		DWORD info_size = GetFileVersionInfoSize(file_name, &unused);
		if (info_size) {
			LPBYTE version_info = new BYTE[info_size];
			if (GetFileVersionInfo(file_name, 0, info_size, version_info)) {
				VS_FIXEDFILEINFO *file_info = NULL;
				UINT size = 0;
				if (VerQueryValue(version_info, _T("\\"), (LPVOID*)&file_info, &size) && file_info) {
					_browser_version.Format(_T("%d.%d.%d.%d"), 
						HIWORD(file_info->dwFileVersionMS),
						LOWORD(file_info->dwFileVersionMS),
						HIWORD(file_info->dwFileVersionLS),
						LOWORD(file_info->dwFileVersionLS));
				}
				// Structure used to store enumerated languages and code pages.
				struct LANGANDCODEPAGE {
					WORD language;
					WORD code_page;
				} *translate;
				// Read the list of languages and code pages.
				if (_browser_name.IsEmpty() && VerQueryValue(version_info, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&translate, &size)) {
					// Use the first language/code page.
					CString key;
					key.Format(_T("\\StringFileInfo\\%04x%04x\\FileDescription"), translate[0].language, translate[0].code_page);
					LPTSTR file_desc = NULL;
					if (VerQueryValue(version_info, key, (LPVOID*)&file_desc, &size)) {
						_browser_name = file_desc;
					}
				}
			}
			delete[] version_info;
		}
		if (_browser_name.IsEmpty()) {
			PathRemoveExtension(file_name);
			PathStripPath(file_name);
			_browser_name = file_name;
		}
		MessageBox(NULL, _browser_name, _browser_version+"Version", 0);
	}
}

void TestState::lock() {
	EnterCriticalSection(&_data_cs);
}

void TestState::unLock() {
	LeaveCriticalSection(&_data_cs);
}

void TestState::done(bool force) {
	WptTrace(loglevel::kFunction, _T("[wpthook] - **** TestState::Done()\n"));
	if (_active) {
		_screen_capture.Capture(_frame_window);
		if (force) {
			// kill the timer that was collecting periodic data (cpu, video, etc)
			if (_data_timer) {
				DeleteTimerQueueTimer(NULL, _data_timer, INVALID_HANDLE_VALUE);
				_data_timer = NULL;
				timeEndPeriod(1);
			}
		}
		_started = false;
		_active = false;
	}
}

//bool TestState::isDone() {
//	bool is_done = false;
//	LARGE_INTEGER now;
//	QueryPerformanceCounter(&now);
//	DWORD test_ms = elapsedMs(_step_start, now);
//	if (_active) {
//		bool is_page_done = false;
//		CString done_reason;
//		DWORD elapsed_timeout_ms = elapsedMs(_timeout_start_time, now);
//		if (elapsed_timeout_ms > _test._test_timeout) {
//			_test_result = TEST_RESULT_TIMEOUT;
//			is_page_done = true;
//			done_reason = _T("Test timed out.");
//			_test._has_test_timed_out = true;
//		}
//	}
//}

void TestState::resizeBrowserForResponsiveTest() {
	RECT rect;
	if (_frame_window && GetWindowRect(_frame_window, &rect)) {
		int height = abs(rect.top - rect.bottom);
		::SetWindowPos(_frame_window, NULL, 0, 0, RESPONSIVE_BROWSER_WIDTH, height, SWP_NOACTIVATE);
		::UpdateWindow(_frame_window);
	}
}

/*-----------------------------------------------------------------------------
We do some sanity checking here to make sure the value reported
from the extension is sane.
-----------------------------------------------------------------------------*/
void TestState::recordTime(CString name, DWORD time, LARGE_INTEGER* out_time) {
	QueryPerformanceFrequency(out_time);
	DWORD elapsed_time = 0;
	if (_step_start.QuadPart && out_time->QuadPart >= _step_start.QuadPart) {
		elapsed_time = (DWORD)((out_time->QuadPart - _step_start.QuadPart) / _ms_frequency.QuadPart);
	}
	if (time > 0 && time <= elapsed_time) {
		WptTrace(loglevel::kFrequentEvent, _T("[wpthook] - Record %s from extension: %dms\n"), name, time);
		out_time->QuadPart = _step_start.QuadPart;
		out_time->QuadPart += _ms_frequency.QuadPart*time;
		WptTrace(loglevel::kFrequentEvent, _T("[wpthook] - Record %s from hook: %dms (instead of %dms)\n"));
	} else {
		WptTrace(loglevel::kFrequentEvent, _T("[wpthook] - Record %s from hook: %dms (instead of %dms)\n"), name, elapsed_time, time);
	}
}