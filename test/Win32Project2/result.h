#pragma once
#include "ximage.h"

class TestState;
class ScreenCapture;

class Results {
public:
	Results(TestState& testState, ScreenCapture& screenCapture);
	~Results(void);

	void reset(void);
	void save();

	CString _label;
private:
	CString _file_base;
	TestState& _test_state;
	ScreenCapture& _screen_capture;
	bool _saved;
	LARGE_INTEGER _visually_complete;
	CRITICAL_SECTION cs;

	void saveVideo();
	void saveImage(CxImage& image, CString file, BYTE quality, bool force_small = false, bool full_size_video = false);
	bool imagesAreDifferent(CxImage* image1, CxImage* image2);
	//CStringA FormatTime(LARGE_INTEGER t);
};