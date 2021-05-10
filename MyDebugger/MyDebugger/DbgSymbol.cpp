#include "DbgSymbol.h"



DbgSymbol::DbgSymbol()
{
}


DbgSymbol::~DbgSymbol()
{
}

// �������ƻ�ȡ��ӦAPI�ĵ�ַ
SIZE_T DbgSymbol::FindApiAddress(HANDLE hProcess, const char * pszName, DWORD ImagesBase, DWORD FileMemSizes)
{
	// ��ȡĿ�����ڴ��е��ļ�
	DWORD TargetMemFileSize = FileMemSizes, size = 0;
	CHAR * TargetMemFile = new CHAR[FileMemSizes];
	ReadProcessMemory(hProcess, (LPVOID)ImagesBase, TargetMemFile, FileMemSizes, &size);

	IMAGE_DOS_HEADER * DosHeader = (IMAGE_DOS_HEADER *)TargetMemFile;
	IMAGE_NT_HEADERS * NtHeader = (IMAGE_NT_HEADERS *)(TargetMemFile + DosHeader->e_lfanew);

	// Ѱ�ҵ�һ����������ڴ��е�λ��
	DWORD importTabAddr = (DWORD)TargetMemFile + NtHeader->OptionalHeader.DataDirectory[1].VirtualAddress;
	IMAGE_IMPORT_DESCRIPTOR * importTab = (IMAGE_IMPORT_DESCRIPTOR *)importTabAddr;

	struct Judge
	{
		DWORD Number : 31;
		DWORD Flag : 1;
	};

	while (importTab->Name)
	{
		// �ҵ� IAT ���λ��
		DWORD iatTableAddr = importTab->FirstThunk + (DWORD)TargetMemFile;
		DWORD index = 0;

		// �Ҵ�INT��λ��
		IMAGE_THUNK_DATA32 * IntTable = (IMAGE_THUNK_DATA32 *)(importTab->OriginalFirstThunk + (DWORD)TargetMemFile);
		for (; IntTable->u1.Ordinal; IntTable++)
		{
			// �ж��Ƿ�����ŵ���
			if (((Judge *)IntTable)->Flag != 1)
			{
				IMAGE_IMPORT_BY_NAME * FristName = (IMAGE_IMPORT_BY_NAME *)(IntTable->u1.AddressOfData + (DWORD)TargetMemFile);
				if (!strcmp(FristName->Name, pszName))
				{
					// ����ҵ���Ӧ�ĺ����ĵ�ַ
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
