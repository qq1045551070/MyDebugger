// capstoneTest.cpp : �������̨Ӧ�ó������ڵ㡣
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
	// �ڱ���capstone���ʱ��, ���ѡ�����û��Զ���ѿռ������(û�ж���`CAPSTONE_USE_SYS_DYN_MEM`��)
	// ����Ҫ����cs_option�������趨�ѿռ�����麯��
	// Ȼ���ýṹ�������ûص�����.

	// ����ṹ��, ���öѿռ�Ļص�����
	cs_opt_mem memop;
	memop.calloc = calloc;
	memop.free = free;
	memop.malloc = malloc;
	memop.realloc = realloc;
	memop.vsnprintf = (cs_vsnprintf_t)vsprintf_s;
	// ע��ѿռ�����麯��
	cs_option(0 , CS_OPT_MEM , (size_t)&memop);


	//��ʼ������������,(x86_64�ܹ�,32λģʽ,���)
	err = cs_open(CS_ARCH_X86 , CS_MODE_32 , &handle);
	if(err != CS_ERR_OK)
	{
		printf("��ʼ������������ʧ��\n");
		return 0;
	}

	unsigned char szOpcode[] = { "\x53\x83\xEC\x18\x83\x3D\x20\x20\x48\x00\x02\x8B\x44\x24\x24" };
	// ��ʼ�����.
	count = cs_disasm(handle ,/*����������,��cs_open�����õ�*/
					  szOpcode ,/*��Ҫ������opcode�Ļ������׵�ַ*/
					  sizeof(szOpcode) , /*opcode���ֽ���*/
					  0x401000 , /*opcode�����ڵ��ڴ��ַ*/
					  0 , /*��Ҫ������ָ������,�����0,�򷴻���ȫ��*/
					  &pInsn/*��������*/
					  );

	size_t j;
	for(j = 0; j < count; j++) {
		printf("0x%"PRIx64":%s %s\n" ,
			   pInsn[ j ].address , /*ָ���ַ*/
			   pInsn[ j ].mnemonic ,/*ָ�������*/
			   pInsn[ j ].op_str/*ָ�������*/
			   );
	}

	cs_free(pInsn , count);
	return 0;
}

