#ifndef _MODULEINSTANCE_H_
#define _MODULEINSTANCE_H_
#if _MSC_VER>1000
#pragma once
#endif // _MSC_VER>1000

#endif // !_MODULEINSTANCE_H_

#include "iostream"
#include "vector"
#include "windows.h"
#include "tlhelp32.h"

using namespace std;

class CModuleInstance;
class CModuleList;
class CRunningProcess;
class CExeModuleInstance;

//---------------------------------------------------------------------------
//
//                   typedefs for ToolHelp32 functions 
//
// We must link to these functions of Kernel32.DLL explicitly. Otherwise 
// a module using this code would fail to load under Windows NT, which does not 
// have the Toolhelp32 functions in the Kernel 32.
//
//---------------------------------------------------------------------------
typedef HANDLE(WINAPI * PFNCREATETOOLHELP32SNAPSHOT) (DWORD dwFlags, DWORD th32ProcessId);
typedef BOOL(WINAPI * PFNPROCESS32FIRST) (HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL(WINAPI * PFNPROCESS32NEXT) (HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef	BOOL(WINAPI * PFNMODULE32FIRST) (HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL(WINAPI * PFNMODULE32NEXT) (HANDLE hSnapshot, LPMODULEENTRY32 lpme);


//---------------------------------------------------------------------------
//
//                   typedefs for PSAPI.DLL functions 
//
//---------------------------------------------------------------------------
typedef BOOL(WINAPI * PFNENUMPROCESSES) (DWORD * lpidProcesses, DWORD cb, DWORD cbNeeded);
typedef BOOL(WINAPI * PFNENUMPROCESSMODULES) (HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef BOOL(WINAPI * PFNGETMODULEFILENAMEEXA) (HANDLE hProcess, HMODULE hModule, LPSTR *lpFilename, DWORD nSize);

class CModuleList :private std::vector<CModuleInstance*> {
public:
	CModuleList();
	~CModuleList();
	void add(CModuleInstance &moduleInstance);
	void releaseAll();
	CModuleInstance* getModule(DWORD dwIndex) const;
	DWORD getCount() const;
};

class CElement : public CModuleList {
public:
	CElement();
	virtual ~CElement();
};

class CLibHandler {
public:
	CLibHandler(CRunningProcess* pProcess);
	~CLibHandler();
	virtual BOOL populateModules(CModuleInstance* pProcess) = 0;
	virtual BOOL populateProcesses(BOOL bPopulateModules) = 0;
	virtual BOOL populatePorcess(DWORD dwProcessId, BOOL bPopulateModules) = 0;

protected:
	CRunningProcess* pRunningProcess;
};

class CTaskManager {
public:
	CTaskManager();
	virtual ~CTaskManager();

	BOOL populate(BOOL bPopulateModules);
	BOOL populateProcess(DWORD dwProcessId, BOOL populateModules);
	DWORD getProcessCount() const;
	CExeModuleInstance* getProcessByIndex(DWORD dwIndex);
	CExeModuleInstance* getProcessById(DWORD dwProcessId);

private:
	CLibHandler* m_pLibHandler;
	CRunningProcess* m_pProcesses;
};

class CLoadedModules :public CElement {
public:
	CLoadedModules(DWORD dwProcessId);
	virtual ~CLoadedModules();
protected:
	DWORD m_dwProcessId;
};

class CRunningProcess :public CElement {
public:
	CRunningProcess();
	virtual ~CRunningProcess();
	CExeModuleInstance* getProcessById(DWORD dwProcessId);
};

class CPsapiHandler :public CLibHandler {
public:
	CPsapiHandler(CRunningProcess* pProcesses);
	virtual ~CPsapiHandler();

	BOOL initialize();
	void finalize();
	virtual BOOL populateModules(CModuleInstance* pProcess);
	virtual BOOL populateProcesses(BOOL pPopulateModules);
	virtual BOOL populatePorcess(DWORD dwProcessId, BOOL bPopulateModules);

private:
	HMODULE m_hModPSAPI;
	PFNENUMPROCESSES m_pfnEnumProcesses;
	PFNENUMPROCESSMODULES m_pfnEnumProcessModules;
	PFNGETMODULEFILENAMEEXA m_pfnGetModuleFileNameExA;
};

class CToolHelperHandler :public CLibHandler {
public:
	CToolHelperHandler(CRunningProcess* pProcesses);
	virtual ~CToolHelperHandler();
	BOOL initialize();
	virtual BOOL populateModules(CModuleInstance* pProcess);
	virtual BOOL populateProcesses(BOOL bPopulateModules);
	virtual BOOL populateProcess(DWORD dwProcessId, BOOL bPopulateModules);
private:
	BOOL moduleFirst(HANDLE hSnapshot, PMODULEENTRY32 pme32) const;
	BOOL moduleNext(HANDLE hSnapshot, PMODULEENTRY32 pme32) const;
	BOOL processFirst(HANDLE hSnapshot, PROCESSENTRY32* pe32) const;
	BOOL processNext(HANDLE hSnapshot, PROCESSENTRY32* pe32) const;

	PFNCREATETOOLHELP32SNAPSHOT m_pfnCreateToolhelp32Snapshot;
	PFNPROCESS32FIRST m_pfnProcess32First;
	PFNPROCESS32NEXT m_pfnProcess32Next;
	PFNMODULE32FIRST m_pfnModule32First;
	PFNMODULE32NEXT m_pfnModule32Next;
};

class CModuleInstance {
public:
	CModuleInstance(char* pszName, HMODULE hModule);
	virtual ~CModuleInstance();
	void addModule(CModuleInstance* pModuleInstance);
	void releaseModules();
	char* getName() const;
	void setName(char* pszName);
	HMODULE getModule() const;
	void setModule(HMODULE module);
	char* getBaseName() const;
private:
	char* m_pszName;
	HMODULE m_hModule;
protected:
	CModuleList* m_pInternalList;
};

class CExeModuleInstance : public CModuleInstance {
public:
	CExeModuleInstance(
		CLibHandler* pLibHandler,
		char* pszName,
		HMODULE hModule,
		DWORD dwProcessId
		);
	virtual ~CExeModuleInstance();

	DWORD getProcessId() const;
	BOOL populateModules();
	DWORD getModuleCount();
	CModuleInstance* getModuleByIndex(DWORD dwIndex);
private:
	DWORD m_dwProcessId;
	CLibHandler* m_pLibHandler;
};