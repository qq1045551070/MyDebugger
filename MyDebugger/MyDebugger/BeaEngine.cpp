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

// 打印opcode
// const unsigned char* pOpcode ： opcode 的缓冲区首地址
// nSize ： opcdoe的字节数
void printOpcode(const unsigned char* pOpcode, int nSize)
{
	for (int i = 0; i < nSize; ++i)
	{
		printf("%02X ", pOpcode[i]);
	}
}

// 内存遍历
BOOL BeaEngine::DisAsm(HANDLE hProcess, LPVOID Addr, LPVOID TargetAddr)
{
	// 读取指定长度的内存空间
	PCHAR buff = new CHAR[16]{ 0 };
	DWORD dwWrite = 0;
	ReadProcessMemory(hProcess, (LPVOID)Addr, buff, 16, &dwWrite);

	// 配置结构体，初始化反汇编的opcode
	disAsm.EIP = (UIntPtr)buff; // 保存opcode的缓冲区首地址
	disAsm.VirtualAddr = (DWORD)buff; // opcode 指令的地址
	disAsm.Archi = 0; // 0 => 32 , 1 => 64
	disAsm.Options = 0x000; // masm 汇编指令格式

	int nLen = 0; // 用于记录当前的汇编指令的字节数

	// 调用Disasm（）进行反汇编， 
	nLen = Disasm(&disAsm); // 每次只反汇编一条汇编指令， 并且返回当前得到的汇编指令的长度

	CString StrBuf;
	StrBuf.Format(L"%x", TargetAddr);

	if (((CString)disAsm.CompleteInstr).Find(StrBuf) != -1)
	{
		// 如果地址在指令中代表，则代表找到目标
		return TRUE;
	}
	else
		return FALSE;

}