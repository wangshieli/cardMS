#include "stdafx.h"

#include "singleton_data.h"

#include "parse_request.h"
#include "db_operation.h"

#include "post_request.h"
#include "complete_notification.h"
#include "objPool.h"

#include <msgpack.hpp>
#include <string>
#include <iostream>

byte csum(unsigned char *addr, int count)
{
	byte sum = 0;
	for (int i = 0; i< count; i++)
	{
		sum += (byte)addr[i];
	}
	return sum;
}

typedef bool(*ProcessFun)(msgpack::unpacked& result_);
typedef enum
{
	OT_NULL,
	OT_TEST
}OP_TYPE;

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

	memcpy(bobj->data, pCh, nLen);// 优化
	bobj->dwRecvedCount = nLen;
	//Send(pCh, nLen);
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

void ReturnUserInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(4);
		_variant_t varUsername = pRecord->GetCollect("username");
		if (varUsername.vt == VT_BSTR)
		{
			//const TCHAR* p = _T("wang");
			msgPack.pack((const TCHAR*)(_bstr_t)varUsername);
			//msgPack.pack(p);
		}
		_variant_t varPassword = pRecord->GetCollect("password");
		if (varPassword.vt == VT_BSTR)
		{
			msgPack.pack((const TCHAR*)(_bstr_t)varPassword);
		}
		int l1 = pRecord->GetCollect("l1");
		msgPack.pack(l1);
		int l2 = pRecord->GetCollect("l2");
		msgPack.pack(l2);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}
}

bool doParseUser(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case 1:// 用户登陆
	{
		std::string strUsername = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strUsername.c_str());
		std::string strPassword = (pObj++)->as<std::string>();
		_tprintf(_T("p1 = %s\n"), strPassword.c_str());

		const TCHAR* pSql = _T("select * from user_tbl where username = '%s' and password = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strUsername.c_str(), strPassword.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
			return false;
		}
		int nn = pRecord->GetRecordCount();
		int l1 = pRecord->GetCollect("l1");
		int l2 = pRecord->GetCollect("l2");
		_tprintf(_T("level: %d, %d, %d\n"), l1, l2, nn);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x1;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(5);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);
		msgPack.pack(1);// 登陆成功 1  失败0
		msgPack.pack(l1);// 需要返回的权限
		msgPack.pack(l2);// 需要返回的权限
		// 增加发送
		DealLast(sbuf, bobj);
	}
	break;
	case 2:// 用户注册
	{
		std::string strUsername = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strUsername.c_str());
		std::string strPassword = (pObj++)->as<std::string>();
		_tprintf(_T("p1 = %s\n"), strPassword.c_str());
		int l1 = (pObj++)->as<int>();
		int l2 = (pObj++)->as<int>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x2;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);

		TCHAR strInsert[256];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 256, _T("insert into user_tbl (username,password,l1,l2) value('%s','%s',%d,%d)"), strUsername.c_str(), strPassword.c_str(), l1, l2);
		if (!ExcuteSql(strInsert, true))
		{
			msgPack.pack(0);
			_tprintf(_T("插入失败\n"));
		}
		else
		{
			msgPack.pack(1);// 登陆成功 1  失败0
		}
		
		DealLast(sbuf, bobj);
	}
	break;
	case 3:
	{
		std::string strUsername = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strUsername.c_str());
		std::string strPassword = (pObj++)->as<std::string>();
		_tprintf(_T("p1 = %s\n"), strPassword.c_str());
		int l1 = (pObj++)->as<int>();
		int l2 = (pObj++)->as<int>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x3;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);

		TCHAR strUpdate[256];
		memset(strUpdate, 0x00, sizeof(strUpdate));
		_stprintf_s(strUpdate, 256, _T("update user_tbl set l1 = %d, l2 = %d where username = '%s' and password = '%s'"), l1, l2, strUsername.c_str(), strPassword.c_str());
		if (!ExcuteSql(strUpdate, true))
		{
			msgPack.pack(0);
			_tprintf(_T("更新失败\n"));
		}
		else
		{
			msgPack.pack(1);// 登陆成功 1  失败0
		}

		DealLast(sbuf, bobj);
	}
	break;
	case 4:// 查询user_tbl全部信息
	{
		int nTag = (pObj++)->as<int>();
		const TCHAR* psql = _T("select * from user_tbl");
		_RecordsetPtr pRecord;
		if (!GetRecordSet(psql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
			return false;
		}

		long lRstCount = pRecord->GetRecordCount();
		_tprintf(_T("查询到的数据条数:%ld\n"), lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x4;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);
		msgPack.pack(1);// 登陆成功 1  失败0
		
		//VARIANT_BOOL bRt = pRecord->GetadoEOF();
		//while (!bRt)
		//{
		//	msgPack.pack_array(4);
		//	_variant_t varUsername = pRecord->GetCollect("username");
		//	if (varUsername.vt == VT_BSTR)
		//	{
		//		//const TCHAR* p = _T("wang");
		//		msgPack.pack((const TCHAR*)(_bstr_t)varUsername);
		//		//msgPack.pack(p);
		//	}
		//	_variant_t varPassword = pRecord->GetCollect("password");
		//	if (varPassword.vt == VT_BSTR)
		//	{
		//		msgPack.pack((const TCHAR*)(_bstr_t)varPassword);
		//	}
		//	int l1 = pRecord->GetCollect("l1");
		//	msgPack.pack(l1);
		//	int l2 = pRecord->GetCollect("l2");
		//	msgPack.pack(l2);

		//	pRecord->MoveNext();
		//	bRt = pRecord->GetadoEOF();
		//}
						 // 增加发送
		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	case 5:// 根据用户名查询
	{
		std::string strUsername = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strUsername.c_str());
		const TCHAR* pSql = _T("select * from user_tbl where username = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strUsername.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
			return false;
		}
		int lRstCount = pRecord->GetRecordCount();
		int l1 = pRecord->GetCollect("l1");
		int l2 = pRecord->GetCollect("l2");
		_tprintf(_T("level: %d, %d, %d\n"), l1, l2, lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x5;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3+ lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		//VARIANT_BOOL bRt = pRecord->GetadoEOF();
		//while (!bRt)
		//{
		//	msgPack.pack_array(4);
		//	_variant_t varUsername = pRecord->GetCollect("username");
		//	if (varUsername.vt == VT_BSTR)
		//	{
		//		//const TCHAR* p = _T("wang");
		//		msgPack.pack((const TCHAR*)(_bstr_t)varUsername);
		//		//msgPack.pack(p);
		//	}
		//	_variant_t varPassword = pRecord->GetCollect("password");
		//	if (varPassword.vt == VT_BSTR)
		//	{
		//		msgPack.pack((const TCHAR*)(_bstr_t)varPassword);
		//	}
		//	int l1 = pRecord->GetCollect("l1");
		//	msgPack.pack(l1);
		//	int l2 = pRecord->GetCollect("l2");
		//	msgPack.pack(l2);

		//	pRecord->MoveNext();
		//	bRt = pRecord->GetadoEOF();
		//}

		ReturnUserInfo(pRecord, msgPack);
		
		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}

void ReturnSimInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(2);
		_variant_t varSim = pRecord->GetCollect("jrhm");
		if (varSim.vt == VT_BSTR)
		{
			//const TCHAR* p = _T("wang");
			msgPack.pack((const TCHAR*)(_bstr_t)varSim);
			//msgPack.pack(p);
		}
		_variant_t varIccid = pRecord->GetCollect("iccid");
		if (varIccid.vt == VT_BSTR)
		{
			msgPack.pack((const TCHAR*)(_bstr_t)varIccid);
		}

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}
}

