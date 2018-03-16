#include "stdafx.h"
#include "singleton_data.h"
#include "complete_notification.h"
#include "post_request.h"
#include "objPool.h"

void AcceptCompFailed(void* _lobj, void* _c_obj)
{
	LISTEN_OBJ* lobj = (LISTEN_OBJ*)_lobj;
	BUFFER_OBJ* c_bobj = (BUFFER_OBJ*)_c_obj;
	SOCKET_OBJ* c_sobj = c_bobj->pRelatedSObj;

	// 关闭socket  释放资源
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

	// 将客户端绑定到完成端口
	if (NULL == CreateIoCompletionPort((HANDLE)c_sobj->sock, hCompPort, (ULONG_PTR)c_sobj, 0))
	{
		_tprintf(_T("客户端socket绑定完成端口失败, errCode = %d\n"), WSAGetLastError());
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

	// 数据处理
	if (NULL == strstr(c_bobj->data, "\r\n\r\n"))
	{
		c_bobj->SetIoRequestFunction(RecvZeroCompFailed, RecvZeroCompSuccess);
		if (!PostZeroRecv(c_sobj, c_bobj))
		{
			_tprintf(_T("客户端信息接收失败, errCode = %d\n"), WSAGetLastError());
			goto error;
		}
	}
	else
	{
		// 处理命令
		_tprintf(_T("接收到的数据: %s\n"), c_bobj->data);
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
		// 处理命令
		_tprintf(_T("接收到的数据: %s\n"), c_bobj->data);
	}
}