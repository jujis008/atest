// hookTest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "iostream"
using namespace std;

int main() {
	for (;;) {
		for (int i = 0; i < 9600000; i++)
		{
			Sleep(10);
		}
	}
	return 0;
}

