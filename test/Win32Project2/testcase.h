#pragma once

class TestCase {
public:
	TestCase();
	virtual ~TestCase(void);

	void init();
	void reset();

	CString _file_base;
	TCHAR* _browser_ex;
private:
	CRITICAL_SECTION cs;
};