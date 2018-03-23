#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_kh.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnKhInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(8);
		_variant_t varKhmc = pRecord->GetCollect("khmc");
		AddData(varKhmc, msgPack);

		_variant_t varLxfs = pRecord->GetCollect("lxfs");
		AddData(varLxfs, msgPack);

		_variant_t varKhjl = pRecord->GetCollect("khjl");
		AddData(varKhjl, msgPack);

		_variant_t varUser = pRecord->GetCollect("user");
		AddData(varUser, msgPack);

		_variant_t varKhdm = pRecord->GetCollect("khdm");
		AddData(varKhdm, msgPack);

		_variant_t varGf = pRecord->GetCollect("gf");
		AddData(varGf, msgPack);

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

bool doParseKh(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	int nCmd = B_MSG_KH_0XCB;

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:
	{
		std::string strKhmc = (pObj++)->as<std::string>();
		std::string strLxfs = (pObj++)->as<std::string>();
		std::string strKhjl = (pObj++)->as<std::string>();
		std::string strUser = (pObj++)->as<std::string>();
		std::string strKhdm = (pObj++)->as<std::string>();
		std::string strGf = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("insert into kh_tbl (id,khmc,lxfs,khjl,user,khdm,gf,bz,xgrq) value(null,'%s','%s','%s','%s','%s','%s','%s',now())");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strKhmc.c_str(), strLxfs.c_str(), strKhjl.c_str(), strUser.c_str(), strKhdm.c_str(), strGf.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:
	{
		std::string strKhmc = (pObj++)->as<std::string>();
		
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from kh_tbl where khmc = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strKhmc.c_str());
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

		ReturnKhInfo(pRecord, msgPack);
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

		const TCHAR* pSql = _T("select * from kh_tbl where id  between %d and %d");// 主键范围
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

		ReturnKhInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_UPDATE_DATA:
	{
		std::string strOkhmc = (pObj++)->as<std::string>();
		std::string strNkhmc = (pObj++)->as<std::string>();
		std::string strLxfs = (pObj++)->as<std::string>();
		std::string strKhjl = (pObj++)->as<std::string>();
		std::string strUser = (pObj++)->as<std::string>();
		std::string strKhdm = (pObj++)->as<std::string>();
		std::string strGf = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		
		const TCHAR* pSql =  _T("update kh_tbl set khmc = '%s', lxfs= '%s',khjl= '%s',user= '%s',khdm= '%s',gf= '%s',bz='%s',xgrq=now() where khmc = '%s'");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strNkhmc.c_str(), strLxfs.c_str(), strKhjl.c_str(), strUser.c_str(), strKhdm.c_str(), strGf.c_str(), strBz.c_str(), strOkhmc.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}