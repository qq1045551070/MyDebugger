#include "BreakPoint.h"
#include "BeaEngine.h"
#include "DbgSymbol.h"
#include <atlstr.h>

BOOL BreakPoint::barwflag = FALSE;

// ������һ������ϵ�
// Process Ŀ�����, Address �ϵ���ʼλ��
BOOL BreakPoint::SetINT3BreakPoint(HANDLE Process, LPVOID Address)
{
	// ����ϵ���Ϣ�ṹ��
	struct BreakPointInfo Info = {Address, 0};
	// Ϊ�˿��Զ�ȡ��д������, ��Ҫ���÷�ҳ����
	DWORD OldProtect = 0;
	VirtualProtectEx(Process, Info.Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	// ����ԭ����1�ֽ�����
	DWORD Size = 0;
	ReadProcessMemory(Process, Info.Address, &Info.OpCode, 1, &Size);

	// д��һ�� 0xCC �ϵ�
	DWORD Flag = WriteProcessMemory(Process, Info.Address, "\xCC", 1, &Size);

	// ��ԭ����ı�������
	VirtualProtectEx(Process, Info.Address, 1, OldProtect, NULL);

	//�洢���ϵ��б���
	CCBreakPointList.push_back(Info);

	if (!Flag) return FALSE;
	else return TRUE;
}

// �޸�CC�ϵ�
VOID BreakPoint::FixINT3BreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// ����CC�ϵ��б��ҵ���Ӧ�ϵ�Ľṹ��
	for (size_t i = 0; i < CCBreakPointList.size(); i++)
	{
		// �ҵ����õĶϵ�
		if (CCBreakPointList[i].Address == Address)
		{
			// Ϊ�˿��Զ�ȡ��д������
			DWORD OldProtect = 0;
			VirtualProtectEx(Process, Address, 1, PAGE_EXECUTE_READWRITE, &OldProtect);

			DWORD Size;
			WriteProcessMemory(Process, Address, &CCBreakPointList[i].OpCode, 1, &Size);

			// ��ԭ����ı�������
			VirtualProtectEx(Process, Address, 1, OldProtect, NULL);

			// ��Ϊ��[�����쳣], ����ʵ�ʶ��µĵ�ַ��int3֮��ĵ�ַ
			CONTEXT Context = {CONTEXT_CONTROL};
			// ��ȡEip
			GetThreadContext(Thread, &Context);

			Context.Eip -= 1;
			// �ı�Eip
			SetThreadContext(Thread, &Context);

			// ����CC�ϵ��������ȥ��
			vector<BreakPointInfo>::iterator it = CCBreakPointList.begin() + 1;
			CCBreakPointList.erase(it);
		}
	}
}

// ����Ӳ��ִ�жϵ�
BOOL BreakPoint::SetHarDwareBreakPoint(HANDLE Thread, LPVOID Address)
{
	/* 
	Ӳ���ϵ��ʵ��������CPU�ṩ�ĵ��ԼĴ���
	���б����ַ�ļĴ����� Dr0/3, �����4��
	Ӳ���ϵ��ǼĴ���������һ���֣�λ��ϵͳ�ϵ�
	��ʱ���߳�û�г�ʼ����ϣ����Բ�������Ӳ���ϵ�
	*/
	/*
	Ӳ��ִ�жϵ㣺��������쳣
	Ӳ����д�ϵ㣺��������쳣��Eipͣ������һ��ָ��
	*/

	CONTEXT Context = {CONTEXT_DEBUG_REGISTERS};
	GetThreadContext(Thread, &Context);

	// ��ȡ Dr7 �����ж�
	PDBG_REG7 Dr7 = (PDBG_REG7)&Context.Dr7;
	// �жϵ�һ��λ���Ƿ�Ϊ��
	if (Dr7->L0 == NULL)
	{
		// �����ϵ�
		Dr7->L0 = 1;
		// ���öϵ�λ��
		Context.Dr0 = (DWORD)Address;
		// ���öϵ�����ͣ�ִ�У�
		Dr7->RW0 = 0;
		// ���öϵ�ĳ��ȣ�ִ�жϵ����Ϊ0
		Dr7->LEN0 = 0;
	}
	// �ж�ʣ�µ�����λ��
	else if (Dr7->L1 == NULL)
	{
		// �����ϵ� // ���öϵ�λ��
		Dr7->L1 = 1; Context.Dr1 = (DWORD)Address;
		// ���öϵ�����ͣ�ִ�У�// ���öϵ�ĳ��ȣ�ִ�жϵ����Ϊ0
		Dr7->RW1 = 0; Dr7->LEN1 = 0;
	}
	else if (Dr7->L2 == NULL)
	{
		// �����ϵ� // ���öϵ�λ��
		Dr7->L2 = 1; Context.Dr2 = (DWORD)Address;
		// ���öϵ�����ͣ�ִ�У�// ���öϵ�ĳ��ȣ�ִ�жϵ����Ϊ0
		Dr7->RW2 = 0; Dr7->LEN2 = 0;
	}
	else if (Dr7->L3 == NULL)
	{
		// �����ϵ� // ���öϵ�λ��
		Dr7->L3 = 1; Context.Dr3 = (DWORD)Address;
		// ���öϵ�����ͣ�ִ�У�// ���öϵ�ĳ��ȣ�ִ�жϵ����Ϊ0
		Dr7->RW3 = 0; Dr7->LEN3 = 0;
	}
	else
	{
		// ���еļĴ�������������
		return FALSE;
	}

	// ���޸ĺ�ļĴ��������޸Ļ�ȥ
	SetThreadContext(Thread, &Context);

	return TRUE;
}

