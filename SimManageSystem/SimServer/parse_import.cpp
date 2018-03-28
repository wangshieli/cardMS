#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_import.h"
#include "db_operation.h"
#include "parse_data.h"

// 新卡清单处理
bool DoTrans_NewCard_LeadIn(msgpack::object* pObj);

// 销售清单
bool DoTrans_XSCard_LeadIn(msgpack::object* pObj);

// 用户状态清单
bool DoTrans_UseCard_LeadIn(msgpack::object* pObj);

// 续费清单
bool DoTrans_RenewData_LeadIn(msgpack::object* pObj);

// 退卡清单
bool DoTrans_ReturnCard_LeadIn(msgpack::object* pObj);

// 注销清单
bool DoTrans_CancelData_LeadIn(msgpack::object* pObj);

bool doParseImport(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	int nCmd = CMD_IMPORT;
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();

	switch (nSubCmd)
	{
	case SUBCMD_IMPORT_NEWCARD:
	{
		_tprintf(_T("%d\n"), bobj->dwRecvedCount);
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_NewCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_IMPORT_SALENOTE:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_XSCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_IMPORT_KHSTATE:
	{
		_tprintf(_T("%d\n"), bobj->dwRecvedCount);
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_UseCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_IMPORT_PAYLIST:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_RenewData_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_IMPORT_CARDRETURNED:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_ReturnCard_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_IMPORT_CARDCANCEL:
	{
		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		int nTag = (pObj++)->as<int>();
		if (!DoTrans_CancelData_LeadIn(pObj))
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 1);
		}
		else
		{
			ReturnSimpleInfo(msgPack, nCmd, nSubCmd, nTag, 0);
		}

		DealLast(sbuf, bobj);
	}
	default:
		break;
	}

	return true;
}

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
		std::string strKhmc = (pObj++)->as<std::string>();
		int nCount = pObj->via.array.size;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();
			std::string strXsrq = (pSubObj++)->as<std::string>();
			std::string strXsy = (pSubObj++)->as<std::string>();
			std::string strBz = (pSubObj++)->as<std::string>();

			pSql = _T("update sim_tbl sim, llc_tbl llc set sim.khmc='%s',sim.xsrq='%s',sim.xsy='%s',sim.bz='%s',sim.tag=false,\
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
			_stprintf_s(sql, 256, pSql, strXfrq.c_str(), strXfrq.c_str(), nMonth, strXfrq.c_str(), strJrhm.c_str());
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
		std::string strKhmc = (pObj++)->as<std::string>();
		int nCount = pObj->via.array.size;
		msgpack::object* pArray = (pObj++)->via.array.ptr;
		for (int i = 0; i < nCount; i++)
		{
			msgpack::object* pSubObj = (pObj++)->via.array.ptr;
			std::string strJrhm = (pSubObj++)->as<std::string>();

			//	pSql = _T("update sim_tbl set user=null,ktrq=null,zt=null,dqrq=null,tag=true where jrhm='%s'");
			pSql = _T("update sim_tbl sim,llc_tbl llc set sim.khmc=null,sim.xsrq=null,sim.xsy=null,sim.bz=null,sim.tag=true,\
llc.kysl=llc.kysl+1 where sim.jrhm='%s' and llc.llchm=sim.llc");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, strJrhm.c_str());
			(*conptr)->Execute(_bstr_t(sql), &EffectedRecCount, adCmdText);
		}

		//		pSql = _T("update llc_tbl llc, kh_tbl kh set llc.kysl=llc.kysl+%d,llc.xgsj=now(),kh.ksl=kh.ksl-%d,kh.xgrq=now()\
		//where llc.llchm='%s' and kh.khmc='%s'");
		pSql = _T("update kh_tbl set ksl=ksl-%d,xgrq=now() where khmc='%s'");
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