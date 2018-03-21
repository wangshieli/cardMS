#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "parse_lltc.h"
#include "db_operation.h"
#include "parse_data.h"

void ReturnLltcInfo(_RecordsetPtr& pRecord, msgpack::packer<msgpack::sbuffer>& msgPack)
{
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		msgPack.pack_array(2);
		_variant_t varTclx = pRecord->GetCollect("tcfl");
		AddData(varTclx, msgPack);

		_variant_t varTcmc = pRecord->GetCollect("tcmc");
		AddData(varTcmc, msgPack);

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}
}

bool doParseLltc(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::object* pObj = result_.get().via.array.ptr;
	pObj++;
	int nSubCmd = (pObj++)->as<int>();
	_tprintf(_T("nSubCmd %d\n"), nSubCmd);

	switch (nSubCmd)
	{
	case 1:
	{
		std::string strTcfl = (pObj++)->as<std::string>();
		std::string strTcmc = (pObj++)->as<std::string>();

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x6;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);

		const TCHAR* pSql = _T("insert into lltc_tbl (id,tcfl,tcmc) value(null,'%s','%s')");

		TCHAR strInsert[512];
		memset(strInsert, 0x00, sizeof(strInsert));
		_stprintf_s(strInsert, 512, pSql, strTcfl.c_str(), strTcmc.c_str());
		if (!ExcuteSql(strInsert, true))
		{
			msgPack.pack(0);
			_tprintf(_T("����ʧ��\n"));
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
		std::string strTcmc = (pObj++)->as<std::string>();
		_tprintf(_T("p = %s\n"), strTcmc.c_str());
		const TCHAR* pSql = _T("select * from lltc_tbl where tcmc = '%s'");
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, strTcmc.c_str());
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("û���ҵ�����"));
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

		ReturnLltcInfo(pRecord, msgPack);

		DealLast(sbuf, bobj);
	}
	break;

	case 3:
	{
		int nTag = (pObj++)->as<int>();
		_tprintf(_T("ntag = %d\n"), nTag);

		int nStart = 200 * (nTag - 1) + 1;
		int nEnd = 200 * nTag;

		const TCHAR* pSql = _T("select * from lltc_tbl where id  between %d and %d");// ������Χ
		TCHAR sql[256];
		memset(sql, 0x00, 256);
		_stprintf_s(sql, 256, pSql, nStart, nEnd);
		_RecordsetPtr pRecord;
		if (!GetRecordSet(sql, pRecord, adCmdText, true))
			return false;
		if (pRecord->adoEOF)
		{
			_tprintf(_T("û���ҵ�����"));
		}

		long lRstCount = pRecord->GetRecordCount();
		_tprintf(_T("��ѯ������������:%ld\n"), lRstCount);

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		int nCmd = 0xb;
		int nSubCmd = 0x8;
		sbuf.write("\xfb\xfc", 6);
		msgPack.pack_array(3 + lRstCount);
		msgPack.pack(nCmd);
		msgPack.pack(nSubCmd);
		msgPack.pack(1);

		ReturnLltcInfo(pRecord, msgPack);
		DealLast(sbuf, bobj);
	}
	break;
	default:
		break;
	}

	return true;
}