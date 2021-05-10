#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
/* 调试器项目 */

class MyDeBugger
{
public:
	// 提升进程权限
	static void DebugPrivilege();
	// 以管理员的身份打开进程
	static void AdminOpenApp();
	// 以创建进程进行调试
	static bool Open(const char * srcFile);
	// 以附加进程进行调试
	static bool Open(DWORD dwPid);
	// 等待系统调试事件, 第一层
	static void StartDebug();
	// 等待系统调试事件，第二层
	static DWORD DispatchEvent(DEBUG_EVENT * pDbgEvent);
	// 等待系统调试事件，第三层
	static DWORD DispatchException(EXCEPTION_DEBUG_INFO * pExcDbgInfo);

protected:
	// 用于判断是否是由系统产生的断点
	static BOOL IsSystemBreakPoint;
	// 保存获取的OEP
	static LPVOID StartAddress;
	// 保存产生异常的进程的句柄
	static HANDLE ProcessHandle;
	// 保存产生异常的线程的句柄
	static HANDLE ThreadHandle;
};
