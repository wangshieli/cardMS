#include "stdafx.h"
#include "singleton_data.h"

HANDLE hCompPort = NULL;
DWORD g_dwPageSize = 0;

GUID GuidAcceptEx = WSAID_ACCEPTEX,
GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

LPFN_ACCEPTEX lpfnAccpetEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;