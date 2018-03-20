#include "stdafx.h"
#include "singleton_data.h"
#include "complete_notification.h"
#include "post_request.h"
#include "objPool.h"

#include "parse_request.h"

struct tcp_keepalive alive_in = { TRUE, 1000 * 10, 1000 };
struct tcp_keepalive alive_out = { 0 };
unsigned long ulBytesReturn = 0;

//void cn_closesocket(SOCKET_OBJ* sobj)
//{
//	if (NULL != sobj)
//	{
//		if (INVALID_SOCKET != sobj->sock)
//		{
//			closesocket(sobj->sock);
//			sobj->sock = INVALID_SOCKET;
//		}
//	}
//}

void AcceptCompFailed(void* _lobj, void* _c_obj)
{
	LISTEN_OBJ* lobj = (LISTEN_OBJ*)_lobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_c_obj;
	SOCKET_OBJ* c_sobj = c_bobj->pRelatedSObj;

	// �ر�socket  �ͷ���Դ
	closesocket(c_sobj->sock);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void AcceptCompSuccess(DWORD dwTranstion, void* _lobj, void* _c_bobj)
{
	if (dwTranstion <= 0)
		return AcceptCompFailed(_lobj, _c_bobj);

	LISTEN_OBJ* lobj = (LISTEN_OBJ*)_lobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_c_bobj;
	SOCKET_OBJ* c_sobj = c_bobj->pRelatedSObj;

	//if (SOCKET_ERROR == WSAIoctl(c_sobj->sock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
	//	&alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL))
	//	_tprintf(_T("���ÿͻ��������������ʧ��, errCode = %d\n"), WSAGetLastError());

	//setsockopt(c_sobj->sock,
	//	SOL_SOCKET,
	//	SO_UPDATE_ACCEPT_CONTEXT,
	//	(char*)&lobj->sListenSock, sizeof(lobj->sListenSock));
	//_tprintf(_T("setsockoptn errcode = %d\n"), WSAGetLastError());

	// ���ͻ��˰󶨵���ɶ˿�
	if (NULL == CreateIoCompletionPort((HANDLE)c_sobj->sock, hCompPort, (ULONG_PTR)c_sobj, 0))
	{
		_tprintf(_T("�ͻ���socket����ɶ˿�ʧ��, errCode = %d\n"), WSAGetLastError());
		goto error;
	}

	c_bobj->dwRecvedCount += dwTranstion;

	SOCKADDR* localAddr,
		*remoteAddr;
	localAddr = NULL;
	remoteAddr = NULL;
	int localAddrlen,
		remoteAddrlen;

	lpfnGetAcceptExSockaddrs(c_bobj->data, c_bobj->datalen - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&localAddr, &localAddrlen,
		&remoteAddr, &remoteAddrlen);

	// ���ݴ���
	//if (NULL == strstr(c_bobj->data, "\r\n\r\n"))
	if (NULL == memchr(c_bobj->data + c_bobj->dwRecvedCount - 1, 0x0d, 1))
	{
		c_bobj->SetIoRequestFunction(RecvZeroCompFailed, RecvZeroCompSuccess);
		if (!PostZeroRecv(c_sobj, c_bobj))
		{
			_tprintf(_T("�ͻ�����Ϣ����ʧ��, errCode = %d\n"), WSAGetLastError());
			goto error;
		}

		//Sleep(1000 * 5);
		//if (SOCKET_ERROR == shutdown(c_sobj->sock, 0))
		//	_tprintf(_T("shutdow errcode = %d\n"), WSAGetLastError());
	}
	else
	{
		doParseData(c_bobj);
		// ��������
		//printf("%d\n", c_bobj->dwRecvedCount);
		//_tprintf(_T("���յ�������: %s\n"), c_bobj->data);
		//msgpack::sbuffer sbuf(c_bobj->dwRecvedCount);
		//memcpy_s(sbuf.data(), c_bobj->dwRecvedCount, c_bobj->data, c_bobj->dwRecvedCount);
		//msgpack::object_handle oh =
		//	msgpack::unpack(c_bobj->data, c_bobj->dwRecvedCount);

		//// print the deserialized object.
		//msgpack::object obj = oh.get();
		//std::cout << obj << std::endl;

		// �����յ�������
		//doParseData(c_bobj->data);
		//// �жϲ�������
		//doParse(1);
		//// ����ҵ����

		//_stprintf_s(c_bobj->data, c_bobj->datalen, _T("�Ѿ��յ���Ϣ�����ڽ��д���"));
		//c_bobj->dwRecvedCount = _tcslen(c_bobj->data) + 1;
		//c_bobj->SetIoRequestFunction(SendCompFailed, SendCompSuccess);
		//if (!PostSend(c_sobj, c_bobj))
		//{
		//	goto error;
		//}
	}

	return;

error:
	closesocket(c_sobj->sock);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
	return;
}

void RecvZeroCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	closesocket(c_sobj->sock);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void RecvZeroCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->SetIoRequestFunction(RecvCompFailed, RecvCompSuccess);
	if (!PostRecv(c_sobj, c_bobj))
	{
		closesocket(c_sobj->sock);
		freeSObj(c_sobj);
		freeBObj(c_bobj);
		return;
	}
}

void RecvCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	closesocket(c_sobj->sock);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void RecvCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return RecvCompFailed(_sobj, _bobj);

	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwRecvedCount += dwTransion;
	if (NULL == strstr(c_bobj->data, "\r\n\r\n"))
	{
		c_bobj->SetIoRequestFunction(RecvZeroCompFailed, RecvZeroCompSuccess);
		if (!PostZeroRecv(c_sobj, c_bobj))
		{
			closesocket(c_sobj->sock);
			freeSObj(c_sobj);
			freeBObj(c_bobj);
			return;
		}
	}
	else
	{
		// ��������
		_tprintf(_T("���յ�������: %s\n"), c_bobj->data);
		_stprintf_s(c_bobj->data, c_bobj->datalen, _T("�Ѿ��յ���Ϣ�����ڽ��д���"));
		c_bobj->dwRecvedCount = _tcslen(c_bobj->data) + 1;
		c_bobj->SetIoRequestFunction(SendCompFailed, SendCompSuccess);
		if (!PostSend(c_sobj, c_bobj))
		{
			closesocket(c_sobj->sock);
			freeSObj(c_sobj);
			freeBObj(c_bobj);
			return;
		}
	}
}

void SendCompFailed(void* _sobj, void* _bobj)
{
	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	closesocket(c_sobj->sock);
	freeSObj(c_sobj);
	freeBObj(c_bobj);
}

void SendCompSuccess(DWORD dwTransion, void* _sobj, void* _bobj)
{
	if (dwTransion <= 0)
		return SendCompFailed(_sobj, _bobj);

	SOCKET_OBJ* c_sobj = (SOCKET_OBJ*)_sobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_bobj;

	c_bobj->dwSendedCount += dwTransion;
	if (c_bobj->dwSendedCount < c_bobj->dwRecvedCount)
	{
		if (!PostSend(c_sobj, c_bobj))
		{
			closesocket(c_sobj->sock);
			freeSObj(c_sobj);
			freeBObj(c_bobj);
			return;
		}
	}

	// ��ҪͶ��һ���رղ�����
}