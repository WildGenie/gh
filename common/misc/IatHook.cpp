#include "stdafx.h"
#include "IatHook.h"


CIatHook::CIatHook()
{
}


CIatHook::~CIatHook()
{
}

/************************************************************************/
/* ��������: Init                                                       */
/* ��������: PiaoYun/P.Y.G                                              */
/* ��������: pDllName:Ŀ��API���ڵ�DLL����                              */
/*           pFunName:Ŀ��API����                                       */
/*           dwNewProc:�Զ���ĺ�����ַ                                 */
/* ��������: PiaoYun/P.Y.G                                              */
/* ��������: PiaoYun/P.Y.G                                              */
/************************************************************************/
BOOL CIatHook::Hook(LPCSTR TargetModule, LPCSTR FunModule, LPCSTR lpcFunName, ULONG ulNewProc)
{
	PIAT_INFO pIATInfo = &iat;

	// �������Ƿ�Ϸ� 
	if (!TargetModule || !lpcFunName || !ulNewProc || !pIATInfo)
		return FALSE;

	// ���Ŀ��ģ���Ƿ���� 
	char szTempDllName[256] = { 0 };
	DWORD dwBaseImage = (DWORD)GetModuleHandleA(TargetModule);
	if (dwBaseImage == 0)
		return FALSE;

	// ȡ��PE�ļ�ͷ��Ϣָ�� 
	PIMAGE_DOS_HEADER   pDosHeader = (PIMAGE_DOS_HEADER)dwBaseImage;
	PIMAGE_NT_HEADERS   pNtHeader = (PIMAGE_NT_HEADERS)(dwBaseImage + (pDosHeader->e_lfanew));
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = &(pNtHeader->OptionalHeader);
	PIMAGE_SECTION_HEADER  pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 0x18 + pNtHeader->FileHeader.SizeOfOptionalHeader);

	// ��������� 
	PIMAGE_THUNK_DATA pThunk, pIAT;
	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)(dwBaseImage + pOptionalHeader->DataDirectory[1].VirtualAddress);
	while (pIID->FirstThunk)
	{
		// ����Ƿ�Ŀ��ģ�� -- ���ִ�Сд 
		if (_stricmp((PCHAR)(dwBaseImage + pIID->Name), FunModule))
		{
			pIID++;
			continue;
		}

		pIAT = (PIMAGE_THUNK_DATA)(dwBaseImage + pIID->FirstThunk);
		if (pIID->OriginalFirstThunk)
			pThunk = (PIMAGE_THUNK_DATA)(dwBaseImage + pIID->OriginalFirstThunk);
		else
			pThunk = pIAT;

		// ����IAT�� 
		DWORD dwThunkValue = 0;
		while ((dwThunkValue = *(PDWORD)pThunk) != 0)
		{
			if ((dwThunkValue & IMAGE_ORDINAL_FLAG32) == 0)
			{
				// ����Ƿ�Ŀ�꺯�� -- �ִ�Сд
				if (strcmp((PCHAR)(dwBaseImage + dwThunkValue + 2), lpcFunName) == 0)
				{
					// ��亯���ض�λ��Ϣ 
					pIATInfo->dwAddr = (DWORD)pIAT;
					pIATInfo->dwOldValue = *(PDWORD)pIAT;
					pIATInfo->dwNewValue = ulNewProc;

					DWORD dwOldProtect = 0;
					VirtualProtect((LPVOID)pIAT, 4, PAGE_READWRITE, &dwOldProtect);
					*(PDWORD)pIATInfo->dwAddr = (DWORD)ulNewProc;
					VirtualProtect((LPVOID)pIAT, 4, dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}

			pThunk++;
			pIAT++;
		}

		pIID++;
	}
	return FALSE;
}

VOID CIatHook::UnHook()
{
	if (iat.dwAddr)
	{
		DWORD dwOldProtect = 0;
		VirtualProtect((LPVOID)iat.dwAddr, 4, PAGE_READWRITE, &dwOldProtect);
		*(PDWORD)iat.dwAddr = iat.dwOldValue;
		VirtualProtect((LPVOID)iat.dwAddr, 4, dwOldProtect, &dwOldProtect);
	}
}

DWORD CIatHook::GetOldFunAddr()
{
	return iat.dwOldValue;
}