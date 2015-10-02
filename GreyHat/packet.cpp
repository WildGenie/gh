#include "stdafx.h"
#include "Packet.h"

CPacket::CPacket(LPBYTE lpBuff, DWORD dwLen, DISPATCH_CONTEXT *ctx)
{
	m_pbRawData = NULL;
	m_pbBackendData = NULL;
	m_dwPacketLen = 0;
	m_DispatchContext = { 0 };
	m_bIsSkip = FALSE;
	m_tstrRemoteIp = _T("N/A");
	m_dwRemotePort = 0;
	m_Type = IO_SEND;

	if (dwLen > 0 && lpBuff != NULL) {
		m_pbRawData = lpBuff;
		m_pbBackendData = new byte[dwLen];
		memcpy(m_pbBackendData, lpBuff, dwLen);
		m_dwPacketLen = dwLen;
	}

	if (ctx != NULL)
	{
		m_DispatchContext = *ctx;
		sockaddr_in sin;
		int sinlen = sizeof( sin );
		if ( 0 == getpeername( ctx->sock,  ( sockaddr* ) &sin,   &sinlen ) )
		{
			TCHAR tszTemp[256], tszIP[64];
			DWORD dwLen = sizeof(tszTemp), dwPort;
			
			if (0 == WSAAddressToString((SOCKADDR *)&sin,sizeof(SOCKADDR),NULL,tszTemp,(LPDWORD)&dwLen))
			{
				_stscanf_s(tszTemp, _T("%[0-9,.]:%d"), tszIP, sizeof(tszIP), &dwPort);
				m_tstrRemoteIp = tszIP;
				m_dwRemotePort = dwPort;
			}
		}
	}
	
}

Tstring& CPacket::GetRemoteIp()
{
	return m_tstrRemoteIp;
}
DWORD   CPacket::GetRemotePort()
{
	return m_dwRemotePort;
}

CPacket::CPacket()
{
	m_pbRawData = NULL;
	m_pbBackendData = NULL;
	m_dwPacketLen = 0;
	m_DispatchContext = {0};
	m_bIsSkip = FALSE;
	m_tstrRemoteIp = _T("N/A");
	m_dwRemotePort = 0;
	m_Type = IO_SEND;
}
CPacket::~CPacket()
{
	delete [] m_pbBackendData;
}

LPBYTE CPacket::GetBackendData()
{
	return m_pbBackendData;
}

LPBYTE CPacket::GetRawData()
{
	return m_pbRawData;
}

DWORD CPacket::GetDataLen()
{
	return m_dwPacketLen;
}

DISPATCH_CONTEXT* CPacket::GetDispatchContext()
{
	return &m_DispatchContext;
}

VOID CPacket::SetSkipFlag(BOOL val)
{
	m_bIsSkip = val;
}
BOOL CPacket::IsSkipped()
{
	return m_bIsSkip;
}

VOID CPacket::SetType(PACKET_TYPE type)
{
	m_Type = type;
}
PACKET_TYPE CPacket::GetType()
{
	return m_Type;
}
///////////////////////////////////////////////////////////////////////////////////

CFilter::CFilter()
{
	m_tstrKeyWords		= _T("");
	m_tstrReplaceData	= _T("");
	m_tstrAdvFilter		= _T("");
	m_pAdvFilterBuffer = NULL;

	memset(m_AdvFilterSplit, 0, sizeof(m_AdvFilterSplit));
}

CFilter::~CFilter()
{
	delete[]m_pAdvFilterBuffer;
}


Tstring& CFilter::GetKeyWord()
{
	return m_tstrKeyWords;
}

VOID CFilter::SetKeyWord(Tstring& tstrKeyWords)
{
	m_tstrKeyWords = tstrKeyWords;
}

Tstring& CFilter::GetReplaceData()
{
	return m_tstrReplaceData;
}

VOID CFilter::SetReplaceData(Tstring& tstrReplaceData)
{
	m_tstrReplaceData = tstrReplaceData;
}

Tstring& CFilter::GetAdvFilterStr()
{
	return m_tstrAdvFilter;
}

VOID CFilter::SetAdvFilterStr(Tstring& tstrAdvFilterStr)
{
	DWORD dwStrLen;
	dwStrLen = tstrAdvFilterStr.length() + 1;

	m_pAdvFilterBuffer = new TCHAR[dwStrLen];
	memset(m_pAdvFilterBuffer, 0, sizeof(TCHAR)*dwStrLen);

	_tcscpy(m_pAdvFilterBuffer, tstrAdvFilterStr.c_str());

	TCHAR* pTemp = m_pAdvFilterBuffer;

	DWORD i = 0;

	if (*pTemp != _T('|'))
	{
		m_AdvFilterSplit[i++] = pTemp;
	}
	
	while (*pTemp != _T('\0'))
	{
		if ( *pTemp == _T('|') )//&& _T('\0') != *(pTemp + 1) 
		{
			*pTemp = _T('\0');
			if ( _T('\0') != *(pTemp + 1) )
			{
				m_AdvFilterSplit[i++] = pTemp + 1;
			}
			
		}
		pTemp++;
	}

	m_tstrAdvFilter = tstrAdvFilterStr;
}


