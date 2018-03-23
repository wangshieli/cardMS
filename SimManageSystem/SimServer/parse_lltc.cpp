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
	int nCmd = B_MSG_LLTC_0X8B;

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:
	{
		std::string strTcfl = (pObj++)->as<std::string>();
		std::string strTcmc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("insert into lltc_tbl (id,tcfl,tcmc) value(null,'%s','%s')");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strTcfl.c_str(), strTcmc.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:
	{
		std::string strTcmc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from lltc_tbl where tcmc = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strTcmc.c_str());
		
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
		msgPack.pack(_T("成功"));

		ReturnLltcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_ID:
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from lltc_tbl where id  between %d and %d");// 主键范围
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
		msgPack.pack(_T("成功"));

		ReturnLltcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_UPDATE_DATA:
	{
		std::string strOtcmc = (pObj++)->as<std::string>();
		std::string strNtcmc = (pObj++)->as<std::string>();
		std::string strTcfl = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("update lltc_tbl set tcmc = '%s',tcfl= '%s' where tcmc = '%s'");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strNtcmc.c_str(), strTcfl.c_str(), strOtcmc.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}