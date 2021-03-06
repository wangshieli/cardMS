// SimServer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "singleton_data.h"
#include "post_request.h"
#include "db_operation.h"

#ifdef _DEBUG
#define LISTEN_PORT		6088
#else
#define LISTEN_PORT		6086
#endif // DEBUG

// 保证服务器运行唯一实例↓↓↓↓↓↓↓↓
#define NP_THE_ONE_INSTANCE _T("Global\\np_The_One_Instance_Event_SimServer")
HANDLE hUniqueInstance = NULL;
bool UniqueInstance()
{
#ifdef _DEBUG
	return true;
#endif // _DEBUG

	hUniqueInstance = ::OpenEvent(EVENT_ALL_ACCESS, false, NP_THE_ONE_INSTANCE);
	if (NULL != hUniqueInstance)
		return false;

	hUniqueInstance = ::CreateEvent(NULL, false, false, NP_THE_ONE_INSTANCE);
	if (NULL == hUniqueInstance)
		return false;

	return true;
}
// 保证服务器运行唯一实例↑↑↑↑↑↑↑↑

// 初始化socket环境↓↓↓↓↓↓↓↓
bool InitSocketEnv()
{
	int rt = 0;
	WSADATA wsadata;

	rt = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (0 != rt)
		return false;

	InitDBPool();
	InitConnection(10, 50);

	SOCKET sTemp = INVALID_SOCKET;
	sTemp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sTemp)
	{
		WSACleanup();
		return false;
	}

	DWORD dwBytes = 0;

	rt = WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs),
		&lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs),
		&dwBytes, NULL, NULL);
	if (SOCKET_ERROR == rt)
		goto error;

	rt = WSAIoctl(sTemp, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&lpfnAccpetEx, sizeof(lpfnAccpetEx),
		&dwBytes, NULL, NULL);
	if (SOCKET_ERROR == rt)
		goto error;

	closesocket(sTemp);
	sTemp = INVALID_SOCKET;

	return true;

error:
	closesocket(sTemp);
	WSACleanup();
	return false;
}
// 初始化socket环境↑↑↑↑↑↑↑↑

unsigned int _stdcall completionRoution(LPVOID);

int main01()
{
	if (!UniqueInstance())
	{
		_tprintf(_T("已经有服务器处于工作状态，不要重复启动\n"));
		Sleep(1000 * 20);
		return -1;
	}

	if (!InitSocketEnv())
	{
		_tprintf(_T("初始化socket环境失败, errCode = %d\n"), GetLastError());
		return -1;
	}

	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 0);
	if (NULL == hCompPort)
	{
		_tprintf(_T("创建完成端口失败, errCode = %d\n"), WSAGetLastError());
		WSACleanup();
		return -1;
	}

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	DWORD dwNumberOfCPU = sys.dwNumberOfProcessors;
	g_dwPageSize = sys.dwPageSize * 4;

	int nThreadCount = 0;
	for (DWORD i = 0; i < dwNumberOfCPU; i++)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, completionRoution, NULL, 0, NULL);
		if (NULL != hCompPort)
		{
			CloseHandle(hThread);
			nThreadCount++;
		}
	}
	if (nThreadCount <= 0)
	{
		_tprintf(_T("没有创建有效的工作线程\n"));
		CloseHandle(hCompPort);
		WSACleanup();
		return -1;
	}

	// 初始化内存池临界区
	InitializeCriticalSection(&g_csSObj);
	InitializeCriticalSection(&g_csBObj);

	LISTEN_OBJ *lobj = new LISTEN_OBJ();
	if (NULL == lobj)
	{
		_tprintf(_T("分配监听socket对象失败, errCode = %d\n"), WSAGetLastError());
		CloseHandle(hCompPort);
		WSACleanup();
		return -1;
	}
	lobj->init();

	lobj->sListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == lobj->sListenSock)
	{
		_tprintf(_T("创建监听socket失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	struct sockaddr_in laddr;
	memset(&laddr, 0x00, sizeof(laddr));
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(LISTEN_PORT);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(lobj->sListenSock, (SOCKADDR*)&laddr, sizeof(laddr)))
	{
		_tprintf(_T("监听socket绑定失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	// 创建事件，完成自动处理资源分配
	lobj->hPostAcceptExEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == lobj->hPostAcceptExEvent)
	{
		_tprintf(_T("监听事件创建失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (SOCKET_ERROR == WSAEventSelect(lobj->sListenSock, lobj->hPostAcceptExEvent, FD_ACCEPT))
	{
		_tprintf(_T("监听事件关联失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (NULL == CreateIoCompletionPort((HANDLE)lobj->sListenSock, hCompPort, (ULONG_PTR)lobj, 0))
	{
		_tprintf(_T("监听socket关联完成端口失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	if (SOCKET_ERROR == listen(lobj->sListenSock, SOMAXCONN))
	{
		_tprintf(_T("socket监听失败, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	for (size_t i = 0; i < 10; i++)
	{
		PostAcceptEx(lobj);
	}

	while (true)
	{
		int rt = WaitForSingleObject(lobj->hPostAcceptExEvent, INFINITE);
		if (WSA_WAIT_FAILED == rt)
		{
			_tprintf(_T("带来异常，推出\n"));
			return -1;
		}

		for (size_t i = 0; i < 10; i++)
		{
			PostAcceptEx(lobj);
		}
	}

    return 0;

error:
	delete lobj;
	CloseHandle(hCompPort);
	DeleteCriticalSection(&g_csSObj);
	DeleteCriticalSection(&g_csBObj);
	WSACleanup();
	system("pause");
	return -1;
}

unsigned int _stdcall completionRoution(LPVOID)
{
	ULONG_PTR key;
	BUFFER_OBJ* bobj;
	LPOVERLAPPED lpol;
	DWORD dwTranstion;
	BOOL bSuccess;

	while (true)
	{
		bSuccess = GetQueuedCompletionStatus(hCompPort, &dwTranstion, &key, &lpol, INFINITE);
		if (NULL == lpol)
		{
			printf("如果不是认为设定NULL为退出信号，那么就是发生重大错误，直接退出\n");
			return 0;
		}

		bobj = CONTAINING_RECORD(lpol, BUFFER_OBJ, ol);

		if (!bSuccess)
			bobj->pfnFailed((void*)key, bobj);
		else
			bobj->pfnSuccess(dwTranstion, (void*)key, bobj);
	}

	return 0;
}


