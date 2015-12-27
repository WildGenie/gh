#include "stdafx.h"
#include "Plugin.h"


ULONG CPlugin::m_offset12FromSend;
ULONG CPlugin::m_offset12FromWSASend;
ULONG CPlugin::m_offset12FromSend2;
ULONG CPlugin::m_offsetRecv;
ULONG CPlugin::m_offset12FromWSARecv;
ULONG CPlugin::m_offset12FromRecvFrom;
ULONG CPlugin::m_offset12FromWSAReceFrom;


CPlugin::CPlugin()
{
}


CPlugin::~CPlugin()
{
}

void CPlugin::SendData(CGPacket& packetBuf)
{
	CProperty pro;
	pro = packetBuf.GetPacketProperty();
	send(pro.s, (const char*)packetBuf.GetBuffer(), packetBuf.GetBufferLen(), 0);
}
BOOL CPlugin::InstallPlugin(SENDPROCHANDLER pfnHandleInputProc, RECVPROCHANDLER pfnHandleOutputProc)
{
	m_pfnHandleSendProc = pfnHandleInputProc;
	m_pfnHandleRecvProc = pfnHandleOutputProc;
	return PatchWinsockApis();
}
BOOL CPlugin::UnInstallPlugin()
{
	return UnPatchWinsockApis();
}



VOID APIENTRY CPlugin::WSASendStub(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount)
{
	for (unsigned int i = 0; i<dwBufferCount; i++)
	{
		if ( lpBuffers[i].len > 0) {
			CProperty pro;
			pro.s = s;
			CGPacket* packetBuf = new CGPacket((LPBYTE)lpBuffers[i].buf, lpBuffers[i].len, pro);
			packetBuf->GetPacketProperty().ioType = IO_OUTPUT;
//			m_pfnHandleSendProc(*packetBuf);
		}
	}
}

VOID __declspec(naked) CPlugin::WSASend12Thunk()
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 40h]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 40h]		//DATA( LPWSABUF )
		push    dword ptr ds : [esp + 40h]
		call    WSASendStub
		popfd
		popad
		jmp     m_offset12FromWSASend
	}
}
VOID APIENTRY CPlugin::SendStub(SOCKET s, const char* buf, int nlen)
{
	if (nlen > 0) {
		CProperty pro;
		pro.s = s;
		pro.ioType = IO_OUTPUT;
		CGPacket *pNewPacket = new CGPacket((LPBYTE)buf, nlen, pro);
		CPluginBase::m_PlugInstance->PreProcessGPacket(*pNewPacket);
		if (CPluginBase::m_PlugInstance->m_bSeePacket && FALSE == pNewPacket->IsFiltered())
		{
			m_pfnHandleSendProc(*pNewPacket);
		}
		else {
			delete pNewPacket;
		}

	}

}
VOID __declspec(naked) CPlugin::Send12Thunk(void)
{
	__asm
	{
		pushad
		pushfd
		push    dword ptr ds : [esp + 4ch]        //SIZE( dwBufferCount )
		push    dword ptr ds : [esp + 4ch]		//DATA( LPWSABUF )
		push    dword ptr ds : [esp + 4ch]  		//SOCKET
		call    SendStub
		popfd
		popad
		jmp     m_offset12FromSend
	}
}

//VOID APIENTRY CPlugin::SendToThunkHandler(SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
//{
//	CProperty pro;
//	pro.s = s;
//	CGPacket* packetBuf = new CGPacket((LPBYTE)buf, len, pro);
//	m_pfnHandleSendProc(*packetBuf);
//}
//VOID __declspec(naked) CPlugin::SendTo12Thunk(void)
//{
//	__asm
//	{
//		pushad
//		pushfd
//		push    dword ptr ds : [esp + 58h]        //tolen
//		push    dword ptr ds : [esp + 58h]        //to
//		push    dword ptr ds : [esp + 58h]        //flags
//		push    dword ptr ds : [esp + 58h]        //len
//		push    dword ptr ds : [esp + 58h]		//buf
//		push    dword ptr ds : [esp + 58h]  		//s
//		call    SendToThunkHandler
//		popfd
//		popad
//		jmp     m_offset12FromSend2
//	}
//}

int WSAAPI CPlugin::RecvStub(SOCKET s, char* buf, int bufsize, int flag)
{
	int retSize = ((int (WSAAPI *)(SOCKET, char*, int, int))m_offsetRecv)(s, buf, bufsize, flag);
	if (retSize != 0 && retSize != -1)
	{
		CProperty pro;
		pro.s = s;
		CGPacket* packetBuf = new CGPacket((LPBYTE)buf, retSize, pro);
		packetBuf->GetPacketProperty().ioType = IO_INPUT;
		m_pfnHandleRecvProc(*packetBuf);
	}
	return retSize;
}

BOOL CPlugin::PatchWinsockApis()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	m_offset12FromSend = (ULONG)(&send) + 12;
	DetourAttach(&(PVOID&)m_offset12FromSend, &Send12Thunk);

	m_offset12FromWSASend = (unsigned long)(GetProcAddress(GetModuleHandleA("ws2_32.dll"), "WSASend")) + 0x000000012;
	DetourAttach(&(PVOID&)m_offset12FromWSASend, &WSASend12Thunk);

	//m_offset12FromSend2 = (ULONG)(&sendto) + 12;
	//DetourAttach(&(PVOID&)m_offset12FromSend2, &SendTo12Thunk);

	m_offsetRecv = (ULONG)(&recv);
	DetourAttach(&(PVOID&)m_offsetRecv, &RecvStub);

	DetourTransactionCommit();

	return TRUE;
}


BOOL CPlugin::UnPatchWinsockApis()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)m_offset12FromSend, &Send12Thunk);
	DetourDetach(&(PVOID&)m_offset12FromWSASend, &WSASend12Thunk);
	//DetourDetach(&(PVOID&)m_offset12FromSend2, &SendTo12Thunk);
	DetourDetach(&(PVOID&)m_offsetRecv, &RecvStub);

	DetourTransactionCommit();
	return TRUE;
}
