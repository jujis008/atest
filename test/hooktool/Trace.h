#pragma once
class Trace {
public:
	Trace(void);
	~Trace(void);
	void reset();
	bool write(CString file);
	void addEvents(CStringA data);

private:
	CRITICAL_SECTION _cs;
	CAtlList<CStringA> _events;
};