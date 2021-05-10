#pragma once
#include <windows.h>
// 添加以下宏
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
// 1. 包含BegEngine的头文件
//	Win32 是32位平台的程序可以使用的头文件
//	Win64 是64位平台的程序可以使用的头文件
#include "BeaEngine_4.1/Win32/headers/BeaEngine.h"

//2. 包含对应版本的静态库
#pragma comment (lib , "BeaEngine_4.1/Win32/Win32/Lib/BeaEngine.lib")

// 防止编译错误。
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")


class BeaEngine
{
public:
	BeaEngine();
	~BeaEngine();

public:
	// 反汇编出指定条数的指令
	static BOOL DisAsm(HANDLE hProcess, LPVOID Addr, LPVOID TargetAddr);

private:
	static DISASM disAsm;
};