// �޸�Ӳ��ִ�жϵ�
VOID BreakPoint::FixHarDwareBreakPoint(HANDLE Thread)
{
	// ����ͨ��Dr6��־λ�ĵ�4λ֪�����ĸ��ϵ㱻����
	// ��ȡ���ԼĴ���
	CONTEXT Context = {CONTEXT_DEBUG_REGISTERS};
	GetThreadContext(Thread, &Context);

	// ��ȡ Dr7
	PDBG_REG7 Dr7 = (PDBG_REG7)&Context.Dr7;
	// ��ȡ Dr6 �ĵ�4λ
	int index = Context.Dr6 & 0xF;

	//����ָ����ֵ��0
	switch (index)
	{
	case 1: Dr7->L0 = 0; break;
	case 2: Dr7->L1 = 0; break;
	case 4: Dr7->L2 = 0; break;
	case 8: Dr7->L3 = 0; break;
	}

	//���޸ĺ�ļĴ������û�ȥ
	SetThreadContext(Thread, &Context);
	//�ж��Ƿ�Ϊ��дӲ���ϵ�, ����
	if (BreakPoint::barwflag == TRUE)
		BreakPoint::barwflag = FALSE;
}

// ����Ӳ����д�ϵ�(���ڴ�ϵ�Ч��һ������ԭ��һ��)
BOOL BreakPoint::SetHarDwareRwBreakPoint(HANDLE hThread, ULONG_PTR uAddress, DWORD type, DWORD dwLen)
{
	// ��ȡ�̻߳�����
	CONTEXT NowText = { 0 };
	NowText.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(hThread, &NowText);
	if (BreakPoint::barwflag == FALSE)
		BreakPoint::barwflag = TRUE;

	// �Ե�ַ�ͳ��Ƚ��ж��봦������ȡ����
	// ����Ӳ����ȡ�ϵ㣬������ó���Ϊ1������Ҫ����
	// �������Ϊ2�����ַ������2��������
	// �������Ϊ4�����ַ������4��������
	if (dwLen == 1)
	{
		// 2 �ֽڵĶ�������
		uAddress = uAddress - uAddress % 2;
	}
	else if (dwLen == 3)
	{
		// 4�ֽڵĶ�������
		uAddress = uAddress - uAddress % 4;
	}
	else if (dwLen > 3)
	{
		return FALSE;
	}

	// �ж���Щ�Ĵ���û�б�����
	DBG_REG7 * pDr7 = (DBG_REG7 *)&NowText.Dr7;
	if (pDr7->L0 == 0)
	{
		// DR0û�б�ʹ��
		NowText.Dr0 = uAddress;
		pDr7->RW0 = type;
		pDr7->LEN0 = dwLen;
	}
	// ��������
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
	// ��������
	else
		return FALSE;

	// ���޸ĺ�ļĴ������û�ȥ
	SetThreadContext(hThread, &NowText);
	return TRUE;
}

// �޸�Ӳ����д�ϵ�
VOID BreakPoint::FixHarDwareRwBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag)
{

}

