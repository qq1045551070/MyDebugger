#include "BreakPoint.h"
#include "BeaEngine.h"
#include "DbgSymbol.h"
#include <atlstr.h>

BOOL BreakPoint::barwflag = FALSE;

// 设置以一个软件断点
// Process 目标进程, Address 断点起始位置
BOOL BreakPoint::SetINT3BreakPoint(HANDLE Process, LPVOID Address)
{
	// 保存断点信息结构体
	struct BreakPointInfo Info = {Address, 0};
	// 为了可以读取和写入数据, 需要设置分页属性
	DWORD OldProtect = 0;
	VirtualProtectEx(Process, Info.Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	// 保存原来的1字节数据
	DWORD Size = 0;
	ReadProcessMemory(Process, Info.Address, &Info.OpCode, 1, &Size);

	// 写入一个 0xCC 断点
	DWORD Flag = WriteProcessMemory(Process, Info.Address, "\xCC", 1, &Size);

	// 还原代码的保护属性
	VirtualProtectEx(Process, Info.Address, 1, OldProtect, NULL);

	//存储到断点列表中
	CCBreakPointList.push_back(Info);

	if (!Flag) return FALSE;
	else return TRUE;
}

// 修复CC断点
VOID BreakPoint::FixINT3BreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// 遍历CC断点列表，找到对应断点的结构体
	for (size_t i = 0; i < CCBreakPointList.size(); i++)
	{
		// 找到设置的断点
		if (CCBreakPointList[i].Address == Address)
		{
			// 为了可以读取和写入数据
			DWORD OldProtect = 0;
			VirtualProtectEx(Process, Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);

			DWORD Size;
			WriteProcessMemory(Process, Address, &CCBreakPointList[i].OpCode, 1, &Size);

			// 还原代码的保护属性
			VirtualProtectEx(Process, Address, 1, OldProtect, NULL);

			// 因为是[陷阱异常], 所以实际断下的地址是int3之后的地址
			CONTEXT Context = {CONTEXT_CONTROL};
			// 获取Eip
			GetThreadContext(Thread, &Context);

			Context.Eip -= 1;
			// 改变Eip
			SetThreadContext(Thread, &Context);

			// 将此CC断点从容器中去除
			vector<BreakPointInfo>::iterator it = CCBreakPointList.begin() + 1;
			CCBreakPointList.erase(it);
		}
	}
}

// 设置硬件执行断点
BOOL BreakPoint::SetHarDwareBreakPoint(HANDLE Thread, LPVOID Address)
{
	/* 
	硬件断点的实现依赖于CPU提供的调试寄存器
	其中保存地址的寄存器有 Dr0/3, 最多有4个
	硬件断点是寄存器环境的一部分，位于系统断点
	的时候，线程没有初始化完毕，所以不能设置硬件断点
	*/
	/*
	硬件执行断点：错误类的异常
	硬件读写断点：陷阱类的异常，Eip停留在下一条指令
	*/

	CONTEXT Context = {CONTEXT_DEBUG_REGISTERS};
	GetThreadContext(Thread, &Context);

	// 获取 Dr7 进行判断
	PDBG_REG7 Dr7 = (PDBG_REG7)&Context.Dr7;
	// 判断第一个位置是否为空
	if (Dr7->L0 == NULL)
	{
		// 启动断点
		Dr7->L0 = 1;
		// 设置断点位置
		Context.Dr0 = (DWORD)Address;
		// 设置断点的类型（执行）
		Dr7->RW0 = 0;
		// 设置断点的长度，执行断点必须为0
		Dr7->LEN0 = 0;
	}
	// 判断剩下的三个位置
	else if (Dr7->L1 == NULL)
	{
		// 启动断点 // 设置断点位置
		Dr7->L1 = 1; Context.Dr1 = (DWORD)Address;
		// 设置断点的类型（执行）// 设置断点的长度，执行断点必须为0
		Dr7->RW1 = 0; Dr7->LEN1 = 0;
	}
	else if (Dr7->L2 == NULL)
	{
		// 启动断点 // 设置断点位置
		Dr7->L2 = 1; Context.Dr2 = (DWORD)Address;
		// 设置断点的类型（执行）// 设置断点的长度，执行断点必须为0
		Dr7->RW2 = 0; Dr7->LEN2 = 0;
	}
	else if (Dr7->L3 == NULL)
	{
		// 启动断点 // 设置断点位置
		Dr7->L3 = 1; Context.Dr3 = (DWORD)Address;
		// 设置断点的类型（执行）// 设置断点的长度，执行断点必须为0
		Dr7->RW3 = 0; Dr7->LEN3 = 0;
	}
	else
	{
		// 所有的寄存器都被设置了
		return FALSE;
	}

	// 将修改后的寄存器环境修改回去
	SetThreadContext(Thread, &Context);

	return TRUE;
}

