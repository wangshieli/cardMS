#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_user.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnUserInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(3);
		_variant_t varUsername = pRecord->GetCollect("username");
		AddData(varUsername, msgPack);

		_variant_t varPassword = pRecord->GetCollect("password");
		AddData(varPassword, msgPack);

		_variant_t varl1 = pRecord->GetCollect("xgrq");
		AddData(varl1, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	ReleaseRecordset(pRecord);
}

bool doParseUser(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	int nCmd = B_MSG_USER_0XAB;

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:// 用户注册
	{
		std::string strUsername = (pObj++)->as<std::string>();
		std::string strPassword = (pObj++)->as<std::string>();
		int l1 = (pObj++)->as<int>();
		int l2 = (pObj++)->as<int>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		const TCHAR* pSql = _T("select * from user_tbl where username = '%s'");
		_stprintf_s(sql, 256, pSql, strUsername.c_str());
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}
		ReleaseRecordset(pRecord);

		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, _T("insert into user_tbl (id,username,password,xgrq) value(null,'%s','%s',now())"), strUsername.c_str(), strPassword.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:// 根据用户名查询
	{
		std::string strUsername = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from user_tbl where username = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strUsername.c_str());
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		int lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(4 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);
		msgPack.pack(_T("success"));

		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	
	case DO_SELECT_BY_ID:// 查询user_tbl全部信息
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from user_tbl where id  between %d and %d");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nEnd);
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		long lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(4 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);
		msgPack.pack(_T("success"));

		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_UPDATE_DATA:
	{
		std::string strUsername = (pObj++)->as<std::string>();
		std::string strPassword = (pObj++)->as<std::string>();
		int l1 = (pObj++)->as<int>();
		int l2 = (pObj++)->as<int>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, _T("update user_tbl set l1 = %d, l2 = %d where username = '%s' and password = '%s'"),
			l1, l2, strUsername.c_str(), strPassword.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_LOGIN:// 用户登陆
	{
		std::string strUsername = (pObj++)->as<std::string>();
		std::string strPassword = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from user_tbl where username = '%s' and password = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strUsername.c_str(), strPassword.c_str());
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}
		
		int l1 = pRecord->GetCollect("l1");
		int l2 = pRecord->GetCollect("l2");
		ReleaseRecordset(pRecord);

		msgPack.pack_array(6);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);	
		msgPack.pack(1);
		msgPack.pack(_T("success"));
		msgPack.pack(l1);
		msgPack.pack(l2);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_MODIFY_PASSWORD:
	{
		std::string strUsername = (pObj++)->as<std::string>();
		std::string strPassword = (pObj++)->as<std::string>();
		std::string strNPassword = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, _T("update user_tbl set password = '%s' where username = '%s' and password = '%s'"),
			strNPassword.c_str(), strUsername.c_str(), strPassword.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	
	default:
		break;
	}

	return true;
}