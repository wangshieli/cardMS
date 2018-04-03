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

		_variant_t varKhjl = pRecord->GetCollect("jlxm");
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
		msgpack::object* pArray = (pObj++)->via.array.ptr;

		msgpack::object* pSubObj = (pArray++)->via.array.ptr;
		std::string strKhmc = (pSubObj++)->as<std::string>();
		std::string strLxfs = (pSubObj++)->as<std::string>();
		std::string strJlxm = (pSubObj++)->as<std::string>();
		std::string strBz = (pSubObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("INSERT INTO kh_tbl (id,khmc,lxfs,jlxm,xgsj,bz) VALUES(null,'%s','%s','%s',now(),'%s')");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strKhmc.c_str(), strLxfs.c_str(), strJlxm.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KH_GET_01:
	{
		int nSSubCmd = (pObj++)->as<int>();
		std::string strKhmc = (pObj++)->as<std::string>();

		const TCHAR* pSql = NULL;
		TCHAR sql[256];

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		switch (nSSubCmd)
		{
		case SSUBCMD_KH_TOTAL:
		{
			pSql = _T("SELECT a.*,b.jlxm,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
SUM(CASE WHEN zt='在用' THEN 1 ELSE 0 END) AS 'sim_using',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'use_15d',\
SUM(CASE WHEN dqrq>CURDATE() AND dqrq<DATE_ADD(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'use_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 1 MONTH) THEN 1 ELSE 0 END) AS 'due_1m',\
SUM(CASE WHEN dqrq<CURDATE() AND dqrq>DATE_SUB(CURDATE(),INTERVAL 15 DAY) THEN 1 ELSE 0 END) AS 'due_15d' FROM sim_tbl WHERE khmc='%s') a \
LEFT JOIN kh_tbl b ON b.khmc='%s'");
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strKhmc.c_str());
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
			{
				DealLast(sbuf, bobj);
				return false;
			}

			msgPack.pack_array(5);
			msgPack.pack(nCmd);
			msgPack.pack(nSubCmd);
			msgPack.pack(nSSubCmd);
			msgPack.pack(0);
			msgPack.pack_array(1);

			msgPack.pack_array(8);
			_variant_t varKhjl = pRecord->GetCollect("jlxm");
			AddData(varKhjl, msgPack);
			_variant_t varLxfs = pRecord->GetCollect("lxfs");
			AddData(varLxfs, msgPack);
			_variant_t varZksl = pRecord->GetCollect("sim_total");
			AddData(varZksl, msgPack);
			_variant_t varKysl = pRecord->GetCollect("sim_using");
			AddData(varKysl, msgPack);
			_variant_t varUse1M = pRecord->GetCollect("use_1m");
			AddData(varUse1M, msgPack);
			_variant_t varUse15D = pRecord->GetCollect("use_15d");
			AddData(varUse15D, msgPack);
			_variant_t varDue1M = pRecord->GetCollect("due_1m");
			AddData(varDue1M, msgPack);
			_variant_t varDue15D = pRecord->GetCollect("due_15d");
			AddData(varDue15D, msgPack);

			ReleaseRecordset(pRecord);

			DealLast(sbuf, bobj);
		}
		break;

		case SSUBCMD_KH_XSINFO:
		{
			int nTag = (pObj++)->as<int>();
			int nNeed = (pObj++)->as<int>();
			int nStart = (nTag - 1) * nNeed;

			pSql = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE khmc='%s' GROUP BY xsrq");// 用来获取客户经理明细中的 销售记录
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strKhmc.c_str());
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, nTag, msgPack))
			{
				DealLast(sbuf, bobj);
				return false;
			}

			int lRstCount = pRecord->GetRecordCount();
			msgPack.pack_array(7);
			msgPack.pack(nCmd);
			msgPack.pack(nSubCmd);
			msgPack.pack(nSSubCmd);
			msgPack.pack(0);
			msgPack.pack(nTag);
			msgPack.pack(lRstCount);// 数据条数

			if (lRstCount - nStart >= nNeed)
				msgPack.pack_array(nNeed);
			else
				msgPack.pack_array(lRstCount - nStart);

			pRecord->Move(nStart);
			VARIANT_BOOL bRt = pRecord->GetadoEOF(); // 销售记录
			while (!bRt && nNeed--)
			{
				msgPack.pack_array(2);

				_variant_t varXsrq = pRecord->GetCollect("xsrq");
				AddYYYYMMDD(varXsrq, msgPack);

				_variant_t varNum = pRecord->GetCollect("num");
				AddData(varNum, msgPack);

				pRecord->MoveNext();
				bRt = pRecord->GetadoEOF();
			}

			ReleaseRecordset(pRecord);
			DealLast(sbuf, bobj);
		}
		break;

		case SSUBCMD_KH_KHTOTAL:
		{}
		break;

		default:
			break;
		}
	}
	break;

	case SUBCMD_KH_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nNeed = (pObj++)->as<int>();
		int nStart = nNeed * (nTag - 1);
		//int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from kh_tbl limit %d,%d");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nNeed);
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, nTag, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		long lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(6);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(nTag);
		msgPack.pack(0);
		msgPack.pack(lRstCount);// 数据条数

		if (lRstCount - nStart >= nNeed)
			msgPack.pack_array(nNeed);
		else
			msgPack.pack_array(lRstCount - nStart);

		pRecord->Move(nStart);

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