// 修复硬件执行断点
VOID BreakPoint::FixHarDwareBreakPoint(HANDLE Thread)
{
	// 可以通过Dr6标志位的低4位知道是哪个断点被触发
	// 获取调试寄存器
	CONTEXT Context = {CONTEXT_DEBUG_REGISTERS};
	GetThreadContext(Thread, &Context);

	// 获取 Dr7
	PDBG_REG7 Dr7 = (PDBG_REG7)&Context.Dr7;
	// 获取 Dr6 的低4位
	int index = Context.Dr6 & 0xF;

	//根据指定的值清0
	switch (index)
	{
	case 1: Dr7->L0 = 0; break;
	case 2: Dr7->L1 = 0; break;
	case 4: Dr7->L2 = 0; break;
	case 8: Dr7->L3 = 0; break;
	}

	//将修改后的寄存器设置回去
	SetThreadContext(Thread, &Context);
	//判断是否为读写硬件断点, 若是
	if (BreakPoint::barwflag == TRUE)
		BreakPoint::barwflag = FALSE;
}

// 设置硬件读写断点(和内存断点效果一样，但原理不一样)
BOOL BreakPoint::SetHarDwareRwBreakPoint(HANDLE hThread, ULONG_PTR uAddress, DWORD type, DWORD dwLen)
{
	// 获取线程环境块
	CONTEXT NowText = { 0 };
	NowText.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(hThread, &NowText);
	if (BreakPoint::barwflag == FALSE)
		BreakPoint::barwflag = TRUE;

	// 对地址和长度进行对齐处理（向上取整）
	// 对于硬件读取断点，如果设置长度为1，则不需要对齐
	// 如果长度为2，则地址必须是2的整数倍
	// 如果长度为4，则地址必须是4的整数倍
	if (dwLen == 1)
	{
		// 2 字节的对齐粒度
		uAddress = uAddress - uAddress % 2;
	}
	else if (dwLen == 3)
	{
		// 4字节的对齐粒度
		uAddress = uAddress - uAddress % 4;
	}
	else if (dwLen > 3)
	{
		return FALSE;
	}

	// 判断哪些寄存器没有被设置
	DBG_REG7 * pDr7 = (DBG_REG7 *)&NowText.Dr7;
	if (pDr7->L0 == 0)
	{
		// DR0没有被使用
		NowText.Dr0 = uAddress;
		pDr7->RW0 = type;
		pDr7->LEN0 = dwLen;
	}
	// 其他三个
	else if (pDr7->L1 == 0)
	{
		NowText.Dr1 = uAddress;
		pDr7->RW1 = type;
		pDr7->LEN1 = dwLen;
	}
	else if (pDr7->L2 == 0)
	{
		NowText.Dr2 = uAddress;
		pDr7->RW2 = type;
		pDr7->LEN2 = dwLen;
	}
	else if (pDr7->L3 == 0)
	{
		NowText.Dr3 = uAddress;
		pDr7->RW3 = type;
		pDr7->LEN3 = dwLen;
	}
	// 都被设置
	else
		return FALSE;

	// 将修改后的寄存器设置回去
	SetThreadContext(hThread, &NowText);
	return TRUE;
}

// 修复硬件读写断点
VOID BreakPoint::FixHarDwareRwBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag)
{

}

