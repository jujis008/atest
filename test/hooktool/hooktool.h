#pragma once
#include "windows.h"


class HooKTool {
public:
	HooKTool(void);
	~HooKTool(void);

	void init();
	void BackgroundThread();
	bool OnMessage(UINT message, WPARAM wParam, LPARAM lParam);

};

