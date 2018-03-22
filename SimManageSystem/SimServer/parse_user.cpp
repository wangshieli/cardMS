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
		_variant_t varUsername = pRecord->GetCollect("username");
		AddData(varUsername, msgPack);

		_variant_t varPassword = pRecord->GetCollect("password");
		AddData(varPassword, msgPack);

		_variant_t varl1 = pRecord->GetCollect("l1");
		AddData(varl1, msgPack);

		_variant_t varl2 = pRecord->GetCollect("l2");
		AddData(varl2, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	pRecord->Close();
	pRecord.Release();
	pRecord = NULL;
}

bool doParseUser(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:// 用户注册
	{
		std::string strUsername = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strUsername.c_str());
		std::string strPassword = (pObj++)->as<std::string>();
		_tprintf(_T("p1 = %s\n"), strPassword.c_str());
		int l1 = (pObj++)->as<int>();
		int l2 = (pObj++)->as<int>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(B_MSG_USER_OXAB);
		msgPack.pack(nSubCmd);

		TCHAR strInsert[256];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 256, _T("insert into user_tbl (id,username,password,l1,l2) value(null,'%s','%s',%d,%d)"), strUsername.c_str(), strPassword.c_str(), l1, l2);
		if (!ExcuteSql(strInsert, true))
		{
			msgPack.pack(0);
			_tprintf(_T("插入失败\n"));
		}
		else
		{
			msgPack.pack(1);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:// 根据用户名查询
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
		}
		int lRstCount = pRecord->GetRecordCount();
		int l1 = pRecord->GetCollect("l1");
		int l2 = pRecord->GetCollect("l2");
		_tprintf(_T("level: %d, %d, %d\n"), l1, l2, lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(B_MSG_USER_OXAB);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnUserInfo(pRecord, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	
	case DO_SELECT_BY_ID:// 查询user_tbl全部信息
	{
		int nTag = (pObj++)->as<int>();
		_tprintf(_T("ntag = %d\n"), nTag);

		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		const TCHAR* pSql = _T("select * from user_tbl where id  between %d and %d");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nEnd);

		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
		}

		long lRstCount = pRecord->GetRecordCount();
		_tprintf(_T("查询到的数据条数:%ld\n"), lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(B_MSG_USER_OXAB);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

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
		msgPack.pack_array(3);
		msgPack.pack(B_MSG_USER_OXAB);
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
			msgPack.pack(1);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case DO_LOGIN:// 用户登陆
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
		pRecord->Close();
		pRecord.Release();
		pRecord = NULL;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(5);
		msgPack.pack(B_MSG_USER_OXAB);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);
		msgPack.pack(l1);
		msgPack.pack(l2);

		DealLast(sbuf, bobj);
	}
	break;
	
	default:
		break;
	}

	return true;
}