//设置单步步入TF断点
VOID BreakPoint::SetTfBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag)
{
	// TF断点用于执行一条指令，原理是将TF标志位置为1
	// 不需要修复TF标志位， CPU会自动清除TF

	CONTEXT Context = {CONTEXT_CONTROL | CONTEXT_ALL};
	GetThreadContext(Thread, &Context);

	// TF标志位是第 8 位， 这是第8位为1(下标0开始)
	Context.EFlags |= 0x100;

	// 将修改后的环境设置回去
	SetThreadContext(Thread, &Context);
}

//设设置单步步过断点
VOID BreakPoint::SetStepOverBreakPoint(HANDLE Process, HANDLE Thread, DWORD Count)
{
	CONTEXT Context = { CONTEXT_CONTROL };
	GetThreadContext(Thread, &Context);

	// 获取EIP的值
	DWORD Addr = Context.Eip;

	PCHAR buff = new CHAR[Count * 16]{ 0 };
	DWORD dwWrite = 0;
	ReadProcessMemory(Process, (LPVOID)Addr, buff, Count * 16, &dwWrite);

	DISASM disAsm = { 0 };
	// 配置结构体，初始化反汇编的opcode
	disAsm.EIP = (UIntPtr)buff; // 保存opcode的缓冲区首地址
	disAsm.VirtualAddr = (DWORD)buff; // opcode 指令的地址
	disAsm.Archi = 0; // 0 => 32 , 1 => 64
	disAsm.Options = 0x000; // masm 汇编指令格式

	int nLen = 0; // 用于记录当前的汇编指令的字节数
	// 调用Disasm（）进行反汇编， 
	nLen = Disasm(&disAsm); // 每次只反汇编一条汇编指令， 并且返回当前得到的汇编指令的长度

	//寻找当前汇编指令中是否含 “call”
	if (((CString)disAsm.CompleteInstr).Find("call") != -1)
	{
		// 有就在其下一个指令地址下 int 3 断点
		DWORD Int3Addr = Context.Eip + nLen;
		// 对齐下INT3断点
		SetINT3BreakPoint(Process, (LPVOID)Int3Addr);
		return;
	}
	
	Context.EFlags |= 0x100;
	SetThreadContext(Thread, &Context);
}

// 设置内存访问断点
VOID BreakPoint::SetMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// 将指定地址设置为不可访问
	// 保存内存断点信息结构体
	struct BreakPointInfo Info = { Address, 0 , 0 };
	VirtualProtectEx(Process, Address, 1, PAGE_NOACCESS, &Info.ThisStyle);
	MemBreakPointList.push_back(Info);
}

// 修复内存访问断点
VOID BreakPoint::FixMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// 修复内存访问属性
	VirtualProtectEx(Process, Address, 1, MemBreakPointList[0].ThisStyle, NULL);
	// 设置TF断点
	SetTfBreakPoint(Process, Thread, Address, TRUE);
}

// API 断点
BOOL BreakPoint::APIBreakPiont(HANDLE NowProcess, CHAR * APIName, DWORD ImageBase, DWORD FileMSize)
{
	BOOL Flag = TRUE;
	// 获取API的地址
	DWORD ApiAddr = DbgSymbol::FindApiAddress(NowProcess, APIName, ImageBase, FileMSize);
	if (!ApiAddr)return FALSE;
	// 在 API 地址处下一个INT 3断点
	Flag = SetINT3BreakPoint(NowProcess, (LPVOID)ApiAddr);

	return Flag;
}

// 获取当前线程寄存器信息
VOID BreakPoint::GetCurrentThreadInfo(HANDLE Thread)
{
	CONTEXT NowContext = { CONTEXT_ALL };
	GetThreadContext(Thread, &NowContext);
	printf("EFlags = %08X, Eip = %08X\n", NowContext.EFlags, NowContext.Eip);
	printf("Eax = %08X, Ebx = %08X, Edx = %08X\n", NowContext.Eax, NowContext.Ebx, NowContext.Edx);
}



