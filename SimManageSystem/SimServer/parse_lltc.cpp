#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_lltc.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnLltcInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(2);
		_variant_t varTclx = pRecord->GetCollect("tcfl");
		AddData(varTclx, msgPack);

		_variant_t varTcmc = pRecord->GetCollect("tcmc");
		AddData(varTcmc, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	pRecord->Close();
	pRecord.Release();
	pRecord = NULL;
}

bool doParseLltc(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:
	{
		std::string strTcfl = (pObj++)->as<std::string>();
		std::string strTcmc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(B_MSG_LLTC_0X8B);
		msgPack.pack(nSubCmd);

		const TCHAR* pSql = _T("insert into lltc_tbl (id,tcfl,tcmc) value(null,'%s','%s')");

		TCHAR strInsert[512];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 512, pSql, strTcfl.c_str(), strTcmc.c_str());
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

	case DO_SELECT_BY_KEY:
	{
		std::string strTcmc = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strTcmc.c_str());
		const TCHAR* pSql = _T("select * from lltc_tbl where tcmc = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strTcmc.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
		}
		int lRstCount = pRecord->GetRecordCount();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(B_MSG_LLTC_0X8B);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnLltcInfo(pRecord, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_ID:
	{
		int nTag = (pObj++)->as<int>();
		_tprintf(_T("ntag = %d\n"), nTag);

		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		const TCHAR* pSql = _T("select * from lltc_tbl where id  between %d and %d");// 主键范围
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
		msgPack.pack(B_MSG_LLTC_0X8B);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnLltcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}