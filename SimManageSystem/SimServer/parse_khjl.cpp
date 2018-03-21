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
		_variant_t varXm = pRecord->GetCollect("xm");
		AddData(varXm, msgPack);

		_variant_t varLxfs = pRecord->GetCollect("lxfs");
		AddData(varLxfs, msgPack);

		_variant_t varUser = pRecord->GetCollect("user");
		AddData(varUser, msgPack);


		_variant_t varGf = pRecord->GetCollect("gf");
		AddData(varGf, msgPack);

		_variant_t varBz = pRecord->GetCollect("bz");
		AddData(varBz, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}
}

bool doParseKhjl(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case 1:
	{
		std::string strXm = (pObj++)->as<std::string>();
		std::string strLxfs = (pObj++)->as<std::string>();
		std::string strUser = (pObj++)->as<std::string>();
		std::string strGf = (pObj++)->as<std::string>();
		std::string strBz = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x6;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);

		const TCHAR* pSql = _T("insert into khjl_tbl (id,xm,lxfs,user,gf,bz) \
value(null,'%s','%s','%s','%s','%s')");

		TCHAR strInsert[512];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 512, pSql, strXm.c_str(), strLxfs.c_str(), strUser.c_str(), strGf.c_str(), strBz.c_str());
		if (!ExcuteSql(strInsert, true))
		{
			msgPack.pack(0);
			_tprintf(_T("插入失败\n"));
		}
		else
		{
			msgPack.pack(1);
		}

		DealLast(sbuf, bobj);
	}
	break;

	case 2:
	{
		std::string strXm = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strXm.c_str());
		const TCHAR* pSql = _T("select * from khjl_tbl where xm = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strXm.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
		}
		int lRstCount = pRecord->GetRecordCount();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x7;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnKhjlInfo(pRecord, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case 3:
	{
		int nTag = (pObj++)->as<int>();
		_tprintf(_T("ntag = %d\n"), nTag);

		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		const TCHAR* pSql = _T("select * from khjl_tbl where id  between %d and %d");// 主键范围
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nEnd);
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("没有找到数据"));
		}

		long lRstCount = pRecord->GetRecordCount();
		_tprintf(_T("查询到的数据条数:%ld\n"), lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x8;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnKhjlInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}