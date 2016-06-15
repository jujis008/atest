// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WIN32PROJECT2_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WIN32PROJECT2_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WRAPPER_EXPORTS
#define WRAPPER_API __declspec(dllexport)
#else
#define WRAPPER_API __declspec(dllimport)
#endif
#include "util.h"
#include "test_state.h"
#include "result.h"
#include "screen_capture.h"
//#include "testcase.h"

extern HINSTANCE global_dll_handle; // DLL handle
// This class is exported from the Win32Project2.dll
class WRAPPER_API wrapper {
public:
	wrapper(const TCHAR* file_base, const TCHAR* browser);
	~wrapper();
	void wrapper::init();
	void start();
	void stop();
private:
	TestState     test_state_;
	Results       results_;
	ScreenCapture screen_capture_;
};

//extern WRAPPER_API int nWrapper;

//WRAPPER_API int fnWrapper(void);
