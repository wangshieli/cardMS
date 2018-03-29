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
		msgPack.pack_array(4);
		_variant_t varId = pRecord->GetCollect("username");
		AddData(varId, msgPack);

		_variant_t varUsername = pRecord->GetCollect("username");
		AddData(varUsername, msgPack);

		_variant_t varPassword = pRecord->GetCollect("password");
		AddData(varPassword, msgPack);

		_variant_t varXgrq = pRecord->GetCollect("xgrq");
		AddData(varXgrq, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	ReleaseRecordset(pRecord);
}

bool doParseUser(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_USER;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SUBCMD_USER_ADD:
	{
		std::string strUsername = (pObj++)->as<std::string>();
		std::string strPassword = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, _T("insert into user_tbl (id,username,password,xgrq) value(null,'%s','%s',now())"), strUsername.c_str(), strPassword.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_USER_GET_01:
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
		msgPack.pack_array(4);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(0);
		msgPack.pack_array(lRstCount);

		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_USER_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1) + 1;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from user_tbl limit %d,200");// Ö÷¼ü·¶Î§
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart);
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		long lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(5);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(nTag);
		msgPack.pack(0);
		msgPack.pack_array(lRstCount);

		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_USER_LOGIN:
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

		long lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(4);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(0);
		msgPack.pack_array(lRstCount);

		ReturnUserInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_USER_CHECK_UNAME:
	{
		std::string strUsername = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		const TCHAR* pSql = _T("select * from user_tbl where username = '%s'");
		_stprintf_s(sql, 256, pSql, strUsername.c_str());
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1);
			ReleaseRecordset(pRecord);
			DealLast(sbuf, bobj);
			return false;
		}
		if (!pRecord->adoEOF)
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1);
			ReleaseRecordset(pRecord);
			DealLast(sbuf, bobj);
			return false;
		}
		ReleaseRecordset(pRecord);
		ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0);

		DealLast(sbuf, bobj);
	}
	break;

	//case USER_MODIFY_PASSWORD:
	//{
	//	std::string strUsername = (pObj++)->as<std::string>();
	//	std::string strPassword = (pObj++)->as<std::string>();
	//	std::string strNPassword = (pObj++)->as<std::string>();

	//	msgpack::sbuffer sbuf;
	//	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	//	sbuf.write("\xfb\xfc", 6);

	//	TCHAR sql[256];
	//	memset(sql, 0x00, sizeof(sql));
	//	_stprintf_s(sql, 256, _T("update user_tbl set password = '%s' where username = '%s' and password = '%s'"),
	//		strNPassword.c_str(), strUsername.c_str(), strPassword.c_str());
	//	CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

	//	DealLast(sbuf, bobj);
	//}
	//break;

	//case USER_MODIFY_PRIVILEGE:
	//{
	//	std::string strUsername = (pObj++)->as<std::string>();
	//	std::string strPassword = (pObj++)->as<std::string>();
	//	int l1 = (pObj++)->as<int>();
	//	int l2 = (pObj++)->as<int>();

	//	msgpack::sbuffer sbuf;
	//	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	//	sbuf.write("\xfb\xfc", 6);

	//	TCHAR sql[256];
	//	memset(sql, 0x00, sizeof(sql));
	//	_stprintf_s(sql, 256, _T("update user_tbl set l1 = %d, l2 = %d where username = '%s' and password = '%s'"),
	//		l1, l2, strUsername.c_str(), strPassword.c_str());
	//	CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

	//	DealLast(sbuf, bobj);
	//}
	//break;
	
	default:
		break;
	}

	return true;
}