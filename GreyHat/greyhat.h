#pragma once

#include <windows.h>
#include <windowsx.h>
#include "core.h"
#include "hexedit.h"
#include "Resource.h"
#include "Utility.h"
#include <Shlwapi.h>

#include "viewpage.h"
#include "sendpage.h"
#include "filterpage.h"
#include "ctrl.h"

#define PACKET_SEC  L"PACKET"

#define PM_HEXEDIT      0
#define PM_DATALIST     1

typedef unsigned char uint8_t;
typedef int  int32_t;
typedef unsigned int  uint32_t;
//DWORD Tstring2Hex( wchar_t *wszStr, BYTE *byteHex );

VOID init_tab_ctrl(  );

BOOL OnWMInitDlg( HWND hWnd, HWND hWndFocus, LPARAM lParam );
void Cls_OnHotKey(HWND hwnd, int idHotKey, UINT fuModifiers, UINT vk);


INT_PTR CALLBACK MainUIDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );


//Windows message handler
//WM_COMMAND
void Cls_OnCommand( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

//WM_SIZE
void OnWMSize( HWND hWnd, UINT state, int cx, int cy );

//WM_NOTIFY
BOOL OnWMNotify( HWND hwnd, INT id, LPNMHDR pnm );

//WM_CTRLCOLOR
HBRUSH OnCtrlColor( HWND hwnd, HDC hdc, HWND hwndChild, int type );

//VOID 
//WINAPI
//OutdataHandler( 
//			   SOCKET s, 
//			   CONST CHAR* pBuffer, 
//			   DWORD dwSize,
//			   LPARAM lParam0,
//			   LPARAM lParam1,
//			   LPARAM lParam2,
//			   LPARAM lParam3 );
//
//VOID WINAPI IndataHandler(CHAR* pBuffer, DWORD cbSize);



extern HWND g_hmain_dlg;
//#define id2handle(id)  GetDlgItem(g_hmain_dlg, id)

struct GLOBAL_ENV{

	HINSTANCE hUiInst;
	HINSTANCE hCoreInst;

	TCHAR tszUiDllPath[MAX_PATH];
	TCHAR tszUiDllName[MAX_PATH];


	TCHAR tszCfgFilePath[MAX_PATH];

	TCHAR tszCoreDllName[MAX_PATH];
	TCHAR tszCoreDllPath[MAX_PATH];

//	TCHAR tszGameName[128];
	TCHAR tszGameName[MAX_PATH];
	TCHAR tszGamePath[MAX_PATH];

	ATTACHPROCESS		pfnAttachProcess;
	DETACHPROCESS		pfnDetachProcess;
	DISPATCHGAMEDATA	pfnDispatchGameData;
	SETGLOBALENV			pfnSetGlobalEnvironment;

};

//struct GLOBAL_DATA
//{
//	list<CDataFilter*> DataFilterLink;
//	CRITICAL_SECTION   CsDataFilter;
//	CRITICAL_SECTION   CsPacket;
//};

extern GLOBAL_ENV GlobalEnv;