#include "StdAfx.h"
#include "screen_capture.h"
//#include "shared_mem.h
#include "ximage.h"
//#include "test_state.h"

// global indicator that we are capturing a screen shot
// (so that any GDI hooks can ignore our activity)
bool wpt_capturing_screen = false;

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
ScreenCapture::ScreenCapture() :_viewport_set(false) {
	InitializeCriticalSection(&cs);
	memset(&_viewport, 0, sizeof(_viewport));
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
ScreenCapture::~ScreenCapture(void) {
	Reset();
	DeleteCriticalSection(&cs);
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void ScreenCapture::Reset() {
	EnterCriticalSection(&cs);
	while (!_captured_images.IsEmpty()) {
		CapturedImage& image = _captured_images.RemoveHead();
		image.Free();
	}
	LeaveCriticalSection(&cs);
}

/*-----------------------------------------------------------------------------
Capture a screen shot and save it in our list
-----------------------------------------------------------------------------*/
void ScreenCapture::Capture(HWND wnd, CString file_base, bool crop_viewport) {
	if (wnd) {
		EnterCriticalSection(&cs);
		RECT * rect = NULL;
		if (crop_viewport && _viewport_set)
			rect = &_viewport;
		CapturedImage image(wnd, rect);
		_captured_images.AddTail(image);
		LeaveCriticalSection(&cs);
	}
}

/*-----------------------------------------------------------------------------
Capture a screen shot and save it in our list
-----------------------------------------------------------------------------*/
void ScreenCapture::Capture(HWND wnd, bool crop_viewport) {
	if (wnd) {
		EnterCriticalSection(&cs);
		RECT * rect = NULL;
		if (crop_viewport && _viewport_set)
			rect = &_viewport;
		CapturedImage image(wnd, rect);
		_captured_images.AddTail(image);
		LeaveCriticalSection(&cs);
	}
}

/*-----------------------------------------------------------------------------
Capture a screen shot and return it without saving it
-----------------------------------------------------------------------------*/
CapturedImage ScreenCapture::CaptureImage(HWND wnd, bool crop_viewport) {
	CapturedImage ret;
	if (wnd) {
		EnterCriticalSection(&cs);
		RECT * rect = NULL;
		if (crop_viewport && _viewport_set)
			rect = &_viewport;
		CapturedImage captured(wnd, rect);
		ret = captured;
		LeaveCriticalSection(&cs);
	}
	return ret;
}

/*-----------------------------------------------------------------------------
Get the last image of the requested type
-----------------------------------------------------------------------------*/
bool ScreenCapture::GetImage(CxImage& image) {
	bool ret = false;
	image.Destroy();
	EnterCriticalSection(&cs);
	POSITION pos = _captured_images.GetHeadPosition();
	while (pos) {
		CapturedImage& captured_image = _captured_images.GetNext(pos);
		ret = captured_image.Get(image);
	}
	LeaveCriticalSection(&cs);

	return ret;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void ScreenCapture::Lock() {
	EnterCriticalSection(&cs);
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void ScreenCapture::Unlock() {
	LeaveCriticalSection(&cs);
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void ScreenCapture::ClearViewport() {
	memset(&_viewport, 0, sizeof(_viewport));
	_viewport_set = false;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void ScreenCapture::SetViewport(RECT& viewport) {
	memcpy(&_viewport, &viewport, sizeof(_viewport));
	_viewport_set = true;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
bool ScreenCapture::IsViewportSet() {
	return _viewport_set;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
CapturedImage::CapturedImage() :_bitmap_handle(NULL) {
	_capture_time.QuadPart = 0;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
CapturedImage::CapturedImage(HWND wnd, RECT * rect) :
	_bitmap_handle(NULL) {
	_capture_time.QuadPart = 0;
	if (wnd) {
		wpt_capturing_screen = true;
		HDC src = GetDC(wnd);
		if (src) {
			HDC dc = CreateCompatibleDC(src);
			if (dc) {
				RECT window_rect = { 0 };
				::GetWindowRect(wnd, &window_rect);
				int left = window_rect.left;
				int top = window_rect.top;
				int width = abs(window_rect.right - window_rect.left);
				int height = abs(window_rect.top - window_rect.bottom);
				if (rect) {
					left = window_rect.left + rect->left;
					top = window_rect.top + rect->top;
					width = rect->right - rect->left;
					height = rect->bottom - rect->top;
				}
				if (width && height) {
					_bitmap_handle = CreateCompatibleBitmap(src, width, height);
					if (_bitmap_handle) {
						QueryPerformanceCounter(&_capture_time);
						HBITMAP hOriginal = (HBITMAP)SelectObject(dc, _bitmap_handle);
						//Capture(wnd, dc);
						BitBlt(dc, 0, 0, width, height, src, left, top, SRCCOPY | CAPTUREBLT);
						SelectObject(dc, hOriginal);
					}
				}
				DeleteDC(dc);
			}
			ReleaseDC(wnd, src);
		}
		QueryPerformanceCounter(&_capture_time);
		wpt_capturing_screen = false;
	}
}

BOOL CapturedImage::Capture(HWND hwnd, HDC hdc) {
	typedef BOOL(WINAPI *tPrintWindow)(HWND, HDC, UINT);
	tPrintWindow printWindow = NULL;
	HINSTANCE handle = ::LoadLibrary("User32.dll");
	BOOL ret = FALSE;
	if (handle) {
		printWindow = (tPrintWindow)::GetProcAddress(handle, "PrintWindow");
		if (printWindow) {
			ret = printWindow(hwnd, hdc, 0);
		} else {
			ret = FALSE;
		}
	}
	::FreeLibrary(handle);
	return ret;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
CapturedImage::~CapturedImage() {}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
const CapturedImage& CapturedImage::operator =(const CapturedImage& src) {
	_bitmap_handle = src._bitmap_handle;
	_capture_time.QuadPart = src._capture_time.QuadPart;
	return src;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void CapturedImage::Free() {
	if (_bitmap_handle)
		DeleteObject(_bitmap_handle);
	_bitmap_handle = NULL;
}

/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
bool CapturedImage::Get(CxImage& image) {
	bool ret = false;

	if (_bitmap_handle)
		ret = image.CreateFromHBITMAP(_bitmap_handle);

	return ret;
}
