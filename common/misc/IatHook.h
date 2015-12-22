#pragma once
#include <windows.h>

typedef struct _IAT_INFO {
	DWORD dwAddr;      // IAT���ڵ�ַ 
	DWORD dwOldValue;  // IATԭʼ������ַ 
	DWORD dwNewValue;  // IAT�º�����ַ 
} IAT_INFO, *PIAT_INFO;

class CIatHook
{
public:
	CIatHook();
	~CIatHook();
public:
	BOOL Hook(LPCSTR TargetModule, LPCSTR FunModule, LPCSTR lpcFunName, ULONG ulNewProc);
	VOID UnHook();
	DWORD GetOldFunAddr();  // ��ȡԭʼ������ַ
private:
	IAT_INFO iat;     // ���ڱ���IAT����Ϣ
};

