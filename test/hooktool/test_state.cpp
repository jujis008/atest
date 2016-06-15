#include "stdafx.h"
#include "test_state.h"
#include "screen_capture.h"
#include "shared_mem.h"
#include "cximage\ximage.h"
#include "Trace.h"
#include "WtsApi32.h"
#include "util.h"

static const DWORD SCREEN_CAPTURE_INCREMENTS = 200;
static const DWORD DATA_COLLECTION_INTERVAL = 100;

TestState::~TestState(void) {
	done(true);
	DeleteCriticalSection(&_data_cs);
}

void TestState::done(bool force) {

}

void TestState::init() {

}

void TestState::findViewPort(bool force) {
	if (_frame_window && (force || !_screen_capture.isViewPointSet())) {
		_screen_capture.clearViewPoint();
		CapturedImage captured = _screen_capture.captureImage(_frame_window);
		CxImage image;
		if (captured.get(image)) {
			DWORD width = image.GetWidth();
			DWORD height = image.GetHeight();
			if (width > 100 && height > 100) {
				DWORD x = width / 2;
				DWORD y = height / 2;
				RECT viewport = { 0,0,0,0 };
				DWORD rawBytes = image.GetEffWidth();
				DWORD pixelBytes = rawBytes / width;
				unsigned char* middle = image.GetBits(y);
				if (middle) {
					middle += x*pixelBytes;
					unsigned char* background[3];
					memcpy(background, middle, 3);
					//find the top
					unsigned char* pixel = middle;
					while (y < height - 1 && !viewport.top) {
						if (memcmp(background, pixel, 3)) {
							viewport.top = height - y;
						}
						pixel += rawBytes;
						y++;
					}

					// find the bottom
					y = height / 2;
					pixel = middle;
					while (y && !viewport.bottom) {
						if (memcmp(background, pixel, 3)) {
							viewport.bottom = height - y;
						}
						pixel -= rawBytes;
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
						pixel -= pixelBytes;
						x--;
					}

					// find the right
					x = width / 2;
					pixel = middle;
					while (x < width && !viewport.right) {
						if (memcmp(background, pixel, 3)) {
							viewport.right = x - 1;
						}
						pixel += pixelBytes;
						x++;
					}
					if (!viewport.right) {
						viewport.right = width - 1;
					}
				}
				if (viewport.right - viewport.left > (long)width / 2 &&
					viewport.bottom - viewport.top > (long)height / 2) {
					_screen_capture.setViewPort(viewport);
				}
			}
		}
		captured.free();
	}
}

void TestState::grabVideoFrame(bool force) {
	if (_active && _frame_window && force) {
		bool grabVideo = false;
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		if (!_last_video_time.QuadPart) {
			grabVideo = true;
		}
		else {
			DWORD interval = DATA_COLLECTION_INTERVAL;
			if (_video_capture_count > SCREEN_CAPTURE_INCREMENTS * 2) {
				interval *= 20;
			}
			else if (_video_capture_count > SCREEN_CAPTURE_INCREMENTS) {
				interval *= 5;
			}
			LARGE_INTEGER minTime;
			minTime.QuadPart = _last_video_time.QuadPart + (interval*_ms_frequency.QuadPart);
			if (now.QuadPart >= minTime.QuadPart) {
				grabVideo = true;
			}
		}
		if (grabVideo) {
			_last_video_time.QuadPart = now.QuadPart;
			_video_capture_count++;
			_screen_capture.capture(_frame_window);
		}
	}
}

void TestState::findBrowserNameAndVersion() {
	TCHAR file_name[MAX_PATH];
	if (GetModuleFileName(NULL, file_name, _countof(file_name))) {
		CString exe(file_name);
		exe.MakeLower();
		if (exe.Find(_T("webkit2webprocess.exe")) >= 0) {
			_no_gdi = true;
			_browser_name = _T("Safari");
		}
		else if (exe.Find(_T("safari.exe")) >= 0) {
			_gdi_only = true;
		}
		DWORD unused;
		DWORD info_size = GetFileVersionInfoSize(file_name, &unused);
		if (info_size) {
			LPBYTE version_info = new BYTE[info_size];
			if (GetFileVersionInfo(file_name, 0, info_size, version_info)) {
				VS_FIXEDFILEINFO *file_info = NULL;
				UINT size = 0;
				if (VerQueryValue(version_info, _T("\\"), (LPVOID*)&file_info, &size) &&
					file_info) {
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
				if (_browser_name.IsEmpty() &&
					VerQueryValue(version_info, TEXT("\\VarFileInfo\\Translation"),
						(LPVOID*)&translate, &size)) {
					// Use the first language/code page.
					CString key;
					key.Format(_T("\\StringFileInfo\\%04x%04x\\FileDescription"),
						translate[0].language, translate[0].code_page);
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
	}
}

void TestState::titleSet(CString title) {

}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
BOOL CALLBACK MakeTopmost(HWND hwnd, LPARAM lParam) {
	TCHAR class_name[1024];
	if (IsWindowVisible(hwnd) &&
		GetClassName(hwnd, class_name, _countof(class_name))) {
		_tcslwr(class_name);
		if (_tcsstr(class_name, _T("chrome")) ||
			_tcsstr(class_name, _T("mozilla"))) {
			::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			::UpdateWindow(hwnd);
		}
	}
	return TRUE;
}

void TestState::updateBrowserWindow() {
	if (!_started) {
		DWORD browserProcessId = GetCurrentProcessId();
		if (_no_gdi) {
			browserProcessId = GetCurrentProcessId();
		}
		HWND oldFrame = _frame_window;
		if (::findBrowserWindow(browserProcessId, _frame_window)) {
			wptTrace(loglevel::kFunction, _T("[wpthook] - Frame Window: %08X\n"), _frame_window);
		}
		// position the browser window
		if (_frame_window && oldFrame != _frame_window) {
			DWORD browserWidth = 800;
			DWORD browserHeight = 600;
			DWORD _viewport_width = 800;
			DWORD _viewport_height = 600;
			::ShowWindow(_frame_window, SW_RESTORE);
			if (_viewport_width && _viewport_height) {
				::UpdateWindow(_frame_window);
				findViewPort();
				RECT browser;
				GetWindowRect(_frame_window, &browser);
				RECT viewport = { 0,0,0,0 };
				if (_screen_capture.isViewPointSet())
					memcpy(&viewport, &_screen_capture._viewPort, sizeof(RECT));
				int vp_width = abs(viewport.right - viewport.left);
				int vp_height = abs(viewport.top - viewport.bottom);
				int br_width = abs(browser.right - browser.left);
				int br_height = abs(browser.top - browser.bottom);
				if (vp_width && vp_height && br_width && br_height &&
					br_width >= vp_width && br_height >= vp_height) {
					browserWidth = _viewport_width + (br_width - vp_width);
					browserHeight = _viewport_height + (br_height - vp_height);
				}
				_screen_capture.clearViewPoint();
			}
			::SetWindowPos(_frame_window, HWND_TOPMOST, 0, 0,
				browserWidth, browserHeight, SWP_NOACTIVATE);
			::UpdateWindow(_frame_window);
			EnumWindows(::MakeTopmost, (LPARAM)this);
			findViewPort();
		}
	}
}

