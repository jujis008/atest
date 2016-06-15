#include "stdafx.h"
#include "result.h"
#include "test_state.h"
#include "util.h"
#include "screen_capture.h"
#include "shared_mem.h"

static const BYTE JPEG_DEFAULT_QUALITY = 30;
static const DWORD RIGHT_MARGIN = 25;
static const DWORD BOTTOM_MARGIN = 25;

Results::Results(TestState& testState, ScreenCapture& screenCapture) :
	_test_state(testState)
	, _screen_capture(screenCapture)
	,_saved(false){
	_visually_complete.QuadPart = 0;
	_file_base = shared_results_file_base;
	WptTrace(loglevel::kFunction, _T("[wpthook] - Results base file: %s"), (LPCTSTR)_file_base);
	InitializeCriticalSection(&cs);
}

Results::~Results() {
	DeleteCriticalSection(&cs);
}

void Results::reset(void) {
	_screen_capture.Reset();
	_saved = false;
	_visually_complete.QuadPart = 0;
}

void Results::save() {
	WptTrace(loglevel::kFunction, _T("[wpthook] - Results::Save()\n"));
	saveVideo();
}


/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
bool Results::imagesAreDifferent(CxImage * img1, CxImage* img2) {
	bool different = false;
	if (img1 && img2 && img1->GetWidth() == img2->GetWidth() &&
		img1->GetHeight() == img2->GetHeight() &&
		img1->GetBpp() == img2->GetBpp()) {
		if (img1->GetBpp() >= 15) {
			DWORD pixel_bytes = 3;
			if (img1->GetBpp() == 32)
				pixel_bytes = 4;
			DWORD width = max(img1->GetWidth() - RIGHT_MARGIN, 0);
			DWORD height = img1->GetHeight();
			DWORD row_bytes = img1->GetEffWidth();
			DWORD row_length = width * (DWORD)(row_bytes / width);
			for (DWORD row = BOTTOM_MARGIN; row < height && !different; row++) {
				BYTE * r1 = img1->GetBits(row);
				BYTE * r2 = img2->GetBits(row);
				if (r1 && r2 && memcmp(r1, r2, row_length))
					different = true;
			}
		}
	}
	else
		different = true;
	return different;
}

void Results::saveImage(CxImage& image, CString file, BYTE quality, bool force_small, bool full_size_video) {
	if (image.IsValid()) {
		CxImage img(image);
		if (!full_size_video) {
			if (force_small || (img.GetWidth() > 600 && img.GetHeight() > 600)) {
				img.Resample2(img.GetWidth() / 2, img.GetHeight() / 2);
			}
		}
		img.SetCodecOption(8, CXIMAGE_FORMAT_JPG);  // optimized encoding
		img.SetCodecOption(16, CXIMAGE_FORMAT_JPG); // progressive
		img.SetJpegQuality((BYTE)quality);
		img.Save(file, CXIMAGE_FORMAT_JPG);
	}
}

void Results::saveVideo() {
	_screen_capture.Lock();
	CxImage* last_image = NULL;
	DWORD width, height;
	CString file_name;
	CString str;
	if (_screen_capture._captured_images.IsEmpty()) {
		return;
	}
	int count = _screen_capture._captured_images.GetCount();
	str.Format("%i", count);
	/*if (strcmp(shared_results_file_base, "")!=0) {
		lstrcpy((LPSTR)(LPCSTR)_file_base, shared_results_file_base);
	} else {
		lstrcpy((LPSTR)(LPCSTR)_file_base, _test_state.tmp_file_base);
	}*/

	/*CString cstr;
	cstr.Format("%s", _test_state.tmp_file_base);*/
	MessageBox(NULL, _file_base, str, 0);

	//iterator tmpImageList save image to file
	POSITION pos = _screen_capture._captured_images.GetHeadPosition();
	int counter = 0;
	while (pos) {
		CapturedImage& image = _screen_capture._captured_images.GetNext(pos);
		CxImage* img = new CxImage;
		counter++;
		if (image.Get(*img)) {
			DWORD image_time_ms = _test_state.elapsedMsFromStart(image._capture_time);
			// we save the frames in increments of 100ms (for now anyway)
			// round it to the closest interval
			DWORD image_time = ((image_time_ms + 50) / 100);
			if (last_image) {
				RGBQUAD black = { 0,0,0,0 };
				if (img->GetWidth() > width)
					img->Crop(0, 0, img->GetWidth() - width, 0);
				if (img->GetHeight() > height)
					img->Crop(0, 0, 0, img->GetHeight() - height);
				if (img->GetWidth() < width)
					img->Expand(0, 0, width - img->GetWidth(), 0, black);
				if (img->GetHeight() < height)
					img->Expand(0, 0, 0, height - img->GetHeight(), black);
				if (strcmp(_file_base, shared_results_file_base) != 0) { //for shared result file base changed.
					_file_base = shared_results_file_base;
				}
				if (imagesAreDifferent(last_image, img)) {
					//if (!_test_state._render_start.QuadPart)
						//_test_state._render_start.QuadPart = image._capture_time.QuadPart;
					//if (_test._video) {
					_visually_complete.QuadPart = image._capture_time.QuadPart;
					file_name.Format(_T("%s_progress_%04d.png"),  _file_base, image_time);
					saveImage(*img, file_name, JPEG_DEFAULT_QUALITY, false, true);//_full_size_video);
				//}
				} else {
					if (counter > 0 && counter == count) {
						file_name.Format(_T("%s_progress_%04d.png"), _file_base, image_time);
						saveImage(*img, file_name, JPEG_DEFAULT_QUALITY, false, true);
					}
				}
			} else {
				width = img->GetWidth();
				height = img->GetHeight();
				// always save the first image at time zero
				image_time = 0;
				image_time_ms = 0;
				file_name = _file_base + _T("_progress_0000.png");
				saveImage(*img, file_name, JPEG_DEFAULT_QUALITY, false, true);//_test._full_size_video);
			}
			if (last_image)
				delete last_image;
			last_image = img;
		}
		else
			delete img;
	}
	if (last_image) {
		delete last_image;
	}
	reset();
	_screen_capture.Unlock();
}