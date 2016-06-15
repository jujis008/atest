#pragma once

class ScreenCapture;
class Trace;
class TestState {
public:
	~TestState(void);
	void start();
	void grabVideoFrame(bool force = false);
	void init();
	void reset();
	void titleSet(CString title);
	void updateBrowserWindow();
	void findBrowserNameAndVersion();
	void lock();
	void unlock();

	bool _active;
	HWND _frame_window;
	bool _no_gdi;
	bool _gdi_only;
	CString _title;
	CString _browser_name;
	CString _browser_version;
private:
	bool _started;

	ScreenCapture& _screen_capture;
	Trace& _trace;
	DWORD _video_capture_count;
	LARGE_INTEGER _last_video_time;
	LARGE_INTEGER _ms_frequency;
	CRITICAL_SECTION _data_cs;

	void done(bool force = false);
	void findViewPort(bool force = false);
};