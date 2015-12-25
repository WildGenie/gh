// common.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include "Plugin.h"
#include "DOMPlugin.h"
#include "BladePlugin.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

#define CONFIG_FILE _T("config.ini")
#define GAME_SECTION _T("GAME")
#define GAME_VALUE _T("NAME")


CPluginBase* pPlugin;
HMODULE g_hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	g_hInstance = hModule;
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


BOOL APIENTRY Install(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	Tstring strDllPath = Utility::Module::GetModulePath(g_hInstance);

	Tstring strCfgFile = strDllPath + Tstring(_T("\\")) + Tstring(CONFIG_FILE);

	Tstring strGameName = Utility::IniAccess::GetPrivateKeyValString(strCfgFile, GAME_SECTION, GAME_VALUE);
	Utility::Log::DbgPrint(_T("game=[%s]"), strGameName.c_str());
	if (strGameName == Tstring(_T("BSTW"))) {
		pPlugin = new CBladePlugin;
		Utility::Log::DbgPrint(_T("new CBladePlugin"));

	}
	else {
		pPlugin = new CPlugin;
		Utility::Log::DbgPrint(_T("new CPlugin"));
	}
	
	if (pPlugin)
		return pPlugin->InstallPlugin(pfnHandleInputProc, pfnHandleOutputProc);
	else
		return FALSE;
}

BOOL APIENTRY UnInstall()
{
	if (pPlugin)
	{
		BOOL ret = pPlugin->UnInstallPlugin();
		delete pPlugin;
		return ret;
	}
	else
		return FALSE;
}

VOID APIENTRY Send(CGPacket& packetBuf)
{
	if (pPlugin)
	{
		pPlugin->SendData(packetBuf);
	}
		
}

void APIENTRY AddFilter(CGPacketFilter& filter)
{
	if (pPlugin)
	{
		pPlugin->AddPacketFilter(filter);
	}
}
void APIENTRY DeleteFilter(CGPacketFilter& filter)
{
	if (pPlugin)
	{
		pPlugin->DeletePacketFilter(filter);
	}
}
void APIENTRY ClearPacketFilters()
{
	if (pPlugin)
	{
		pPlugin->ClearPacketFilters();
	}
}
void APIENTRY EnableFilter(BOOL bEnable)
{
	if (pPlugin)
	{
		pPlugin->SetFilterEnable(bEnable);
	}
}
void APIENTRY EnableReplace(BOOL bEnable)
{
	if (pPlugin)
	{
		pPlugin->SetReplaceEnable(bEnable);
	}
}