#include "stdafx.h"
#include "Trace.h"

Trace::Trace(void) {
	InitializeCriticalSection(&_cs);
}

Trace::~Trace(void) {
	DeleteCriticalSection(&_cs);
}

void Trace::reset() {
	EnterCriticalSection(&_cs);
	_events.RemoveAll();
	LeaveCriticalSection(&_cs);
}

bool Trace::write(CString file) {
	bool ok = false;
	EnterCriticalSection(&_cs);
	HANDLE fileHandle = ::CreateFile(file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		ok = true;
		DWORD bytesWritten;
		bool first = true;
		CStringA eventString = "{\"traceEvents\": [";
		WriteFile(fileHandle, eventString, eventString.GetLength(), &bytesWritten, NULL);
		POSITION pos = _events.GetHeadPosition();
		while (pos) {
			eventString = _events.GetNext(pos);
			eventString.Trim("[]");
			if (eventString.GetLength()) {
				if (first) {
					first = false;
				} else {
					eventString = CStringA(",") + eventString;
					WriteFile(fileHandle, eventString, eventString.GetLength(), &bytesWritten, NULL);
				}
			}
			eventString = "]}";
			WriteFile(fileHandle, eventString, eventString.GetLength(), &bytesWritten, NULL);
			CloseHandle(fileHandle);
		}
	}
	LeaveCriticalSection(&_cs);
	return ok;
}

void Trace::addEvents(CStringA data) {
	EnterCriticalSection(&_cs);
	_events.AddTail(data);
	LeaveCriticalSection(&_cs);
}