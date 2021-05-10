#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
/* ��������Ŀ */

class MyDeBugger
{
public:
	// ��������Ȩ��
	static void DebugPrivilege();
	// �Թ���Ա����ݴ򿪽���
	static void AdminOpenApp();
	// �Դ������̽��е���
	static bool Open(const char * srcFile);
	// �Ը��ӽ��̽��е���
	static bool Open(DWORD dwPid);
	// �ȴ�ϵͳ�����¼�, ��һ��
	static void StartDebug();
	// �ȴ�ϵͳ�����¼����ڶ���
	static DWORD DispatchEvent(DEBUG_EVENT * pDbgEvent);
	// �ȴ�ϵͳ�����¼���������
	static DWORD DispatchException(EXCEPTION_DEBUG_INFO * pExcDbgInfo);

protected:
	// �����ж��Ƿ�����ϵͳ�����Ķϵ�
	static BOOL IsSystemBreakPoint;
	// �����ȡ��OEP
	static LPVOID StartAddress;
	// ��������쳣�Ľ��̵ľ��
	static HANDLE ProcessHandle;
	// ��������쳣���̵߳ľ��
	static HANDLE ThreadHandle;
};
