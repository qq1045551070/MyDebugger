#pragma once

// 1. 包含头文件
#include "Keystone/keystone.h"
// 2. 包含静态库
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
	// 初始话反汇编引擎
	static VOID Init();
	// 打印汇编指令
	static VOID PrintOpcode(const unsigned char* pOpcode, int nSize);
	// 转换硬编码为汇编指令
	static VOID PrtAsm(HANDLE Handle, LPVOID asmCode, LPVOID Addr);
};
