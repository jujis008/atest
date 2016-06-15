#pragma once
class CxImage;
class CapturedImage {
public:
	CapturedImage(void);
	~CapturedImage(void);
	CapturedImage(const CapturedImage& src) {
		*this = src;
	}
	CapturedImage(HWND wnd, RECT* rect = NULL);
	const CapturedImage& operator =(const CapturedImage& src);
	void free();
	bool get(CxImage& image);
	HBITMAP _bitmap_handle;
	LARGE_INTEGER _captureTime;
};

class ScreenCapture {
public:
	ScreenCapture(void);
	~ScreenCapture(void);
	void capture(HWND wnd, bool cropViewPoint = true);
	CapturedImage captureImage(HWND wnd, bool cropViewPoint = true);
	bool getImage(CxImage& image);
	void lock();
	void unlock();
	void reset();
	void setViewPort(RECT& viewPort);
	void clearViewPoint();
	bool isViewPointSet();
	CAtlList<CapturedImage> _captured_image_list;
	RECT _viewPort;
private:
	CRITICAL_SECTION _cs;
	bool _viewport_set;
};