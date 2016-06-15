#include "stdafx.h"
#include "screen_capture.h"
#include "shared_mem.h"
#include "cximage\ximage.h"

bool capture_screen = false;

ScreenCapture::ScreenCapture() :_viewport_set(false) {
	InitializeCriticalSection(&_cs);
	memset(&_viewPort, 0, sizeof(_viewPort));
}

ScreenCapture::~ScreenCapture(void) {
	reset();
	DeleteCriticalSection(&_cs);
}

void ScreenCapture::reset() {
	EnterCriticalSection(&_cs);
	while (!_captured_image_list.IsEmpty()) {
		CapturedImage& image = _captured_image_list.RemoveHead();
		image.free();
	}
	LeaveCriticalSection(&_cs);
}

/*-----------------------------------------------------------------------------
Capture a screen shot and save it in our list
-----------------------------------------------------------------------------*/
void ScreenCapture::capture(HWND wnd, bool cropViewPoint) {
	if (wnd) {
		EnterCriticalSection(&_cs);
		RECT* rect = NULL;
		if (cropViewPoint && _viewport_set) {
			rect = &_viewPort;
		}
		CapturedImage image(wnd, rect);
		_captured_image_list.AddTail(image);
		LeaveCriticalSection(&_cs);
	}
}

CapturedImage ScreenCapture::captureImage(HWND wnd, bool cropViewPoint) {
	CapturedImage ret;
	if (wnd) {
		EnterCriticalSection(&_cs);
		RECT* rect = NULL;
		if (cropViewPoint && _viewport_set) {
			rect = &_viewPort;
		}
		CapturedImage captured(wnd, rect);
		ret = captured;
		LeaveCriticalSection(&_cs);
	}
	return ret;
}

/*-----------------------------------------------------------------------------
Get the last image of the requested type
-----------------------------------------------------------------------------*/
bool ScreenCapture::getImage(CxImage& image) {
	bool ret = false;
	image.Destroy();
	EnterCriticalSection(&_cs);
	POSITION pos = _captured_image_list.GetHeadPosition();
	while (pos) {
		CapturedImage& captured_image = _captured_image_list.GetNext(pos);
		ret = captured_image.get(image);
	}
	LeaveCriticalSection(&_cs);
	return ret;
}

void ScreenCapture::lock() {
	EnterCriticalSection(&_cs);
}

void ScreenCapture::unlock() {
	LeaveCriticalSection(&_cs);
}

void ScreenCapture::clearViewPoint() {
	memset(&_viewPort, 0, sizeof(_viewPort));
	_viewport_set = false;
}

void ScreenCapture::setViewPort(RECT& viewPort) {
	memcpy(&_viewPort, &viewPort, sizeof(_viewPort));
	_viewport_set = true;
}

bool ScreenCapture::isViewPointSet() {
	return _viewport_set;
}

CapturedImage::CapturedImage() :_bitmap_handle(NULL) {
	_captureTime.QuadPart = 0;
}

CapturedImage::CapturedImage(HWND wnd, RECT* rect) : _bitmap_handle(NULL) {
	_captureTime.QuadPart = 0;
	if (wnd) {
		capture_screen = true;
		HDC src = GetDC(NULL);
		if (src) {
			HDC hdc = CreateCompatibleDC(src);
			if (hdc) {
				RECT windowRect;
				GetWindowRect(wnd, &windowRect);
				int left = windowRect.left;
				int top = windowRect.top;
				int width = abs(windowRect.right - left);
				int height = abs(windowRect.bottom - top);
				if (rect) {
					left += rect->left;
					top += rect->top;
					width = rect->right - rect->left;
					height = rect->bottom - rect->top;
				}
				if (width && height) {
					_bitmap_handle = CreateCompatibleBitmap(src, width, height);
					if (_bitmap_handle) {
						QueryPerformanceCounter(&_captureTime);
						HBITMAP hOriginal = (HBITMAP) SelectObject(hdc, _bitmap_handle);
						BitBlt(hdc, 0, 0, width, height, src, left, top, SRCCOPY | CAPTUREBLT);
						SelectObject(hdc, hOriginal);
					}
				}
				DeleteDC(hdc);
			}
			ReleaseDC(wnd, src);
		}
		QueryPerformanceCounter(&_captureTime);
		capture_screen = false;
	}
}

CapturedImage::~CapturedImage() {

}

const CapturedImage& CapturedImage::operator=(const CapturedImage& src) {
	_bitmap_handle = src._bitmap_handle;
	_captureTime.QuadPart = src._captureTime.QuadPart;
	return src;
}

void CapturedImage::free() {
	if (_bitmap_handle) {
		DeleteObject(_bitmap_handle);
	}
	_bitmap_handle = NULL;
}

bool CapturedImage::get(CxImage& image) {
	bool ret = false;
	if (_bitmap_handle) {
		ret = image.CreateFromHBITMAP(_bitmap_handle);
	}
	return ret;
}