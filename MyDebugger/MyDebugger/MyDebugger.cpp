#include "MyDebugger.h"
#include "BreakPoint.h"
#include "BeaEngine.h"

BOOL MyDeBugger::IsSystemBreakPoint = TRUE;
LPVOID MyDeBugger::StartAddress = NULL;
HANDLE MyDeBugger::ProcessHandle = NULL;
HANDLE MyDeBugger::ThreadHandle = NULL;

// 提升进程权限
void MyDeBugger::DebugPrivilege()
{
	HANDLE hToken = NULL;
	//打开当前进程的访问令牌
	int hRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

	if (hRet)
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		//取得描述权限的LUID
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		//调整访问令牌的权限
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

		CloseHandle(hToken);
	}
}

// 以管理员的身份打开进程
void MyDeBugger::AdminOpenApp()
{
	// 1. 隐藏当前窗口
	::ShowWindow(::AfxGetMainWnd()->m_hWnd, SW_HIDE);
	// 2. 获取当前程序路径
	TCHAR szApplication[MAX_PATH] = { 0 };
	DWORD cchLength = _countof(szApplication);
	QueryFullProcessImageName(GetCurrentProcess(), 0, szApplication, &cchLength);
	// 3. 以管理员身份重新打开进程
	SHELLEXECUTEINFO esi = { sizeof(SHELLEXECUTEINFO) };
	esi.lpVerb = TEXT("runas"); // 请求提升权限
	esi.lpFile = szApplication; // 可执行文件路径
	esi.nShow = SW_SHOWNORMAL; // 正常显示窗口
	if (ShellExecuteEx(&esi))
	{
		ExitThread(0);
	}
	else
	{
		::ShowWindow(AfxGetMainWnd()->m_hWnd, SW_SHOWNORMAL);
	}
}

// 以创建进程进行调试
bool MyDeBugger::Open(const char * srcFile)
{
	if (srcFile == NULL)
	{
		return false;
	}

	bool bRet = false;
	STARTUPINFOA stcStartupInfo = { sizeof(STARTUPINFOA) };
	PROCESS_INFORMATION stcProcInfo = { 0 }; // 进程信息
	/* 创建调试进程 */
	bRet = CreateProcessA(
		srcFile, // 可执行文件路径
		NULL, // 命令行
		NULL, // 安全描述符
		NULL, // 线程属性是否可继承
		FALSE, // 是否继承进程句柄
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, // 以调试的方式启动
		NULL, // 新进程的环境块
		NULL, // 新进程的当前工作路径(当前目录)
		&stcStartupInfo, // 指定进程的主窗口特性
		&stcProcInfo // 接收新进程的识别信息
	);
}

// 以附加进程进行调试
bool MyDeBugger::Open(DWORD dwPid)
{
	// 注意需要管理员权限
	return DebugActiveProcess(dwPid);
}

// 等待系统调试事件, 第一层
void MyDeBugger::StartDebug()
{
	// 函数主要分有 3 个部分：
	// 1. 等待调试事件
	// 2. 处理调试事件
	// 3. 回复调试子系统
	DEBUG_EVENT dbgEvent = { 0 };
	DWORD dwRetCode = DBG_CONTINUE;
	
	// 1. 等待调试事件
	while (WaitForDebugEvent(&dbgEvent, INFINITE))
	{	
		// 1.1 获取线程句柄
		ThreadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, dbgEvent.dwThreadId);
		// 2. 分发消息进入第二层
		dwRetCode = DispatchEvent(&dbgEvent);
	}
	// 3. 回复调试子系统, 符过不回复进程会一直处于暂停状态
	ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, dwRetCode);
}

// 等待系统调试事件，第二层
DWORD MyDeBugger::DispatchEvent(DEBUG_EVENT * pDbgEvent)
{
	DWORD bRet = 0;
	// 2. 处理调试事件
	switch (pDbgEvent->dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT: /* 调试异常事件 */
	{
		bRet = DispatchException(&pDbgEvent->u.Exception);

		return bRet;
	}
	case CREATE_PROCESS_DEBUG_EVENT: // 创建进程之后的第一个调试事件
	{
		// 获取进程
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pDbgEvent->dwProcessId);
		// 获取线程开始位置(OEP)
		StartAddress = pDbgEvent->u.CreateProcessInfo.lpStartAddress;
	}

	// 2.1 其他异常事件
	default:
		return DBG_CONTINUE;
	}
}

//等待系统调试事件，第三层
DWORD MyDeBugger::DispatchException(EXCEPTION_DEBUG_INFO * pExcDbgInfo)
{
	// 获取产生的异常类型和异常产生的地址
	DWORD ExceptionCode = pExcDbgInfo->ExceptionRecord.ExceptionCode;
	LPVOID ExceptionAddr = pExcDbgInfo->ExceptionRecord.ExceptionAddress;

	// 根据不同的异常类型处理产生的异常
	switch (ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT: // 软件断点
	{
		// 第一次的INT 3 断点是由系统产生
		if (IsSystemBreakPoint)
		{
			IsSystemBreakPoint = FALSE;
			// 在OEP处设置断点
			BreakPoint::SetINT3BreakPoint(ProcessHandle, StartAddress);
		}	
	}
	break;

	case EXCEPTION_SINGLE_STEP: // 硬件断点 : tf(不需要修复的)，Dr0~Dr3硬件断点
	{
		BreakPoint::FixHarDwareBreakPoint(ThreadHandle);
	}
	break;

	case EXCEPTION_ACCESS_VIOLATION: // 内存访问断点
	{
		// 判断是否到达指定目标若没有继续设置内存断点
		DWORD Flag = FALSE; DWORD OldStyle = 0;
		VirtualProtectEx(ProcessHandle, ExceptionAddr, 1, PAGE_EXECUTE_READ, &OldStyle);
		if (BeaEngine::DisAsm(ProcessHandle, ExceptionAddr, BreakPoint::MemBreakPointList[0].Address))
		{
			// 找到指定目标将内存访问断点修复并删除
			BreakPoint::FixMemBreakPoint(ProcessHandle, ThreadHandle, ExceptionAddr);
			BreakPoint::MemBreakPointList.pop_back();
		}
		VirtualProtectEx(ProcessHandle, ExceptionAddr, 1, OldStyle, NULL);
	}
	break;

	default:
		return DBG_EXCEPTION_NOT_HANDLED; // 不能修复的异常
	}

	// 和用户交互
	//UserInput();

	return DBG_CONTINUE;
}

// 