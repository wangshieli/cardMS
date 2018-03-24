#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_llc.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnLlcInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(5);
		_variant_t varLx = pRecord->GetCollect("lx");
		AddData(varLx, msgPack);

		_variant_t varDm = pRecord->GetCollect("dm");
		AddData(varDm, msgPack);

		_variant_t varDxzh = pRecord->GetCollect("dxzh");
		AddData(varDxzh, msgPack);

		_variant_t varXgrq = pRecord->GetCollect("xgrq");
		AddData(varXgrq, msgPack);

		_variant_t varBz = pRecord->GetCollect("bz");
		AddData(varBz, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	pRecord->Close();
	pRecord.Release();
	pRecord = NULL;
}

bool doParseLlc(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_LLC;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SUBCMD_ADD:
	{
		std::string strDm = (pObj++)->as<std::string>();
		std::string strLx = (pObj++)->as<std::string>();
		std::string strDxzh = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("insert into llc_tbl (id,lx,dm,dxzh,bz,xgrq) value(null,'%s','%s','%s','%s',now())");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strLx.c_str(), strDm.c_str(), strDxzh.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_MODIFY:
	{
		std::string strOdm = (pObj++)->as<std::string>();
		std::string strNdm = (pObj++)->as<std::string>();
		std::string strLx = (pObj++)->as<std::string>();
		std::string strDxzh = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("update llc_tbl set dm = '%s', lx = '%s',dxzh= '%s',bz='%s',xgrq=now() where dm = '%s'");

		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strNdm.c_str(), strLx.c_str(), strDxzh.c_str(), strBz.c_str(), strOdm.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SELECT_BY_KEY:
	{
		std::string strDm = (pObj++)->as<std::string>();
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from llc_tbl where dm = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strDm.c_str());
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		int lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(4 + lRstCount);
		msgPack.pack(B_MSG_LLC_0X9B);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);
		msgPack.pack(_T("成功"));

		ReturnLlcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SELECT_BY_TAG:
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from llc_tbl where id  between %d and %d");// 主键范围
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

		ReturnLlcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	default:
		break;
	}

	return true;
}