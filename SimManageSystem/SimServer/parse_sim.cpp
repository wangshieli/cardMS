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
		msgPack.pack_array(15);
		_variant_t varSim = pRecord->GetCollect("jrhm");
		AddData(varSim, msgPack);

		_variant_t varIccid = pRecord->GetCollect("iccid");
		AddData(varIccid, msgPack);

		_variant_t varDxzh = pRecord->GetCollect("dxzh");
		AddData(varDxzh, msgPack);

		_variant_t varJhrq = pRecord->GetCollect("jhrq");
		AddData(varJhrq, msgPack);

		_variant_t varZt = pRecord->GetCollect("zt");
		AddData(varZt, msgPack);

		_variant_t varSsdq = pRecord->GetCollect("ssdq");
		AddData(varSsdq, msgPack);

		_variant_t varLltc = pRecord->GetCollect("lltc");
		AddData(varLltc, msgPack);

		_variant_t varLlc = pRecord->GetCollect("llc");
		AddData(varLlc, msgPack);

		_variant_t varKh = pRecord->GetCollect("kh");
		AddData(varKh, msgPack);

		_variant_t varKhjl = pRecord->GetCollect("khjl");
		AddData(varKhjl, msgPack);

		_variant_t varXsrq = pRecord->GetCollect("xsrq");
		AddData(varXsrq, msgPack);

		_variant_t varDqrq = pRecord->GetCollect("dqrq");
		AddData(varDqrq, msgPack);

		_variant_t varXfrq = pRecord->GetCollect("xfrq");
		AddData(varXfrq, msgPack);

		_variant_t varZxrq = pRecord->GetCollect("zxrq");
		AddData(varZxrq, msgPack);

		_variant_t varDj = pRecord->GetCollect("dj");
		AddData(varDj, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	ReleaseRecordset(pRecord);
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
		std::string strSim = (pObj++)->as<std::string>();
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

		const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,zt,kh,khjl,llc,dqrq,xsrq,xfrq,jhrq,zxrq,dj,ssdq,lltc) \
value(null,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%f','%s','%s')");
		TCHAR sql[512];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 512, pSql, strSim.c_str(), strIccid.c_str(), strDxzh.c_str(), strZt.c_str(), strKh.c_str(), strKhjl.c_str(), strLlc.c_str(), strDqrq.c_str(), strXsrq.c_str(), strXfrq.c_str(),
			strJhrq.c_str(), strZxrq.c_str(), dDj, strSsdq.c_str(), strLltc.c_str());
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

		const TCHAR* pSql = _T("select * from sim_tbl where jrhm = '%s'");
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
		msgPack.pack(_T("成功"));

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

		const TCHAR* pSql = _T("select * from sim_tbl where id  between %d and %d");// 主键范围
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

		const TCHAR* pSql = _T("update sim_tbl set jrhm='%s',iccid='%s',dxzh='%s',zt='%s',kh='%s',khjl='%s',llc='%s',dqrq='%s',xsrq='%s',\
			xfrq='%s',jhrq='%s',zxrq='%s',dj='%s',ssdq='%s',lltc='%s' where jrhm = '%s'");
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
	default:
		break;
	}
	return true;
}