// capstoneTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "include\capstone.h"

#ifdef _64
#pragma comment(lib,"capstone_x64.lib")
#else
#pragma comment(lib,"capstone_x86.lib")
#endif // _64



int _tmain(int argc , _TCHAR* argv[])
{
	csh handle;
	cs_err err;
	cs_insn* pInsn;
	unsigned int count = 0;
	// 在编译capstone库的时候, 如果选择了用户自定义堆空间管理组(没有定义`CAPSTONE_USE_SYS_DYN_MEM`宏)
	// 则需要调用cs_option函数来设定堆空间管理组函数
	// 然后用结构体来配置回调函数.

	// 定义结构体, 配置堆空间的回调函数
	cs_opt_mem memop;
	memop.calloc = calloc;
	memop.free = free;
	memop.malloc = malloc;
	memop.realloc = realloc;
	memop.vsnprintf = (cs_vsnprintf_t)vsprintf_s;
	// 注册堆空间管理组函数
	cs_option(0 , CS_OPT_MEM , (size_t)&memop);


	//初始化反汇编器句柄,(x86_64架构,32位模式,句柄)
	err = cs_open(CS_ARCH_X86 , CS_MODE_32 , &handle);
	if(err != CS_ERR_OK)
	{
		printf("初始化反汇编器句柄失败\n");
		return 0;
	}

	unsigned char szOpcode[] = { "\x53\x83\xEC\x18\x83\x3D\x20\x20\x48\x00\x02\x8B\x44\x24\x24" };
	// 开始反汇编.
	count = cs_disasm(handle ,/*反汇编器句柄,从cs_open函数得到*/
					  szOpcode ,/*需要反汇编的opcode的缓冲区首地址*/
					  sizeof(szOpcode) , /*opcode的字节数*/
					  0x401000 , /*opcode的所在的内存地址*/
					  0 , /*需要反汇编的指令条数,如果是0,则反汇编出全部*/
					  &pInsn/*反汇编输出*/
					  );

	size_t j;
	for(j = 0; j < count; j++) {
		printf("0x%"PRIx64":%s %s\n" ,
			   pInsn[ j ].address , /*指令地址*/
			   pInsn[ j ].mnemonic ,/*指令操作码*/
			   pInsn[ j ].op_str/*指令操作数*/
			   );
	}

	cs_free(pInsn , count);
	return 0;
}

