#pragma once

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;
/* 断点管理 */

// DR7 结构体
typedef struct _DBG_REG7
{
	/*
		// 局部断点(L0~3)与全局断点(G0~3)的标记位
	*/
	unsigned L0 : 1;  // 对Dr0保存的地址启用 局部断点
	unsigned G0 : 1;  // 对Dr0保存的地址启用 全局断点
	unsigned L1 : 1;  // 对Dr1保存的地址启用 局部断点
	unsigned G1 : 1;  // 对Dr1保存的地址启用 全局断点
	unsigned L2 : 1;  // 对Dr2保存的地址启用 局部断点
	unsigned G2 : 1;  // 对Dr2保存的地址启用 全局断点
	unsigned L3 : 1;  // 对Dr3保存的地址启用 局部断点
	unsigned G3 : 1;  // 对Dr3保存的地址启用 全局断点

	/*
	// 【以弃用】用于降低CPU频率，以方便准确检测断点异常
	*/
	unsigned LE : 1;
	unsigned GE : 1;
	/*
	// 保留字段
	*/
	unsigned Reserve1 : 3;
	/*
	// 保护调试寄存器标志位，如果此位为1，则有指令修改调试寄存器时会触发异常
	*/
	unsigned GD : 1;
	/*
	// 保留字段
	*/
	unsigned Reserve2 : 2;
	/*
	// 保存Dr0~Dr3地址所指向位置的断点类型(RW0~3)与断点长度(LEN0~3)，状态描述如下：
	*/
	unsigned RW0 : 2;  // 设定Dr0指向地址的断点类型
	unsigned LEN0 : 2;  // 设定Dr0指向地址的断点长度
	unsigned RW1 : 2;  // 设定Dr1指向地址的断点类型
	unsigned LEN1 : 2;  // 设定Dr1指向地址的断点长度
	unsigned RW2 : 2;  // 设定Dr2指向地址的断点类型
	unsigned LEN2 : 2;  // 设定Dr2指向地址的断点长度
	unsigned RW3 : 2;  // 设定Dr3指向地址的断点类型
	unsigned LEN3 : 2;  // 设定Dr3指向地址的断点长度
}DBG_REG7, *PDBG_REG7;

// 断点信息结构体
struct BreakPointInfo
{
	LPVOID Address; // 断点的地址
	DWORD ThisStyle;
	BYTE OpCode; // 原始的1字节
};

class BreakPoint
{
public:
	static BOOL barwflag;
	// CC断点信息列表
	static vector<BreakPointInfo> CCBreakPointList;
	// 内存访问断点List， 但是只能有一个
	static vector<BreakPointInfo> MemBreakPointList;

public:
	// 设置Int3断点
	static BOOL SetINT3BreakPoint(HANDLE Process, LPVOID Address);
	// 修复Int3断点
	static VOID FixINT3BreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// 设置硬件执行断点
	static BOOL SetHarDwareBreakPoint(HANDLE Thread, LPVOID Address);
	// 修复硬件执行断点
	static VOID FixHarDwareBreakPoint(HANDLE Thread);
	// 设置硬件读写断点
	static BOOL SetHarDwareRwBreakPoint(HANDLE hThread, ULONG_PTR uAddress, DWORD type, DWORD dwLen);
	// 修复硬件读写断点
	static VOID FixHarDwareRwBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag);
	// 设置TF断点（单步步入）断点
	static VOID SetTfBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag);
	// 设置单步步过断点
	static VOID SetStepOverBreakPoint(HANDLE Process, HANDLE Thread, DWORD Count);
	// 设置内存访问断点
	static VOID SetMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// 修复内存访问断点
	static VOID FixMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address);
	// API断点
	static BOOL APIBreakPiont(HANDLE NowProcess, CHAR * APIName, DWORD ImageBase, DWORD FileMSize);
	// 获取寄存器当前信息
	static VOID GetCurrentThreadInfo(HANDLE Thread);
};
