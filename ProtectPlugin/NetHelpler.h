#pragma once

#include<string>
#include<vector>

#ifndef _UNICODE
	#define Tstring std::string
#else
	#define Tstring std::wstring
#endif

class CNetHelpler
{
public:
	CNetHelpler();
	~CNetHelpler();
	BOOL StartNetService(Tstring strIpaddr, int nPort);
	BOOL StopNetService();

private:
	HANDLE m_completionPort;
	SOCKET m_listenSocket;
	HANDLE m_hMutex;

public:
	static DWORD WINAPI AcceptThread(LPVOID lpParameter);
	static DWORD WINAPI WorkerThread(LPVOID lpParameter);
};


/**
* �ṹ�����ƣ�PER_IO_DATA
* �ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
**/
const int DataBuffSize = 2 * 1024;
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[DataBuffSize];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
* �ṹ�����ƣ�PER_HANDLE_DATA
* �ṹ��洢����¼�����׽��ֵ����ݣ��������׽��ֵı������׽��ֵĶ�Ӧ�Ŀͻ��˵ĵ�ַ��
* �ṹ�����ã��������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У�֪���ͻ��˵ĵ�ַ�Ա��ڻطá�
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// ����ȫ�ֱ���  
const int DefaultPort = 6000;
std::vector < PER_HANDLE_DATA* > clientGroup;      // ��¼�ͻ��˵������� 
