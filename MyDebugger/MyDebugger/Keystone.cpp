#include "Keystone.h"

ks_engine * KeyStone::pengine = NULL;

// 初始化反汇编引擎
VOID KeyStone::Init()
{
	if (KS_ERR_OK != ks_open(KS_ARCH_X86, KS_MODE_32, &pengine))
	{
		printf("反汇编引擎初始化失败\n");
		return;
	}
}

//打印汇编指令
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
// 名称:  Keystone::PrtAsm
// 作用:  Print OpCode
// 继承:  public static 
// 返回:  VOID
// 参数:  HANDLE Handle // 目标进程句柄
// 参数:  LPVOID asmCode // 用户输出的汇编指令
// 参数:  LPVOID Addr // 需要修改的起始位置
//************************************
VOID KeyStone::PrtAsm(HANDLE Handle, LPVOID asmCode, LPVOID Addr)
{
	int nRet = 0; // 保存函数的返回值，用于判断函数是否执行成功
	size_t stat_count = 0; // 保存成功汇编的指令的条数
	unsigned char* opcode = NULL; // 汇编得到的opcode的缓冲区首地址
	unsigned int nOpcodeSize = 0; // 汇编出来的opcode的字节数

	// 修复分页属性
	DWORD OldProtect = 0;
	VirtualProtectEx(Handle, Addr, 1, PAGE_EXECUTE_READWRITE, &OldProtect);

	nRet = ks_asm(pengine, /* 汇编引擎句柄，通过ks_open函数得到*/
		(const char*)asmCode, /*要转换的汇编指令*/
		(DWORD)Addr, /*汇编指令所在的地址*/
		&opcode,/*输出的opcode*/
		&nOpcodeSize, /*输出的opcode的字节数*/
		&stat_count /*输出成功汇编的指令的条数*/);
	DWORD sizew = 0;
	WriteProcessMemory(Handle, Addr, opcode, nOpcodeSize, &sizew);
	// 还原代码的保护属性
	VirtualProtectEx(Handle, Addr, 1, OldProtect, &OldProtect);

	// 返回值等于-1时反汇编错误
	if (nRet == -1)
	{
		// 输出错误信息
		// ks_errno 获得错误码
		// ks_strerror 将错误码转换成字符串，并返回这个字符串
		printf("错误信息：%s\n", ks_strerror(ks_errno(pengine)));
		return;
	}

	// 打印汇编出来的opcode
	PrintOpcode(opcode, nOpcodeSize);

	// 释放空间
	ks_free(opcode);
	// 关闭句柄
	ks_close(pengine);
}

