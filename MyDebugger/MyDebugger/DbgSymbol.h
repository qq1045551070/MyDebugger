#pragma once
#include <windows.h>

class DbgSymbol
{
public:
	DbgSymbol();
	~DbgSymbol();

	// �������ƻ�ȡ��ӦAPI�ĵ�ַ
	static SIZE_T FindApiAddress(HANDLE hProcess, const char * pszName, DWORD ImagesBase, DWORD FileMemSizes);
};

