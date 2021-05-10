#include "MyDebugger.h"
#include "BreakPoint.h"
#include "BeaEngine.h"

BOOL MyDeBugger::IsSystemBreakPoint = TRUE;
LPVOID MyDeBugger::StartAddress = NULL;
HANDLE MyDeBugger::ProcessHandle = NULL;
HANDLE MyDeBugger::ThreadHandle = NULL;

// ��������Ȩ��
void MyDeBugger::DebugPrivilege()
{
	HANDLE hToken = NULL;
	//�򿪵�ǰ���̵ķ�������
	int hRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

	if (hRet)
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		//ȡ������Ȩ�޵�LUID
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		//�����������Ƶ�Ȩ��
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

		CloseHandle(hToken);
	}
}

// �Թ���Ա����ݴ򿪽���
void MyDeBugger::AdminOpenApp()
{
	// 1. ���ص�ǰ����
	::ShowWindow(::AfxGetMainWnd()->m_hWnd, SW_HIDE);
	// 2. ��ȡ��ǰ����·��
	TCHAR szApplication[MAX_PATH] = { 0 };
	DWORD cchLength = _countof(szApplication);
	QueryFullProcessImageName(GetCurrentProcess(), 0, szApplication, &cchLength);
	// 3. �Թ���Ա������´򿪽���
	SHELLEXECUTEINFO esi = { sizeof(SHELLEXECUTEINFO) };
	esi.lpVerb = TEXT("runas"); // ��������Ȩ��
	esi.lpFile = szApplication; // ��ִ���ļ�·��
	esi.nShow = SW_SHOWNORMAL; // ������ʾ����
	if (ShellExecuteEx(&esi))
	{
		ExitThread(0);
	}
	else
	{
		::ShowWindow(AfxGetMainWnd()->m_hWnd, SW_SHOWNORMAL);
	}
}

// �Դ������̽��е���
bool MyDeBugger::Open(const char * srcFile)
{
	if (srcFile == NULL)
	{
		return false;
	}

	bool bRet = false;
	STARTUPINFOA stcStartupInfo = { sizeof(STARTUPINFOA) };
	PROCESS_INFORMATION stcProcInfo = { 0 }; // ������Ϣ
	/* �������Խ��� */
	bRet = CreateProcessA(
		srcFile, // ��ִ���ļ�·��
		NULL, // ������
		NULL, // ��ȫ������
		NULL, // �߳������Ƿ�ɼ̳�
		FALSE, // �Ƿ�̳н��̾��
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, // �Ե��Եķ�ʽ����
		NULL, // �½��̵Ļ�����
		NULL, // �½��̵ĵ�ǰ����·��(��ǰĿ¼)
		&stcStartupInfo, // ָ�����̵�����������
		&stcProcInfo // �����½��̵�ʶ����Ϣ
	);
}

// �Ը��ӽ��̽��е���
bool MyDeBugger::Open(DWORD dwPid)
{
	// ע����Ҫ����ԱȨ��
	return DebugActiveProcess(dwPid);
}

// �ȴ�ϵͳ�����¼�, ��һ��
void MyDeBugger::StartDebug()
{
	// ������Ҫ���� 3 �����֣�
	// 1. �ȴ������¼�
	// 2. ��������¼�
	// 3. �ظ�������ϵͳ
	DEBUG_EVENT dbgEvent = { 0 };
	DWORD dwRetCode = DBG_CONTINUE;
	
	// 1. �ȴ������¼�
	while (WaitForDebugEvent(&dbgEvent, INFINITE))
	{	
		// 1.1 ��ȡ�߳̾��
		ThreadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, dbgEvent.dwThreadId);
		// 2. �ַ���Ϣ����ڶ���
		dwRetCode = DispatchEvent(&dbgEvent);
	}
	// 3. �ظ�������ϵͳ, �������ظ����̻�һֱ������ͣ״̬
	ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, dwRetCode);
}

// �ȴ�ϵͳ�����¼����ڶ���
DWORD MyDeBugger::DispatchEvent(DEBUG_EVENT * pDbgEvent)
{
	DWORD bRet = 0;
	// 2. ��������¼�
	switch (pDbgEvent->dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT: /* �����쳣�¼� */
	{
		bRet = DispatchException(&pDbgEvent->u.Exception);

		return bRet;
	}
	case CREATE_PROCESS_DEBUG_EVENT: // ��������֮��ĵ�һ�������¼�
	{
		// ��ȡ����
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pDbgEvent->dwProcessId);
		// ��ȡ�߳̿�ʼλ��(OEP)
		StartAddress = pDbgEvent->u.CreateProcessInfo.lpStartAddress;
	}

	// 2.1 �����쳣�¼�
	default:
		return DBG_CONTINUE;
	}
}

//�ȴ�ϵͳ�����¼���������
DWORD MyDeBugger::DispatchException(EXCEPTION_DEBUG_INFO * pExcDbgInfo)
{
	// ��ȡ�������쳣���ͺ��쳣�����ĵ�ַ
	DWORD ExceptionCode = pExcDbgInfo->ExceptionRecord.ExceptionCode;
	LPVOID ExceptionAddr = pExcDbgInfo->ExceptionRecord.ExceptionAddress;

	// ���ݲ�ͬ���쳣���ʹ���������쳣
	switch (ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT: // ����ϵ�
	{
		// ��һ�ε�INT 3 �ϵ�����ϵͳ����
		if (IsSystemBreakPoint)
		{
			IsSystemBreakPoint = FALSE;
			// ��OEP�����öϵ�
			BreakPoint::SetINT3BreakPoint(ProcessHandle, StartAddress);
		}	
	}
	break;

	case EXCEPTION_SINGLE_STEP: // Ӳ���ϵ� : tf(����Ҫ�޸���)��Dr0~Dr3Ӳ���ϵ�
	{
		BreakPoint::FixHarDwareBreakPoint(ThreadHandle);
	}
	break;

	case EXCEPTION_ACCESS_VIOLATION: // �ڴ���ʶϵ�
	{
		// �ж��Ƿ񵽴�ָ��Ŀ����û�м��������ڴ�ϵ�
		DWORD Flag = FALSE; DWORD OldStyle = 0;
		VirtualProtectEx(ProcessHandle, ExceptionAddr, 1, PAGE_EXECUTE_READ, &OldStyle);
		if (BeaEngine::DisAsm(ProcessHandle, ExceptionAddr, BreakPoint::MemBreakPointList[0].Address))
		{
			// �ҵ�ָ��Ŀ�꽫�ڴ���ʶϵ��޸���ɾ��
			BreakPoint::FixMemBreakPoint(ProcessHandle, ThreadHandle, ExceptionAddr);
			BreakPoint::MemBreakPointList.pop_back();
		}
		VirtualProtectEx(ProcessHandle, ExceptionAddr, 1, OldStyle, NULL);
	}
	break;

	default:
		return DBG_EXCEPTION_NOT_HANDLED; // �����޸����쳣
	}

	// ���û�����
	//UserInput();

	return DBG_CONTINUE;
}

// 