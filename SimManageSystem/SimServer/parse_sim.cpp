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

// 新卡清单处理
bool DoTrans_NewCard_LeadIn(msgpack::object* pObj)
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
		std::string llchm = (pObj++)->as<std::string>();
		std::string llclx = (pObj++)->as<std::string>();
		int nCount = (pObj)->via.array.size;
		_tprintf(_T("%d\n"), nCount);
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pArray++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strIccid = (pSubObj++)->as<std::string>();
			std::string strDxzh = (pSubObj++)->as<std::string>();

			pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llc,tag) value(null,'%s','%s','%s','%s',true) ON DUPLICATE KEY UPDATE iccid='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(), strDxzh.c_str(), llchm.c_str(), strIccid.c_str());
			(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
			if ((int)EffectedRecCount == 0 || (int)EffectedRecCount == 2)// 说明此条数据被忽略
				++nIgnore;
		}
		if (nIgnore < nCount)
		{
			pSql = _T("update llc_tbl set zsl=zsl+%d,kysl=kysl+%d,xgsj=now() where llchm='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, nCount - nIgnore, nCount - nIgnore, llchm.c_str());
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

// 用户状态清单处理
bool DoTrans_UseCard_LeadIn(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		int nCount = pObj->via.array.size;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pArray++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strKtrq = (pSubObj++)->as<std::string>();
			std::string strZt = (pSubObj++)->as<std::string>();

			pSql = _T("update sim_tbl set ktrq='%s',zt='%s'where jrhm='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKtrq.c_str(), strZt.c_str(), strJrhm.c_str());
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

// 注销清单处理
bool DoTrans_CancelData_LeadIn(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		int nCount = pObj->via.array.size;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strZxrq = (pSubObj++)->as<std::string>();
			std::string strZt = (pSubObj++)->as<std::string>();

			pSql = _T("update sim_tbl set zt='%s',zxrq='%s' where jrhm='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strZt.c_str(), strZxrq.c_str(), strJrhm.c_str());
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

// 续费清单处理
bool DoTrans_RenewData_LeadIn(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		int nCount = pObj->via.array.size;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strXfrq = (pSubObj++)->as<std::string>();
			int nMonth = (pSubObj++)->as<int>();

			pSql = _T("update sim_tbl set dqrq=DATE_ADD(IF('%s'>dqrq,'%s',dqrq),INTERVAL %d MONTH),xfrq='%s' where jrhm='%s'");// 续费日期 到期日期
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strXfrq.c_str(),strXfrq.c_str(), nMonth, strXfrq.c_str(), strJrhm.c_str());
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

// 退卡卡清单处理
bool DoTrans_ReturnCard_LeadIn(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		int nCount = (pObj++)->as<int>();// 数据包中的元素个数
		std::string llchm = (pObj++)->as<std::string>();
		std::string strKhmc = (pObj++)->as<std::string>();
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();

			pSql = _T("update sim_tbl set user=null,ktrq=null,zt=null,dqrq=null,tag=true where jrhm='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJrhm.c_str());
			(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		}

		pSql = _T("update llc_tbl llc, kh_tbl kh set llc.kysl=llc.kysl+%d,llc.xgsj=now(),kh.ksl=kh.ksl-%d,kh.xgrq=now()\
where llc.llchm='%s' and kh.khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, nCount, nCount, llchm.c_str(), strKhmc.c_str());
		(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);

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

bool DoTrans_XSCard_LeadIn(msgpack::object* pObj)
{
	_ConnectionPtr* conptr = NULL;
	const TCHAR* pSql = NULL;
	TCHAR sql[256];
	try
	{
		conptr = GetTransConnection();
		_variant_t EffectedRecCount;
		(*conptr)->BeginTrans();
		int nCount = (pObj++)->as<int>();// 数据包中的元素个数
		std::string strKhmc = (pObj++)->as<std::string>();
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strXsrq = (pSubObj++)->as<std::string>();
			std::string strXsy= (pSubObj++)->as<std::string>();
			std::string strBz = (pSubObj++)->as<std::string>();

			pSql = _T("update sim_tbl sim, llc_tbl llc set sim.khmc='%s',sim.xsrq='%s',sim.xsy='%s',sim.bz='%s',\
llc.kysl=llc.kysl-1 where sim.jrhm='%s' and llc.llchm=sim.llc");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strKhmc.c_str(), strXsrq.c_str(), strXsy.c_str(), strBz.c_str(), strJrhm.c_str());
			(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		}

		pSql = _T("update kh_tbl set ksl=ksl+%d,xgrq=now() where khmc='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, nCount, strKhmc.c_str());
		(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		
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

bool doParseSim(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_SIM;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SIM_NEWCARD_LEAD_IN: // 导入新卡
	{
		_tprintf(_T("%d\n"), bobj->dwRecvedCount);
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_NewCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SIM_USECARD_LEAD_IN: // 导入用户状态
	{
		_tprintf(_T("%d\n"), bobj->dwRecvedCount);
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_UseCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SIM_CANCELDATA_LEAD_IN: // 导入注销
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_CancelData_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SIM_RENEWDATE_LEAD_IN: // 导入续费
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_RenewData_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SIM_RETURNCARD_LEAD_IN: // 导入退卡
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_ReturnCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SIM_XSINFO_LEAD_IN:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_XSCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 0, _T("失败"));
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, 1, _T("success"));
		}

		DealLast(sbuf, bobj);
	}

	case SUBCMD_ADD:// 新卡直接分配给用户
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

		const TCHAR* pSql = _T("insert into sim_tbl (id,jrhm,iccid,dxzh,llc,tag) value(null,'%s','%s','%s','%s',false) \
ON DUPLICATE KEY UPDATE tag=false");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJrhm.c_str(), strIccid.c_str(),strDxzh.c_str(),strLlc.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_MODIFY:
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

	case SUBCMD_SELECT_BY_KEY:// 使用接入号码查sim
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

	case SUBCMD_SELECT_BY_TAG:// 根据tag返回一定数量的卡信息
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

	case SIM_SELECT_BY_ICCIC:
	{}
	break;

	case SIM_SELECT_BY_KH:
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

	case SIM_STOP:
	{
		
	}
	break;

	case SIM_START:
	{
		
	}
	break;

	case SIM_STATE_UPDATE:
	{
		
	}
	break;
	default:
		break;
	}
	return true;
}