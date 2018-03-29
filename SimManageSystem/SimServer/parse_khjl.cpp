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

		_variant_t varXm = pRecord->GetCollect("jlmc");
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

		const TCHAR* pSql = _T("insert into khjl_tbl (id,jlmc,lxfs,bz,xgsj) value(null,'%s','%s','%s',now())");
		TCHAR sql[256];
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, strJlmc.c_str(), strLxfs.c_str(), strBz.c_str());
		CheckSqlResult(sql, nCmd, nSubCmd, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KHJL_GET_01:
	{
		std::string strJlmc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from khjl_tbl where jlmc = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strJlmc.c_str());
		if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
		{
			DealLast(sbuf, bobj);
			return false;
		}

		//pSql = _T("SELECT count(*) num1,num2 FROM (SELECT zt,count(*) num2 FROM sim_tbl WHERE khjl='%s') WHERE zt='在用'");// num1在用状态数量， num2客户经理名下卡数量

		//_RecordsetPtr pRecord1;
		//pSql = _T("SELECT xsrq,count(*) num FROM sim_tbl WHERE khjl='%s' GROUP BY xsrq");
		//memset(sql, 0x00, 256);
		//_stprintf_s(sql, 256, pSql, strJlmc.c_str());
		//if (!GetRecordSetDate(sql, pRecord1, nCmd, nSubCmd, msgPack))
		//{
		//	DealLast(sbuf, bobj);
		//	return false;
		//}

		int lRstCount = pRecord->GetRecordCount();
		msgPack.pack_array(4);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(0);
		msgPack.pack_array(lRstCount);

		ReturnKhjlInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;

	case SUBCMD_KHJL_GET_02:
	{
		int nTag = (pObj++)->as<int>();
		int nStart = 200 * (nTag - 1);
		//int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from khjl_tbl limit %d,200");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart);
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