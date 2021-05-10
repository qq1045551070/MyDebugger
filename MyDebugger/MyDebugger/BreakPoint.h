#pragma once

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;
/* �ϵ���� */

// DR7 �ṹ��
typedef struct _DBG_REG7
{
	/*
		// �ֲ��ϵ�(L0~3)��ȫ�ֶϵ�(G0~3)�ı��λ
	*/
	unsigned L0 : 1;  // ��Dr0����ĵ�ַ���� �ֲ��ϵ�
	unsigned G0 : 1;  // ��Dr0����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L1 : 1;  // ��Dr1����ĵ�ַ���� �ֲ��ϵ�
	unsigned G1 : 1;  // ��Dr1����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L2 : 1;  // ��Dr2����ĵ�ַ���� �ֲ��ϵ�
	unsigned G2 : 1;  // ��Dr2����ĵ�ַ���� ȫ�ֶϵ�
	unsigned L3 : 1;  // ��Dr3����ĵ�ַ���� �ֲ��ϵ�
	unsigned G3 : 1;  // ��Dr3����ĵ�ַ���� ȫ�ֶϵ�

	/*
	// �������á����ڽ���CPUƵ�ʣ��Է���׼ȷ���ϵ��쳣
	*/
	unsigned LE : 1;
	unsigned GE : 1;
	/*
	// �����ֶ�
	*/
	unsigned Reserve1 : 3;
	/*
	// �������ԼĴ�����־λ�������λΪ1������ָ���޸ĵ��ԼĴ���ʱ�ᴥ���쳣
	*/
	unsigned GD : 1;
	/*
	// �����ֶ�
	*/
	unsigned Reserve2 : 2;
	/*
	// ����Dr0~Dr3��ַ��ָ��λ�õĶϵ�����(RW0~3)��ϵ㳤��(LEN0~3)��״̬�������£�
	*/
	unsigned RW0 : 2;  // �趨Dr0ָ���ַ�Ķϵ�����
	unsigned LEN0 : 2;  // �趨Dr0ָ���ַ�Ķϵ㳤��
	unsigned RW1 : 2;  // �趨Dr1ָ���ַ�Ķϵ�����
	unsigned LEN1 : 2;  // �趨Dr1ָ���ַ�Ķϵ㳤��
	unsigned RW2 : 2;  // �趨Dr2ָ���ַ�Ķϵ�����
	unsigned LEN2 : 2;  // �趨Dr2ָ���ַ�Ķϵ㳤��
	unsigned RW3 : 2;  // �趨Dr3ָ���ַ�Ķϵ�����
	unsigned LEN3 : 2;  // �趨Dr3ָ���ַ�Ķϵ㳤��
}DBG_REG7, *PDBG_REG7;

// �ϵ���Ϣ�ṹ��
struct BreakPointInfo
{
	LPVOID Address; // �ϵ�ĵ�ַ
	DWORD ThisStyle;
	BYTE OpCode; // ԭʼ��1�ֽ�
};

class BreakPoint
{
public:
	static BOOL barwflag;
	// CC�ϵ���Ϣ�б�
	static vector<BreakPointInfo> CCBreakPointList;
	// �ڴ���ʶϵ�List�� ����ֻ����һ��
	static vector<BreakPointInfo> MemBreakPointList;

public:
	// ����Int3�ϵ�
	static BOOL SetINT3BreakPoint(HANDLE Process, LPVOID Address);
	// �޸�Int3�ϵ�
	static VOID FixINT3BreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// ����Ӳ��ִ�жϵ�
	static BOOL SetHarDwareBreakPoint(HANDLE Thread, LPVOID Address);
	// �޸�Ӳ��ִ�жϵ�
	static VOID FixHarDwareBreakPoint(HANDLE Thread);
	// ����Ӳ����д�ϵ�
	static BOOL SetHarDwareRwBreakPoint(HANDLE hThread, ULONG_PTR uAddress, DWORD type, DWORD dwLen);
	// �޸�Ӳ����д�ϵ�
	static VOID FixHarDwareRwBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag);
	// ����TF�ϵ㣨�������룩�ϵ�
	static VOID SetTfBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag);
	// ���õ��������ϵ�
	static VOID SetStepOverBreakPoint(HANDLE Process, HANDLE Thread, DWORD Count);
	// �����ڴ���ʶϵ�
	static VOID SetMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// �޸��ڴ���ʶϵ�
	static VOID FixMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// API�ϵ�
	static BOOL APIBreakPiont(HANDLE NowProcess, CHAR * APIName, DWORD ImageBase, DWORD FileMSize);
	// ��ȡ�Ĵ�����ǰ��Ϣ
	static VOID GetCurrentThreadInfo(HANDLE Thread);
};
