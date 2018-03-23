#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_data.h"
#include "post_request.h"
#include "complete_notification.h"
#include "objPool.h"
#include "db_operation.h"

byte csum(unsigned char *addr, int count)
{
	byte sum = 0;
	for (int i = 0; i< count; i++)
	{
		sum += (byte)addr[i];
	}
	return sum;
}

void DealLast(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj)
{
	char* pCh = sBuf.data();
	int nLen = sBuf.size();
	byte pData[1024 * 10];
	memset(pData, 0, 1024 * 10);
	memcpy(pData, pCh + 6, nLen - 6);
	byte nSum = csum(pData, nLen - 6);
	sBuf.write("\x00", 1);
	memcpy(pCh + nLen, &nSum, 1);
	sBuf.write("\x0d", 1);
	nLen = sBuf.size();
	nLen -= 8;
	memcpy(pCh + 2, &nLen, 4);
	nLen += 8;

	memcpy(bobj->data, pCh, nLen);// ÓÅ»¯
	bobj->dwRecvedCount = nLen;
	bobj->SetIoRequestFunction(SendCompFailed, SendCompSuccess);
	SOCKET_OBJ* sobj = bobj->pRelatedSObj;
	if (!PostSend(sobj, bobj))
	{
		closesocket(sobj->sock);
		freeSObj(sobj);
		freeBObj(bobj);
		return;
	}
}

void AddData(const _variant_t& var, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	switch (var.vt)
	{
	case VT_BSTR:
	case VT_LPSTR:
	case VT_LPWSTR:
	{
		msgPack.pack((const TCHAR*)(_bstr_t)var);
	}
	break;

	case VT_I1:
	case VT_UI1:
	{
		msgPack.pack(var.bVal);
	}
	break;

	case VT_I2:
	{
		msgPack.pack(var.iVal);
	}
	break;

	case VT_UI2:
	{
		msgPack.pack(var.uiVal);
	}
	break;

	case VT_INT:
	{
		msgPack.pack(var.intVal);
	}
	break;

	case VT_I4:
	case VT_I8:
	{
		msgPack.pack(var.lVal);
	}
	break;

	case VT_UINT:
	{
		msgPack.pack(var.uintVal);
	}
	break;

	case VT_UI4:
	case VT_UI8:
	{
		msgPack.pack(var.ulVal);
	}
	break;

	case VT_R4:
	{
		msgPack.pack(var.fltVal);
	}
	break;

	case VT_R8:
	{
		msgPack.pack(var.dblVal);
	}
	break;

	case VT_DATE:
	{
		SYSTEMTIME st;
		VariantTimeToSystemTime(var.date, &st);
		TCHAR date[32];
		memset(date, 0x00, sizeof(date));
		_stprintf_s(date, 32, _T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		msgPack.pack(date);
	}
	break;

	case VT_NULL:
	case VT_EMPTY:
	{
		msgPack.pack(_T(""));
	}
	break;

	case VT_UNKNOWN:
	default:
		msgPack.pack(_T("VT_UNKNOWN"));
		break;
	}
}

void AddYYYYMMDD(const _variant_t& var, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	switch (var.vt)
	{
	case VT_DATE:
	{
		SYSTEMTIME st;
		VariantTimeToSystemTime(var.date, &st);
		TCHAR date[32];
		memset(date, 0x00, sizeof(date));
		_stprintf_s(date, 32, _T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
		msgPack.pack(date);
	}
	break;

	case VT_NULL:
	case VT_EMPTY:
	{
		msgPack.pack(_T(""));
	}
	break;

	case VT_UNKNOWN:
	default:
		msgPack.pack(_T("VT_UNKNOWN"));
		break;
	}
}

void ReturnSimpleInfo(msgpack::packer<msgpack::sbuffer>& msgPack, int nCmd, int nSubCmd, int nSuccess, const TCHAR* pErrInfo)
{
	msgPack.pack_array(4);
	msgPack.pack(nCmd);
	msgPack.pack(nSubCmd);
	msgPack.pack(nSuccess);
	msgPack.pack(pErrInfo);
}

bool GetRecordSetDate(const TCHAR* sql, _RecordsetPtr& pRecord, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	if (!GetRecordSet(sql, pRecord, adCmdText, true))
	{
		ReturnSimpleInfo(msgPack, B_MSG_SIM_0XBB, nSubCmd, 0, _T("Ê§°Ü"));
		ReleaseRecordset(pRecord);
		return false;
	}
	if (pRecord->adoEOF)
	{
		ReturnSimpleInfo(msgPack, B_MSG_SIM_0XBB, nSubCmd, 0, _T("Ê§°Ü"));
		ReleaseRecordset(pRecord);
		return false;
	}

	return true;
}

void CheckSqlResult(const TCHAR* sql, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	if (!ExcuteSql(sql, true))
	{
		ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("Ê§°Ü"));
	}
	else
	{
		ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
	}
}