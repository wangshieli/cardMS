#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_sim.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnSimInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(18);
		_variant_t varId = pRecord->GetCollect("id");
		AddData(varId, msgPack);

		_variant_t varSim = pRecord->GetCollect("jrhm");
		AddData(varSim, msgPack);

		_variant_t varIccid = pRecord->GetCollect("iccid");
		AddData(varIccid, msgPack);

		_variant_t varKh = pRecord->GetCollect("kh");
		AddData(varKh, msgPack);

		_variant_t varKhjl = pRecord->GetCollect("khjl");
		AddData(varKhjl, msgPack);

		_variant_t varDxzh = pRecord->GetCollect("dxzh");
		AddData(varDxzh, msgPack);

		_variant_t varLlc= pRecord->GetCollect("llc");
		AddData(varLlc, msgPack);

		_variant_t varLlclx = pRecord->GetCollect("llclx");
		AddData(varLlclx, msgPack);

		_variant_t varLltc = pRecord->GetCollect("lltc");
		AddData(varLltc, msgPack);

		_variant_t varSsdq = pRecord->GetCollect("ssdq");
		AddData(varSsdq, msgPack);

		_variant_t varZt = pRecord->GetCollect("zt");
		AddData(varZt, msgPack);

		_variant_t varJhrq= pRecord->GetCollect("jhrq");
		AddYYYYMMDD(varJhrq, msgPack);

		_variant_t varXsrq = pRecord->GetCollect("xsrq");
		AddYYYYMMDD(varXsrq, msgPack);

		_variant_t varDqrq = pRecord->GetCollect("dqrq");
		AddYYYYMMDD(varDqrq, msgPack);

		_variant_t varXfrq = pRecord->GetCollect("xfrq");
		AddYYYYMMDD(varXfrq, msgPack);

		_variant_t varZxrq = pRecord->GetCollect("zxrq");
		AddYYYYMMDD(varZxrq, msgPack);

		_variant_t varDj = pRecord->GetCollect("dj");
		AddData(varDj, msgPack);

		_variant_t varBz = pRecord->GetCollect("bz");
		AddData(varBz, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	ReleaseRecordset(pRecord);
}

bool DoTrans_SimAdd(msgpack::object* pObj);

bool doParseSim(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_SIM;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SUBCMD_SIM_ADD:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		if (!DoTrans_SimAdd(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SIM_GET_01:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		std::string strJrhm = (pObj++)->as<std::string>();

		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select a.*,b.llclx from sim_tbl as a, llc_tbl as b where a.jrhm='%s' and b.llchm=a.llchm");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strJrhm.c_str());
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

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SIM_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nNeed = (pObj++)->as<int>();
		int nStart = nNeed * (nTag - 1);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select a.*,b.llclx from (select * from sim_tbl limit %d,%d) a left join llc_tbl b on b.llchm=a.llchm");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nNeed);
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, nTag, msgPack))
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

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SIM_GET_ICCID:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		std::string strIccid = (pObj++)->as<std::string>();

		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select a.*,b.llclx from sim_tbl as a, llc_tbl as b where a.iccid='%s' and b.llchm=a.llchm");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strIccid.c_str());
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

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_SIM_GET_XSRQ:
	{}
	break;

	case SUBCMD_SIM_GET_TOTAL:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		int nTag = (pObj++)->as<int>();
		int nNeed = (pObj++)->as<int>();
		int nStart = nNeed * (nTag - 1);

		const TCHAR* pSql = _T("SELECT * from sim_tbl");
		_RecordsetPtr pRecord;
		if (!GetRecordSetDate(pSql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		int lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(5);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(nTag);
		msgPack.pack(0);
		msgPack.pack(lRstCount);// 数据条数

		if (lRstCount - nStart >= nNeed)
			msgPack.pack_array(nNeed);
		else
			msgPack.pack_array(lRstCount - nStart);

		_variant_t varNum = pRecord->GetCollect("num");
		AddData(varNum, msgPack);

		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}
	return true;
}

bool DoTrans_SimAdd(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	int nIgnore = 0;
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();

		std::string strJrhm = (pObj++)->as<std::string>();
		std::string strIccid = (pObj++)->as<std::string>();
		std::string strDxzh = (pObj++)->as<std::string>();
		std::string strLlc = (pObj++)->as<std::string>();

		pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llc,tag) value(null,'%s','%s','%s','%s',true) ON DUPLICATE KEY UPDATE iccid='%s'");

		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(), strDxzh.c_str(), strLlc.c_str(), strIccid.c_str());
		(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		if ((int)EffectedRecCount == 0 || (int)EffectedRecCount == 2)// 说明此条数据被忽略
			++nIgnore;

		if (nIgnore < 1)
		{
			pSql = _T("update llc_tbl set zsl=zsl+1,kysl=kysl+1,xgsj=now() where llchm='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strLlc.c_str());
			(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		}

		(*conptr)->CommitTrans();
	}
	catch (_com_error e)
	{
		_tprintf(_T("%s-%d:错误信息:%s 错误代码:%08lx 错误源:%s 错误描述:%s\n"), __FILE__, __LINE__, e.ErrorMessage(), e.Error(), (const TCHAR*)e.Source(), (const TCHAR*)e.Description());

		(*conptr)->RollbackTrans();
		PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
		return false;
	}

	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return true;
}