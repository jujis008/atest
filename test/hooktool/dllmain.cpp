// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "hooktool.h"
#include "shared_mem.h"

HINSTANCE global_dll_handle = NULL; //dll handle
//extern WptHook * global_hook;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


static DWORD WINAPI hookThreadProc(void* args) {
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

}
