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
		msgPack.pack_array(17);
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

		_variant_t varLlclx = pRecord->GetCollect("lx");
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

bool DoTrans(int nCount, ...)
{
	va_list ap;
	va_start(ap, nCount);
	_ConnectionPtr* conptr = GetTransConnection();
	try
	{
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		for (int i = 0; i < nCount; i++)
		{
			(*conptr)->Execute(_bstr_t(va_arg(ap, const TCHAR*)), &EffectedRecCount, adCmdText);
		}
		(*conptr)->CommitTrans();
	}
	catch (_com_error e)
	{
		(*conptr)->RollbackTrans();
		PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
		return false;
	}
	catch (...)
	{
		PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
		return false;
	}
	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return true;
}

bool doParseSim(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	int nCmd = B_MSG_SIM_0XBB;

	switch (nSubCmd)
	{
	case DO_INSERT_DATA:
	{
		std::string strJrhm = (pObj++)->as<std::string>();
		std::string strIccid = (pObj++)->as<std::string>();
		std::string strDxzh = (pObj++)->as<std::string>();
		std::string strLlc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		//const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llc) value(null,'%s','%s','%s','%s')");
		//TCHAR sql[256];
		//TCHAR sql_u[256];
		//memset(sql, 0x00, sizeof(sql));
		//memset(sql_u, 0x00, sizeof(sql_u));
		//_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(), strDxzh.c_str(), strLlc.c_str());
		//_stprintf_s(sql_u, 256, _T("update llc_tbl set ksl=ksl+1 where dm='%s'"), strLlc.c_str());
		//if (!DoTrans(2, sql, sql_u))
		//{
		//	// 失败
		//}
		//else
		//{
		//	// 成功
		//}

		const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llc) value(null,'%s','%s','%s','%s')");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(),strDxzh.c_str(),strLlc.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_KEY:// 使用接入号码查sim
	{
		std::string strSim = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select a.*,b.lx from sim_tbl as a, llc_tbl as b where a.jrhm='%s' and a.llc=b.dm");
		//const TCHAR* pSql = _T("select * from sim_tbl where jrhm = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strSim.c_str());
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
		msgPack.pack(_T("success"));

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_ID:// 根据tag返回一定数量的卡信息
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select a.*,b.lx from sim_tbl as a, llc_tbl as b where a.id between %d and %d and a.llc=b.dm");
		//const TCHAR* pSql = _T("select * from sim_tbl where id  between %d and %d");// 主键范围
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
		msgPack.pack(_T("success"));

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_UPDATE_DATA:
	{
		std::string strOsim = (pObj++)->as<std::string>();
		std::string strNsim = (pObj++)->as<std::string>();
		std::string strIccid = (pObj++)->as<std::string>();
		std::string strDxzh = (pObj++)->as<std::string>();
		std::string strJhrq = (pObj++)->as<std::string>();
		std::string strZt = (pObj++)->as<std::string>();
		std::string strSsdq = (pObj++)->as<std::string>();
		std::string strLltc = (pObj++)->as<std::string>();
		std::string strLlc = (pObj++)->as<std::string>();
		std::string strKh = (pObj++)->as<std::string>();
		std::string strKhjl = (pObj++)->as<std::string>();
		std::string strXsrq = (pObj++)->as<std::string>();
		std::string strDqrq = (pObj++)->as<std::string>();
		std::string strXfrq = (pObj++)->as<std::string>();
		std::string strZxrq = (pObj++)->as<std::string>();
		double dDj = (pObj++)->as<double>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		const TCHAR* pSql = _T("update sim_tbl set zt='%s',kh='%s',khjl='%s',ssdq='%s',lltc='%s',dj='%s',jhrq='%s',dqrq='%s' where jrhm = '%s'");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strNsim.c_str(), strIccid.c_str(), strDxzh.c_str(), strZt.c_str(), strKh.c_str(), strKhjl.c_str(), strLlc.c_str(), strDqrq.c_str(), strXsrq.c_str(), strXfrq.c_str(),
			strJhrq.c_str(), strZxrq.c_str(), dDj, strSsdq.c_str(), strLltc.c_str(), strOsim.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case DO_SELECT_BY_USER:
	{
		int nTag = (pObj++)->as<int>();
		std::string strUser = (pObj++)->as<std::string>();
		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from sim_tbl where user = '%s' and id between %d and %d");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
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

		ReturnSimInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case DO_DEVICE_STOP:
	{
		
	}
	break;

	case DO_DEVICE_START:
	{
		
	}
	break;

	case DO_DEVICE_UPDATE:
	{
		
	}
	break;
	default:
		break;
	}
	return true;
}