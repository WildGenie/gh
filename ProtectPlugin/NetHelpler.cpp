#include "stdafx.h"
#include "NetHelpler.h"

//IOCP��Ҫ������ͷ�ļ�
#include<WinSock2.h>
#include<Windows.h>

#pragma comment(lib, "Ws2_32.lib")      // Socket������õĶ�̬���ӿ�  
#pragma comment(lib, "Kernel32.lib")    // IOCP��Ҫ�õ��Ķ�̬���ӿ�  


DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);

CNetHelpler::CNetHelpler()
{

}


CNetHelpler::~CNetHelpler()
{
}


BOOL CNetHelpler::StartNetService(Tstring strIpaddr, int nPort)
{
	// ����socket��̬���ӿ�  
	WORD wVersionRequested = MAKEWORD(2, 2); // ����2.2�汾��WinSock��  
	WSADATA wsaData;    // ����Windows Socket�Ľṹ��Ϣ  
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err) {  // ����׽��ֿ��Ƿ�����ɹ�  
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {// ����Ƿ�����������汾���׽��ֿ�  
		WSACleanup();
		return FALSE;
	}

	//m_hMutex = CreateMutex(NULL, FALSE, NULL);
	// ����IOCP���ں˶���  
	/**
	* ��Ҫ�õ��ĺ�����ԭ�ͣ�
	* HANDLE WINAPI CreateIoCompletionPort(
	*    __in   HANDLE FileHandle,     // �Ѿ��򿪵��ļ�������߿վ����һ���ǿͻ��˵ľ��
	*    __in   HANDLE ExistingCompletionPort, // �Ѿ����ڵ�IOCP���
	*    __in   ULONG_PTR CompletionKey,   // ��ɼ���������ָ��I/O��ɰ���ָ���ļ�
	*    __in   DWORD NumberOfConcurrentThreads // ��������ͬʱִ������߳�����һ���ƽ���CPU������*2
	* );
	**/
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_completionPort) {    // ����IO�ں˶���ʧ��  
		return FALSE;
	}

	// ����IOCP�߳�--�߳����洴���̳߳�  

	// ȷ���������ĺ�������  
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	// ���ڴ������ĺ������������߳�  
	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�  
		HANDLE ThreadHandle = CreateThread(NULL, 0, WorkerThread, m_completionPort, 0, NULL);
		if (NULL == ThreadHandle) {

			return FALSE;
		}
		CloseHandle(ThreadHandle);
	}

	// ������ʽ�׽���  
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	// ��SOCKET������  
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(DefaultPort);
	int bindResult = bind(m_listenSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == bindResult) {
		return FALSE;
	}

	// ��SOCKET����Ϊ����ģʽ  
	int listenResult = listen(m_listenSocket, 10);
	if (SOCKET_ERROR == listenResult) {
		return FALSE;
	}

	// ��ʼ����IO����  
	//cout << "����������׼�����������ڵȴ��ͻ��˵Ľ���...\n";

	// �������ڷ������ݵ��߳�  
	HANDLE sendThread = CreateThread(NULL, 0, AcceptThread, this, 0, NULL);

	return 0;
}


BOOL CNetHelpler::StopNetService()
{
	return 0;
}

//�����߳�����
DWORD WINAPI CNetHelpler::AcceptThread(LPVOID lpParameter) {

	CNetHelpler* pServer = (CNetHelpler*)lpParameter;

	SOCKET srvSocket = (SOCKET)lpParameter;
	while (TRUE) {
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		// �������ӣ���������ɶˣ����������AcceptEx()  
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(pServer->m_listenSocket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {   // ���տͻ���ʧ��  
											  //cerr << "Accept Socket Error: " << GetLastError() << endl;
											  //system("pause");
			return FALSE;
		}

		// �����������׽��ֹ����ĵ����������Ϣ�ṹ  
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // �ڶ���Ϊ���PerHandleData����ָ����С���ڴ�  
		PerHandleData->socket = acceptSocket;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
		clientGroup.push_back(PerHandleData);       // �������ͻ�������ָ��ŵ��ͻ�������  

													// �������׽��ֺ���ɶ˿ڹ���  
		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), pServer->m_completionPort, (DWORD)PerHandleData, 0);


		// ��ʼ�ڽ����׽����ϴ���I/Oʹ���ص�I/O����  
		// ���½����׽�����Ͷ��һ�������첽  
		// WSARecv��WSASend������ЩI/O������ɺ󣬹������̻߳�ΪI/O�����ṩ����      
		// ��I/O��������(I/O�ص�)  
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read  

		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
}
//�����߳�����

DWORD WINAPI CNetHelpler::WorkerThread(LPVOID lpParam)
{
	HANDLE CompletionPort = (HANDLE)lpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = false;

	while (true) {
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		if (bRet == 0) {
			//cerr << "GetQueuedCompletionStatus Error: " << GetLastError() << endl;
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

		// ������׽������Ƿ��д�����  
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}

		// ��ʼ���ݴ����������Կͻ��˵�����  
		//WaitForSingleObject(pServer->m_hMutex, INFINITE);
		//cout << "A Client says: " << PerIoData->databuff.buf << endl;
		//ReleaseMutex(pServer->m_hMutex);

		// Ϊ��һ���ص����ý�����I/O��������  
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�  
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read  
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}