#include "BeaEngine.h"
#include <cstdio>
#include <atlstr.h>

DISASM BeaEngine::disAsm = { 0 };

BeaEngine::BeaEngine()
{
}


BeaEngine::~BeaEngine()
{
}

// ��ӡopcode
// const unsigned char* pOpcode �� opcode �Ļ������׵�ַ
// nSize �� opcdoe���ֽ���
void printOpcode(const unsigned char* pOpcode, int nSize)
{
	for (int i = 0; i < nSize; ++i)
	{
		printf("%02X ", pOpcode[i]);
	}
}

// �ڴ����
BOOL BeaEngine::DisAsm(HANDLE hProcess, LPVOID Addr, LPVOID TargetAddr)
{
	// ��ȡָ�����ȵ��ڴ�ռ�
	PCHAR buff = new CHAR[16]{ 0 };
	DWORD dwWrite = 0;
	ReadProcessMemory(hProcess, (LPVOID)Addr, buff, 16, &dwWrite);

	// ���ýṹ�壬��ʼ��������opcode
	disAsm.EIP = (UIntPtr)buff; // ����opcode�Ļ������׵�ַ
	disAsm.VirtualAddr = (DWORD)buff; // opcode ָ��ĵ�ַ
	disAsm.Archi = 0; // 0 => 32 , 1 => 64
	disAsm.Options = 0x000; // masm ���ָ���ʽ

	int nLen = 0; // ���ڼ�¼��ǰ�Ļ��ָ����ֽ���

	// ����Disasm�������з���࣬ 
	nLen = Disasm(&disAsm); // ÿ��ֻ�����һ�����ָ� ���ҷ��ص�ǰ�õ��Ļ��ָ��ĳ���

	CString StrBuf;
	StrBuf.Format(L"%x", TargetAddr);

	if (((CString)disAsm.CompleteInstr).Find(StrBuf) != -1)
	{
		// �����ַ��ָ���д���������ҵ�Ŀ��
		return TRUE;
	}
	else
		return FALSE;

}