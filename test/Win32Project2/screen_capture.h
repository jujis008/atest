#pragma once
class CxImage;

class CapturedImage {
public:
	CapturedImage();
	CapturedImage(const CapturedImage& src) { *this = src; }
	CapturedImage(HWND wnd, RECT * rect = NULL);
	BOOL Capture(HWND hwnd, HDC hdc);
	~CapturedImage();
	const CapturedImage& operator =(const CapturedImage& src);
	void Free();
	bool Get(CxImage& image);

	HBITMAP       _bitmap_handle;
	LARGE_INTEGER _capture_time;
};

class ScreenCapture {
public:
	ScreenCapture();
	~ScreenCapture(void);
	void Capture(HWND wnd, bool crop_viewport = true);
	void Capture(HWND wnd, CString file_base, bool crop_viewport = true);
	CapturedImage CaptureImage(HWND wnd, bool crop_viewport = true);
	bool GetImage(CxImage& image);
	void Lock();
	void Unlock();
	void Reset();
	void SetViewport(RECT& viewport);
	void ClearViewport();
	bool IsViewportSet();

	CAtlList<CapturedImage> _captured_images;
	//CAtlMap<CString, CAtlList<CapturedImage>> _capture_images_map;
	RECT _viewport;

private:
	CRITICAL_SECTION cs;
	bool _viewport_set;
};
