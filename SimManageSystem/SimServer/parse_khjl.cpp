#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_khjl.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnKhjlInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(5);
		_variant_t varId = pRecord->GetCollect("id");
		AddData(varId, msgPack);

		_variant_t varXm = pRecord->GetCollect("jlxm");
		AddData(varXm, msgPack);

		_variant_t varLxfs = pRecord->GetCollect("lxfs");
		AddData(varLxfs, msgPack);

		_variant_t varXgrq = pRecord->GetCollect("xgsj");
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

#define SQL_ITEM_COUNT _T("SELECT %s,COUNT(*) num FROM %s GROUP BY %s");// 用来获取客户经理明细中的 销售记录 %s=字段名  %s=表名  %s=字段名

const TCHAR* pSql_XSJL = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE khjl='%s' GROUP BY xsrq");// 用来获取客户经理明细中的 销售记录

#define ERR_EMPTY 0X0000001;

int TestFunc(const TCHAR* sql, _RecordsetPtr& pRecord)
{
	int nRet = 0;
	_ConnectionPtr* conptr = NULL;
	try
	{
		pRecord.CreateInstance(__uuidof(Recordset));
		conptr = GetTransConnection();
		pRecord->Open(_bstr_t(sql), _variant_t((IDispatch*)(*conptr)), adOpenDynamic, adLockOptimistic, adCmdText);
		if (pRecord->adoEOF)
			nRet = ERR_EMPTY;
	}
	catch (_com_error e)
	{
		// 数据库异常
	}

	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return nRet;
}

void ReturnListInfo_khjl(_RecordsetPtr& pRecord, _RecordsetPtr& pRecord_kh, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	_variant_t varKzsl = pRecord->GetCollect("num2");
	AddData(varKzsl, msgPack);

	_variant_t varZysl = pRecord->GetCollect("num1");
	AddData(varZysl, msgPack);

	int lRstCount = pRecord_kh->GetRecordCount();
	msgPack.pack_array(lRstCount);
	VARIANT_BOOL bRt = pRecord_kh->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(5);
		_variant_t varId = pRecord_kh->GetCollect("id");
		AddData(varId, msgPack);

		_variant_t varKhmc = pRecord_kh->GetCollect("khmc");
		AddData(varKhmc, msgPack);

		_variant_t varLxfs = pRecord_kh->GetCollect("lxfs");
		AddData(varLxfs, msgPack);

		_variant_t varXgrq = pRecord_kh->GetCollect("xgsj");
		AddData(varXgrq, msgPack);

		_variant_t varBz = pRecord_kh->GetCollect("bz");
		AddData(varBz, msgPack);

		pRecord_kh->MoveNext();
		bRt = pRecord_kh->GetadoEOF();
	}

	ReleaseRecordset(pRecord);
	ReleaseRecordset(pRecord_kh);
}

bool doParseKhjl(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_KHJL;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	
	switch (nSubCmd)
	{
	case SUBCMD_KHJL_ADD:
	{
		std::string strJlmc = (pObj++)->as<std::string>();
		std::string strLxfs = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("INSERT INTO khjl_tbl (id,jlxm,lxfs,xgsj,bz) VALUES(null,'%s','%s',now(),'%s')");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJlmc.c_str(), strLxfs.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KHJL_GET_01:
	{
		int nSSubCmd = (pObj++)->as<int>();
		std::string strJlxm = (pObj++)->as<std::string>();

		const TCHAR* pSql = NULL;
		TCHAR sql[256];

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		switch (nSSubCmd)
		{
		case SSUBCMD_KHJL_TOTAL:
		{
			pSql = _T("SELECT a.*,b.lxfs FROM (SELECT COUNT(*) AS 'sim_total',\
SUM(CASE WHEN zt='在用' THEN 1 ELSE 0 END) AS 'sim_using' FROM sim_tbl WHERE jlxm='%s') a \
FEFT JOIN khjl_tbl b ON b.jlxm='%s'");
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlxm.c_str(), strJlxm.c_str());
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

			msgPack.pack_array(3);
			_variant_t varLxfs = pRecord->GetCollect("lxfs");
			AddData(varLxfs, msgPack);
			_variant_t varZksl = pRecord->GetCollect("sim_total");
			AddData(varZksl, msgPack);
			_variant_t varKysl = pRecord->GetCollect("sim_using");
			AddData(varKysl, msgPack);

			ReleaseRecordset(pRecord);
			DealLast(sbuf, bobj);
		}
		break;

		case SSUBCMD_KHJL_KHINFO:
		{
			int nTag = (pObj++)->as<int>();
			int nNeed = (pObj++)->as<int>();// 每次请求的数量
			int nStart = (nTag - 1) * nNeed;

			pSql = _T("SELECT id,khmc,lxfs,jlxm,xgsj,bz FROM kh_tbl WHERE jlxm='%s'");
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlxm.c_str());
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

			// khmc, lxfs, khjl, gf, khjl1, xgsj, bz
			pRecord->Move(nStart);
			VARIANT_BOOL bRt = pRecord->GetadoEOF(); // 客户信息
			while (!bRt && nNeed--)
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

				_variant_t varXgsj = pRecord->GetCollect("xgsj");
				AddData(varXgsj, msgPack);

				_variant_t varBz = pRecord->GetCollect("bz");
				AddData(varBz, msgPack);

				pRecord->MoveNext();
				bRt = pRecord->GetadoEOF();
			}

			ReleaseRecordset(pRecord);
			DealLast(sbuf, bobj);
		}
		break;

		case SSUBCMD_KHJL_XSINFO:
		{
			int nTag = (pObj++)->as<int>();
			int nNeed = (pObj++)->as<int>();
			int nStart = (nTag - 1) * nNeed;

			pSql = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE jlxm='%s' GROUP BY xsrq");// 用来获取客户经理明细中的 销售记录
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlxm.c_str());
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

		default:
			break;
		}
	}
	break;

	case SUBCMD_KHJL_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nNeed = (pObj++)->as<int>();
		int nStart = nNeed * (nTag - 1);
		//int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("SELECT * FROM khjl_tbl");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart);
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

		ReturnKhjlInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	//case SUBCMD_MODIFY:
	//{
	//	std::string strOxm = (pObj++)->as<std::string>();
	//	std::string strNxm = (pObj++)->as<std::string>();
	//	std::string strLxfs = (pObj++)->as<std::string>();
	//	std::string strUser = (pObj++)->as<std::string>();
	//	std::string strGf = (pObj++)->as<std::string>();
	//	std::string strBz = (pObj++)->as<std::string>();

	//	msgpack::sbuffer sbuf;
	//	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	//	sbuf.write("\xfb\xfc", 6);

	//	const TCHAR* pSql = _T("update khjl_tbl set xm = '%s', lxfs= '%s',user= '%s',gf= '%s',bz='%s',xgrq=now() where xm = '%s'");

	//	TCHAR sql[512];
	//	memset(sql, 0x00, sizeof(sql));
	//	_stprintf_s(sql, 512, pSql, strNxm.c_str(), strLxfs.c_str(), strUser.c_str(), strGf.c_str(), strBz.c_str(), strOxm.c_str());
	//	CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

	//	DealLast(sbuf, bobj);
	//}
	//break;

	default:
		break;
	}

	return true;
}