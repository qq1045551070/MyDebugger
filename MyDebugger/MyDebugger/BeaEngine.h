#pragma once
#include <windows.h>
// ������º�
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
// 1. ����BegEngine��ͷ�ļ�
//	Win32 ��32λƽ̨�ĳ������ʹ�õ�ͷ�ļ�
//	Win64 ��64λƽ̨�ĳ������ʹ�õ�ͷ�ļ�
#include "BeaEngine_4.1/Win32/headers/BeaEngine.h"

//2. ������Ӧ�汾�ľ�̬��
#pragma comment (lib , "BeaEngine_4.1/Win32/Win32/Lib/BeaEngine.lib")

// ��ֹ�������
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")


class BeaEngine
{
public:
	BeaEngine();
	~BeaEngine();

public:
	// ������ָ��������ָ��
	static BOOL DisAsm(HANDLE hProcess, LPVOID Addr, LPVOID TargetAddr);

private:
	static DISASM disAsm;
};