BOOL CFilter::IsPacketFiltered(CPacket* pPacket)
{
	//1, ��ͨ����
	DWORD dwKeyWordSize, dwHexBuffSize, dwPacketSize,dwPacketPort, dwReplaceDataSize;
	LPBYTE lpKeyWordBuffer, lpPacketData, lpMatch, lpReplaceData;
	LPCTSTR lpstrPacketIp;

	lpPacketData = pPacket->GetRawData();
	dwPacketSize = pPacket->GetDataLen();
	dwPacketPort = pPacket->GetRemotePort();
	lpstrPacketIp = pPacket->GetRemoteIp().c_str();


	if (m_tstrKeyWords != _T("") && m_tstrReplaceData == _T(""))
	{
		lpKeyWordBuffer = new BYTE[m_tstrKeyWords.length()];

		dwKeyWordSize = Utility::StringLib::Tstring2Hex(m_tstrKeyWords.c_str(), lpKeyWordBuffer);


		lpMatch = SearchKeyWord(lpPacketData, dwPacketSize, lpKeyWordBuffer, dwKeyWordSize);
		delete[]lpKeyWordBuffer;

		if ( lpMatch ){
			return TRUE;
		}
	}


	//2,�߼�����  L=20|01=0C|02=0B|20=CC|IP=211.1.1.1|P=9900 �������붼Ҫ����
	TCHAR tszIp[128];
	TCHAR* pTemp; 
	DWORD dwLen, dwPort, dwIndex, dwValue;

	if ( m_AdvFilterSplit[0])
	{
		for (int i = 0; i < 256; i++ )
		{
			pTemp = m_AdvFilterSplit[i];
			if ( NULL == pTemp ) {
				return TRUE;
			}

			if ( *pTemp == _T('L'))//����
			{
				_stscanf_s(pTemp, _T("L=%d"), &dwLen);
				if (dwLen != dwPacketSize)
				{
					return FALSE;
				}
			}
			else if( *pTemp == _T('P'))//�˿�
			{
				_stscanf_s(pTemp, _T("P=%d"), &dwPort);
				if (dwPort != dwPacketPort)
				{
					return FALSE;
				}
			}
			else if ( *pTemp == _T('I') && *(pTemp + 1) == _T('P'))//IP
			{
				_stscanf_s(pTemp, _T("IP=%s"), tszIp);
				if(_tcscmp(tszIp, lpstrPacketIp))
				{
					return FALSE;
				}
			}else
			{
				_stscanf_s(pTemp, _T("%d=%x"), &dwIndex, &dwValue);
				if ( lpPacketData[dwIndex-1] != dwValue )
				{
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}

BOOL CFilter::ReplacePacketData(CPacket* pPacket)
{
	DWORD dwKeyWordSize, dwHexBuffSize, dwPacketSize, dwReplaceDataSize;
	LPBYTE lpKeyWordData, lpPacketData, lpMatch, lpReplaceData;

	if (m_tstrKeyWords != _T("") && m_tstrReplaceData != _T(""))
	{
		lpKeyWordData = new BYTE[m_tstrKeyWords.length()];
		lpReplaceData = new BYTE[m_tstrReplaceData.length()];

		dwKeyWordSize		= Utility::StringLib::Tstring2Hex(m_tstrKeyWords.c_str(), lpKeyWordData);
		dwReplaceDataSize	= Utility::StringLib::Tstring2Hex(m_tstrReplaceData.c_str(), lpReplaceData);

		lpPacketData = pPacket->GetRawData();
		dwPacketSize = pPacket->GetDataLen();

		lpMatch = SearchKeyWord(lpPacketData, dwPacketSize, lpKeyWordData, dwKeyWordSize);
		if (lpMatch) {
			memcpy( lpMatch, lpReplaceData, dwKeyWordSize );
		}
		delete[]lpKeyWordData;
		delete[]lpReplaceData;
	}
	
	return TRUE;
}

LPBYTE CFilter::SearchKeyWord(LPBYTE lpSrc, DWORD dwLen, LPBYTE lpKey, DWORD dwKeyLen)
{
	return (LPBYTE)Utility::Memory::BooyerSearch(lpSrc, dwLen, lpKey, dwKeyLen);
}