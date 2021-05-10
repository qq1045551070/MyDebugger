#include "DbgSymbol.h"



DbgSymbol::DbgSymbol()
{
}


DbgSymbol::~DbgSymbol()
{
}

// 根据名称获取对应API的地址
SIZE_T DbgSymbol::FindApiAddress(HANDLE hProcess, const char * pszName, DWORD ImagesBase, DWORD FileMemSizes)
{
	// 读取目标在内存中的文件
	DWORD TargetMemFileSize = FileMemSizes, size = 0;
	CHAR * TargetMemFile = new CHAR[FileMemSizes];
	ReadProcessMemory(hProcess, (LPVOID)ImagesBase, TargetMemFile, FileMemSizes, &size);

	IMAGE_DOS_HEADER * DosHeader = (IMAGE_DOS_HEADER *)TargetMemFile;
	IMAGE_NT_HEADERS * NtHeader = (IMAGE_NT_HEADERS *)(TargetMemFile + DosHeader->e_lfanew);

	// 寻找第一个导入表在内存中的位置
	DWORD importTabAddr = (DWORD)TargetMemFile + NtHeader->OptionalHeader.DataDirectory[1].VirtualAddress;
	IMAGE_IMPORT_DESCRIPTOR * importTab = (IMAGE_IMPORT_DESCRIPTOR *)importTabAddr;

	struct Judge
	{
		DWORD Number : 31;
		DWORD Flag : 1;
	};

	while (importTab->Name)
	{
		// 找到 IAT 表的位置
		DWORD iatTableAddr = importTab->FirstThunk + (DWORD)TargetMemFile;
		DWORD index = 0;

		// 找打INT表位置
		IMAGE_THUNK_DATA32 * IntTable = (IMAGE_THUNK_DATA32 *)(importTab->OriginalFirstThunk + (DWORD)TargetMemFile);
		for (; IntTable->u1.Ordinal; IntTable++)
		{
			// 判断是否是序号导出
			if (((Judge *)IntTable)->Flag != 1)
			{
				IMAGE_IMPORT_BY_NAME * FristName = (IMAGE_IMPORT_BY_NAME *)(IntTable->u1.AddressOfData + (DWORD)TargetMemFile);
				if (!strcmp(FristName->Name, pszName))
				{
					// 如果找到对应的函数的地址
					DWORD TargetFunAddr = ((PDWORD)iatTableAddr)[index];
					return TargetFunAddr;
				}
			}
			index++;
		}
		importTab++;
	}

	return NULL;
}