bool doParseSim(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case 1:
	{
		std::string strSim = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strSim.c_str());
		std::string strIccid = (pObj++)->as<std::string>();
		_tprintf(_T("p1 = %s\n"), strIccid.c_str());

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x6;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);

		TCHAR strInsert[256];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 256, _T("insert into sim_tbl (jrhm,iccid) value('%s','%s')"), strSim.c_str(), strIccid.c_str());
		if (!ExcuteSql(strInsert, true))
		{
			msgPack.pack(0);
			_tprintf(_T("插入失败\n"));
		}
		else
		{
			msgPack.pack(1);// 登陆成功 1  失败0
		}

		DealLast(sbuf, bobj);
	}
	break;
	case 2:// 更加接入号码查sim
	{
		std::string strSim = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strSim.c_str());
		const TCHAR* pSql = _T("select * from sim_tbl where jrhm = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strSim.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
			return false;
		}
		int lRstCount = pRecord->GetRecordCount();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x7;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnSimInfo(pRecord, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	case 3:
	{
		int nTag = (pObj++)->as<int>();
		_tprintf(_T("ntag = %d\n"), nTag);

		//int nStart = 60 * (nTag - 1);
		//int nEnd = 60 * nTag;
		// select * from sim_tbl where id  between %d and %d
		
		const TCHAR* psql = _T("select * from sim_tbl");// 主键范围
		_RecordsetPtr pRecord;
		if (!GetRecordSet(psql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
			return false;
		}

		long lRstCount = pRecord->GetRecordCount();
		_tprintf(_T("查询到的数据条数:%ld\n"), lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x8;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);// 登陆返回
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}
	return true;
}

bool doParseData(BUFFER_OBJ* bobj)
{
	const TCHAR* pRequest = bobj->data;
	int nLen = bobj->dwRecvedCount;
	if (nLen < 8)
	{
		_tprintf(_T("数据太短了\n"));
		return false;
	}
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  没有数据开始标志
	{
		_tprintf(_T("没有数据开始标志\n"));
		return false;
	}
	int nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
	{
		_tprintf(_T("长度不够\n"));
		return false;
	}
	byte nSum = pRequest[6 + nFrameLen];// 检验和
	if (nSum != csum((unsigned char*)pRequest + 6, nFrameLen))
	{
		_tprintf(_T("检验和失败\n"));
		return false;
	}

	if (0x0d != pRequest[nFrameLen + 7])
	{
		_tprintf(_T("结尾错误\n"));
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
			return false;
		}
		int cmdno = result_.get().via.array.ptr->as<int>();
		if (cmdno > -1)
		{
			_tprintf(_T("nCmd = %d\n"), cmdno);
		//	return doParseUser(result_, bobj);
			return doParseSim(result_, bobj);
		}
	}
	catch (msgpack::type_error e)
	{
		_tprintf(_T("数据类型错误\n"));
		return false;
	}
	catch (msgpack::unpack_error e)
	{
		_tprintf(_T("解析错误: %s\n"), e.what());
		return false;
	}
	catch (...)
	{
		_tprintf(_T("解析出错\n"));
		return false;
	}

	return true;
}

// 增加用户
bool pr_ZJYH()
{
	TCHAR sql[256];
	memset(sql, 0x00, sizeof(sql));
	_stprintf_s(sql, 256, _T("insert into tbl (id,name,address,year) value(3,'往事了', '周口', '2018-01-23')"));
	ExcuteSql(sql, true);
	return true;
}

// 增加客户
bool pr_ZJKH()
{
	return true;
}

// 增加客户经理
bool pr_ZJKHJL()
{
	return true;
}

// 增加sim卡信息
bool pr_ZJSIMKXX()
{
	return true;
}

// 查询sim卡信息
bool pr_CXSIMKXX()
{
	return true;
}

