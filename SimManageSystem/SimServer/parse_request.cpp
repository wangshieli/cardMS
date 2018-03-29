#include "stdafx.h"

#include "singleton_data.h"

#include "parse_request.h"

#include <msgpack.hpp>

#include "parse_data.h"
#include "parse_user.h"
#include "parse_sim.h"
#include "parse_kh.h"
#include "parse_khjl.h"
#include "parse_llc.h"
#include "parse_lltc.h"
#include "parse_ssdq.h"
#include "parse_import.h"
#include "sql_test.h"

void ErrorInfo(BUFFER_OBJ* bobj, const TCHAR* pErrorInfo)
{
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	int nCmd = 0xbb;
	int nSubCmd = 0xbb;
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack(nCmd);
	msgPack.pack(nSubCmd);
	msgPack.pack(0);
	msgPack.pack(pErrorInfo);

	DealLast(sbuf, bobj);
}

bool doParseData(BUFFER_OBJ* bobj)
{
	bool bRtn = false;
	const TCHAR* pRequest = bobj->data;
	int nLen = bobj->dwRecvedCount;
	if (nLen < 8)
	{
		ErrorInfo(bobj, _T("数据太短了\n"));
		return bRtn;
	}
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  没有数据开始标志
	{
		ErrorInfo(bobj, _T("没有数据开始标志\n"));
		return bRtn;
	}
	int nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
	{
		ErrorInfo(bobj, _T("长度不够\n"));
		return bRtn;
	}
	byte nSum = pRequest[6 + nFrameLen];// 检验和
	if (nSum != csum((unsigned char*)pRequest + 6, nFrameLen))
	{
		ErrorInfo(bobj, _T("检验和失败\n"));
		return bRtn;
	}

	if (0x0d != pRequest[nFrameLen + 7])
	{
		ErrorInfo(bobj, _T("结尾错误\n"));
		return bRtn;
	}

	try
	{
		msgpack::unpacker unpack_;
		msgpack::object_handle result_;
		unpack_.reserve_buffer(nLen);
		memcpy_s(unpack_.buffer(), nLen, pRequest + 6, nFrameLen);
		unpack_.buffer_consumed(nFrameLen);
		unpack_.next(result_);
		if (msgpack::type::ARRAY != result_.get().type)
		{
			ErrorInfo(bobj, _T("数据不完整\n"));
			return bRtn;
		}
		int cmdno = result_.get().via.array.ptr->as<int>();
		switch (cmdno)
		{
		case CMD_USER:
		{
			bRtn = doParseUser(result_, bobj);
		}
		break;

		case CMD_SIM:
		{
			bRtn = doParseSim(result_, bobj);
		}
		break;

		case CMD_KH:
		{
			bRtn = doParseKh(result_, bobj);
		}
		break;

		case CMD_KHJL:
		{
			bRtn = doParseKhjl(result_, bobj);
		}
		break;

		case CMD_LLC:
		{
			bRtn = doParseLlc(result_, bobj);
		}
		break;

		case CMD_LLTC:
		{
			bRtn = doParseLltc(result_, bobj);
		}
		break;

		case CMD_SSDQ:
		{
			bRtn = doParseSsdq(result_, bobj);
		}
		break;

		case CMD_IMPORT:
		{
			bRtn = doParseImport(result_, bobj);
		}
		break;

		case CMD_SQLTEST:
		{
			bRtn = doSqlTest(result_, bobj);
		}
		break;

		default:
			ErrorInfo(bobj, _T("未知命令\n"));
			break;
		}
		/*if (cmdno == MSG_USER_0X0A)
		{
			return doParseUser(result_, bobj);
		}
		else if (cmdno == MSG_SIM_0X0B)
		{
			return doParseSim(result_, bobj);
		}
		else if (cmdno == MSG_KH_0X0C)
		{
			return doParseKh(result_, bobj);
		}
		else if (cmdno == MSG_KHJL_0X0E)
		{
			return doParseKhjl(result_, bobj);
		}
		else if (cmdno == MSG_LLC_0X09)
		{
			return doParseLlc(result_, bobj);
		}
		else if (cmdno == MSG_LLTC_0X08)
		{
			return doParseLltc(result_, bobj);
		}
		else if (cmdno == MSG_SSDQ_0X07)
		{
			return doParseSsdq(result_, bobj);
		}
		else
		{
			ErrorInfo(bobj, _T("未知命令\n"));
			return false;
		}*/
	}
	catch (msgpack::type_error e)
	{
		ErrorInfo(bobj, _T("数据类型错误\n"));
		return bRtn;
	}
	catch (msgpack::unpack_error e)
	{
		_tprintf(_T("解析错误: %s\n"), e.what());
		ErrorInfo(bobj, _T("解析错误\n"));
		return bRtn;
	}
	catch (...)
	{
		ErrorInfo(bobj, _T("解析出错\n"));
		return bRtn;
	}

	return bRtn;
}