//���õ�������TF�ϵ�
VOID BreakPoint::SetTfBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address, BOOL MFlag)
{
	// TF�ϵ�����ִ��һ��ָ�ԭ���ǽ�TF��־λ��Ϊ1
	// ����Ҫ�޸�TF��־λ�� CPU���Զ����TF

	CONTEXT Context = {CONTEXT_CONTROL | CONTEXT_ALL};
	GetThreadContext(Thread, &Context);

	// TF��־λ�ǵ� 8 λ�� ���ǵ�8λΪ1(�±�0��ʼ)
	Context.EFlags |= 0x100;

	// ���޸ĺ�Ļ������û�ȥ
	SetThreadContext(Thread, &Context);
}

//�����õ��������ϵ�
VOID BreakPoint::SetStepOverBreakPoint(HANDLE Process, HANDLE Thread, DWORD Count)
{
	CONTEXT Context = { CONTEXT_CONTROL };
	GetThreadContext(Thread, &Context);

	// ��ȡEIP��ֵ
	DWORD Addr = Context.Eip;

	PCHAR buff = new CHAR[Count * 16]{ 0 };
	DWORD dwWrite = 0;
	ReadProcessMemory(Process, (LPVOID)Addr, buff, Count * 16, &dwWrite);

	DISASM disAsm = { 0 };
	// ���ýṹ�壬��ʼ��������opcode
	disAsm.EIP = (UIntPtr)buff; // ����opcode�Ļ������׵�ַ
	disAsm.VirtualAddr = (DWORD)buff; // opcode ָ��ĵ�ַ
	disAsm.Archi = 0; // 0 => 32 , 1 => 64
	disAsm.Options = 0x000; // masm ���ָ���ʽ

	int nLen = 0; // ���ڼ�¼��ǰ�Ļ��ָ����ֽ���
	// ����Disasm�������з���࣬ 
	nLen = Disasm(&disAsm); // ÿ��ֻ�����һ�����ָ� ���ҷ��ص�ǰ�õ��Ļ��ָ��ĳ���

	//Ѱ�ҵ�ǰ���ָ�����Ƿ� ��call��
	if (((CString)disAsm.CompleteInstr).Find("call") != -1)
	{
		// �о�������һ��ָ���ַ�� int 3 �ϵ�
		DWORD Int3Addr = Context.Eip + nLen;
		// ������INT3�ϵ�
		SetINT3BreakPoint(Process, (LPVOID)Int3Addr);
		return;
	}
	
	Context.EFlags |= 0x100;
	SetThreadContext(Thread, &Context);
}

// �����ڴ���ʶϵ�
VOID BreakPoint::SetMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// ��ָ����ַ����Ϊ���ɷ���
	// �����ڴ�ϵ���Ϣ�ṹ��
	struct BreakPointInfo Info = { Address, 0 , 0 };
	VirtualProtectEx(Process, Address, 1, PAGE_NOACCESS, &Info.ThisStyle);
	MemBreakPointList.push_back(Info);
}

// �޸��ڴ���ʶϵ�
VOID BreakPoint::FixMemBreakPoint(HANDLE Process, HANDLE Thread, LPVOID Address)
{
	// �޸��ڴ��������
	VirtualProtectEx(Process, Address, 1, MemBreakPointList[0].ThisStyle, NULL);
	// ����TF�ϵ�
	SetTfBreakPoint(Process, Thread, Address, TRUE);
}

// API �ϵ�
BOOL BreakPoint::APIBreakPiont(HANDLE NowProcess, CHAR * APIName, DWORD ImageBase, DWORD FileMSize)
{
	BOOL Flag = TRUE;
	// ��ȡAPI�ĵ�ַ
	DWORD ApiAddr = DbgSymbol::FindApiAddress(NowProcess, APIName, ImageBase, FileMSize);
	if (!ApiAddr)return FALSE;
	// �� API ��ַ����һ��INT 3�ϵ�
	Flag = SetINT3BreakPoint(NowProcess, (LPVOID)ApiAddr);

	return Flag;
}

// ��ȡ��ǰ�̼߳Ĵ�����Ϣ
VOID BreakPoint::GetCurrentThreadInfo(HANDLE Thread)
{
	CONTEXT NowContext = { CONTEXT_ALL };
	GetThreadContext(Thread, &NowContext);
	printf("EFlags = %08X, Eip = %08X\n", NowContext.EFlags, NowContext.Eip);
	printf("Eax = %08X, Ebx = %08X, Edx = %08X\n", NowContext.Eax, NowContext.Ebx, NowContext.Edx);
}



