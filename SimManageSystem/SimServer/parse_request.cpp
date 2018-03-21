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

void ErrorInfo(BUFFER_OBJ* bobj, const TCHAR* pErrorInfo)
{
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	int nCmd = 0xbb;
	int nSubCmd = 0xbb;
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack(nCmd);
	msgPack.pack(nSubCmd);
	msgPack.pack(pErrorInfo);

	DealLast(sbuf, bobj);
}

bool doParseData(BUFFER_OBJ* bobj)
{
	const TCHAR* pRequest = bobj->data;
	int nLen = bobj->dwRecvedCount;
	if (nLen < 8)
	{
		_tprintf(_T("数据太短了\n"));
		ErrorInfo(bobj, _T("数据太短了\n"));
		return false;
	}
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  没有数据开始标志
	{
		_tprintf(_T("没有数据开始标志\n"));
		ErrorInfo(bobj, _T("没有数据开始标志\n"));
		return false;
	}
	int nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
	{
		_tprintf(_T("长度不够\n"));
		ErrorInfo(bobj, _T("长度不够\n"));
		return false;
	}
	byte nSum = pRequest[6 + nFrameLen];// 检验和
	if (nSum != csum((unsigned char*)pRequest + 6, nFrameLen))
	{
		_tprintf(_T("检验和失败\n"));
		ErrorInfo(bobj, _T("检验和失败\n"));
		return false;
	}

	if (0x0d != pRequest[nFrameLen + 7])
	{
		_tprintf(_T("结尾错误\n"));
		ErrorInfo(bobj, _T("结尾错误\n"));
		return false;
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
			_tprintf(_T("数据不完整\n"));
			ErrorInfo(bobj, _T("数据不完整\n"));
			return false;
		}
		int cmdno = result_.get().via.array.ptr->as<int>();
		if (cmdno == MSG_USER_0X0A)
		{
			return doParseUser(result_, bobj);
		}
		else if (cmdno == MSG_SIM_OX0B)
		{
			return doParseSim(result_, bobj);
		}
		else if (cmdno == MSG_KH_OX0C)
		{
			return doParseKh(result_, bobj);
		}
		else if (cmdno == MSG_KHJL_OX0E)
		{
			return doParseKhjl(result_, bobj);
		}
		else if (cmdno == MSG_LLC_OX09)
		{
			return doParseLlc(result_, bobj);
		}
		else if (cmdno == MSG_LLTC_OX08)
		{
			return doParseLltc(result_, bobj);
		}
		else
		{
			_tprintf(_T("未知命令\n"));
			ErrorInfo(bobj, _T("未知命令\n"));
			return false;
		}
	}
	catch (msgpack::type_error e)
	{
		_tprintf(_T("数据类型错误\n"));
		ErrorInfo(bobj, _T("数据类型错误\n"));
		return false;
	}
	catch (msgpack::unpack_error e)
	{
		_tprintf(_T("解析错误: %s\n"), e.what());
		ErrorInfo(bobj, _T("解析错误\n"));
		return false;
	}
	catch (...)
	{
		_tprintf(_T("解析出错\n"));
		ErrorInfo(bobj, _T("解析出错\n"));
		return false;
	}

	return true;
}