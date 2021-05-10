#pragma once

// 1. ����ͷ�ļ�
#include "Keystone/keystone.h"
// 2. ������̬��
#pragma comment (lib, "Keystone/x86/keystone_x86.lib")

#include <windows.h>

class KeyStone
{
public:
	KeyStone();
	~KeyStone();

private:
	static ks_engine *pengine;

public:
	// ��ʼ�����������
	static VOID Init();
	// ��ӡ���ָ��
	static VOID PrintOpcode(const unsigned char* pOpcode, int nSize);
	// ת��Ӳ����Ϊ���ָ��
	static VOID PrtAsm(HANDLE Handle, LPVOID asmCode, LPVOID Addr);
};
