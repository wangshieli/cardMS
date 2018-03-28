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
		msgPack.pack_array(6);
		_variant_t varId = pRecord->GetCollect("id");
		AddData(varId, msgPack);

		_variant_t varKhmc = pRecord->GetCollect("khmc");
		AddData(varKhmc, msgPack);

		_variant_t varLxfs = pRecord->GetCollect("lxfs");
		AddData(varLxfs, msgPack);

		_variant_t varKhjl = pRecord->GetCollect("khjl");
		AddData(varKhjl, msgPack);

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
	int nCmd = CMD_KH;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SUBCMD_KH_ADD:
	{
		if (pObj->type == msgpack::type::ARRAY)
		{
			_tprintf(_T("test"));
		}
		msgpack::object* pArray = (pObj++)->via.array.ptr;

		msgpack::object* pSubObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pSubObj++)->as<std::string>();
		std::string strLxfs = (pSubObj++)->as<std::string>();
		std::string strKhjl = (pSubObj++)->as<std::string>();
		std::string strBz = (pSubObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("insert into kh_tbl (id,khmc,lxfs,khjl,xgrq,bz) value(null,'%s','%s','%s',CURDATE(),'%s')");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strKhmc.c_str(), strLxfs.c_str(), strKhjl.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KH_GET_01:
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
		msgPack.pack_array(4);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(0);
		msgPack.pack_array(lRstCount);

		ReturnKhInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KH_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1);
		//int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from kh_tbl limit %d,200");// Ö÷¼ü·¶Î§
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
		msgPack.pack(0);
		msgPack.pack(nTag);
		msgPack.pack_array(lRstCount);

		ReturnKhInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KH_MODIFY:
	{
		int nId = (pObj++)->as<int>();
		std::string strKhmc = (pObj++)->as<std::string>();
		std::string strLxfs = (pObj++)->as<std::string>();
		std::string strKhjl = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("update kh_tbl set khmc='%s',lxfs='%s',khjl='%s',bz='%s',xgrq=CURDATE() where id=%d");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strKhmc.c_str(), strLxfs.c_str(), strKhjl.c_str(), strBz.c_str(), nId);
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	default:
		break;
	}

	return true;
}