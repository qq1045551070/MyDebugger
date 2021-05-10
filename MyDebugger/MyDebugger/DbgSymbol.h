#pragma once
#include <windows.h>

class DbgSymbol
{
public:
	DbgSymbol();
	~DbgSymbol();

	// 根据名称获取对应API的地址
	static SIZE_T FindApiAddress(HANDLE hProcess, const char * pszName, DWORD ImagesBase, DWORD FileMemSizes);
};

