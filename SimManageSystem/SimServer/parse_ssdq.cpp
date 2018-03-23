#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_ssdq.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnSsdqInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(1);
		_variant_t varTclx = pRecord->GetCollect("ssdq");
		AddData(varTclx, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	pRecord->Close();
	pRecord.Release();
	pRecord = NULL;
}

bool doParseSsdq(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	int nCmd = B_MSG_SSDQ_0X7B;

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:
	{
		std::string strSsdq = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("insert into ssdq_tbl (id,ssdq) value(null,'%s')");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strSsdq.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:
	{
		std::string strSsdq = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from ssdq_tbl where ssdq = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strSsdq.c_str());
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

		ReturnSsdqInfo(pRecord, msgPack);
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

		const TCHAR* pSql = _T("select * from ssdq_tbl where id  between %d and %d");// 主键范围
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

		ReturnSsdqInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_UPDATE_DATA:
	{
		std::string strOssdq= (pObj++)->as<std::string>();
		std::string strNssdq = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("update ssdq_tbl set ssdq = '%s' where ssdq = '%s'");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strNssdq.c_str(), strOssdq.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}