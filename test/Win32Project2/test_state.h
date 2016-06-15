#pragma once

class Results;
class ScreenCapture;

class TestState {
public:
	TestState(Results& result, ScreenCapture& screen_capture);
	~TestState();

	void start();
	//bool isDone();
	void done(bool force = false);
	void grabVideoFrame(bool force = false);
	void collectData();
	void reset();
	void init();
	void titleSet(CString title);
	void updateBrowserWindow(TCHAR* browserName);
	DWORD elapsedMsFromStart(LARGE_INTEGER end) const;
	void findBrowserNameAndVersion();

	void lock();
	void unLock();
	void resizeBrowserForResponsiveTest();

	// times
	LARGE_INTEGER _start;
	LARGE_INTEGER _step_start;
	LARGE_INTEGER _ms_frequency;
	LARGE_INTEGER _title_time;
	SYSTEMTIME    _start_time;

	//Timeout measurer
	LARGE_INTEGER _timeout_start_time;
	CString _title;
	CString _browser_name;
	CString _browser_version;
	CString _user_agent;
	int _fixed_viewport;
	int _viewport_specified;
	bool _active;
	int _current_document;
	bool  _exit;

	CString tmp_file_base;

	HWND  _frame_window;

	bool no_gdi_;
	bool gdi_only_;
private:
	bool _started;
	Results& _results;
	ScreenCapture& _screen_capture;
	HANDLE  _data_timer;
	CString process_full_path_;
	CString process_base_exe_;
	CString last_title_;
	
	// tracking of the periodic data capture
	LARGE_INTEGER  _last_data;
	DWORD _video_capture_count;
	LARGE_INTEGER  _last_video_time;
	
	CRITICAL_SECTION  _data_cs;

	void checkTitle();
	void findViewport(bool force = false);
	void recordTime(CString time_name, DWORD time, LARGE_INTEGER * out_time);
	DWORD elapsedMs(LARGE_INTEGER start, LARGE_INTEGER end) const;
	//double GetElapsedMilliseconds(FILETIME &start, FILETIME &end);
};