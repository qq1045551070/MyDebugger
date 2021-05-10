#include "Keystone.h"

ks_engine * KeyStone::pengine = NULL;

// ��ʼ�����������
VOID KeyStone::Init()
{
	if (KS_ERR_OK != ks_open(KS_ARCH_X86, KS_MODE_32, &pengine))
	{
		printf("����������ʼ��ʧ��\n");
		return;
	}
}

//��ӡ���ָ��
VOID KeyStone::PrintOpcode(const unsigned char* pOpcode, int nSize)
{
	for (int i = 0; i < nSize; ++i)
	{
		printf("%02X ", pOpcode[i]);
	}
	putchar('\n');
	putchar('\n');
}


//************************************
// ����:  Keystone::PrtAsm
// ����:  Print OpCode
// �̳�:  public static 
// ����:  VOID
// ����:  HANDLE Handle // Ŀ����̾��
// ����:  LPVOID asmCode // �û�����Ļ��ָ��
// ����:  LPVOID Addr // ��Ҫ�޸ĵ���ʼλ��
//************************************
VOID KeyStone::PrtAsm(HANDLE Handle, LPVOID asmCode, LPVOID Addr)
{
	int nRet = 0; // ���溯���ķ���ֵ�������жϺ����Ƿ�ִ�гɹ�
	size_t stat_count = 0; // ����ɹ�����ָ�������
	unsigned char* opcode = NULL; // ���õ���opcode�Ļ������׵�ַ
	unsigned int nOpcodeSize = 0; // ��������opcode���ֽ���

	// �޸���ҳ����
	DWORD OldProtect = 0;
	VirtualProtectEx(Handle, Addr, 1, PAGE_EXECUTE_READWRITE, &OldProtect);

	nRet = ks_asm(pengine, /* �����������ͨ��ks_open�����õ�*/
		(const char*)asmCode, /*Ҫת���Ļ��ָ��*/
		(DWORD)Addr, /*���ָ�����ڵĵ�ַ*/
		&opcode,/*�����opcode*/
		&nOpcodeSize, /*�����opcode���ֽ���*/
		&stat_count /*����ɹ�����ָ�������*/);
	DWORD sizew = 0;
	WriteProcessMemory(Handle, Addr, opcode, nOpcodeSize, &sizew);
	// ��ԭ����ı�������
	VirtualProtectEx(Handle, Addr, 1, OldProtect, &OldProtect);

	// ����ֵ����-1ʱ��������
	if (nRet == -1)
	{
		// ���������Ϣ
		// ks_errno ��ô�����
		// ks_strerror ��������ת�����ַ���������������ַ���
		printf("������Ϣ��%s\n", ks_strerror(ks_errno(pengine)));
		return;
	}

	// ��ӡ��������opcode
	PrintOpcode(opcode, nOpcodeSize);

	// �ͷſռ�
	ks_free(opcode);
	// �رվ��
	ks_close(pengine);
}

