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

#define SQL_ITEM_COUNT _T("SELECT %s,COUNT(*) num FROM %s GROUP BY %s");// ������ȡ�ͻ�������ϸ�е� ���ۼ�¼ %s=�ֶ���  %s=����  %s=�ֶ���

const TCHAR* pSql_XSJL = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE khjl='%s' GROUP BY xsrq");// ������ȡ�ͻ�������ϸ�е� ���ۼ�¼

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
		// ���ݿ��쳣
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
		int nSSubCmd = (pObj++)->as<int>();
		std::string strJlmc = (pObj++)->as<std::string>();

		const TCHAR* pSql = NULL;
		TCHAR sql[256];

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);

		switch (nSSubCmd)
		{
		case SSUBCMD_KHJL_TOTAL:
		{
			pSql = _T("SELECT lxfs FROM khjl_tbl WHERE jlmc='%s'");// num1����״̬������ num2�ͻ��������¿�����
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlmc.c_str());
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
			{
				DealLast(sbuf, bobj);
				return false;
			}

			pSql = _T("SELECT count(*) num1,num2 FROM (SELECT zt,count(*) num2 FROM sim_tbl WHERE khjl='%s') t WHERE t.zt='����'");// num1����״̬������ num2�ͻ��������¿�����
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlmc.c_str());
			_RecordsetPtr pRecord01;
			if (!GetRecordSetDate(sql, pRecord01, nCmd, nSubCmd, msgPack))
			{
				DealLast(sbuf, bobj);
				return false;
			}

			int lRstCount = pRecord01->GetRecordCount();
			msgPack.pack_array(5);
			msgPack.pack(nCmd);
			msgPack.pack(nSubCmd);
			msgPack.pack(nSSubCmd);
			msgPack.pack(0);
			msgPack.pack_array(lRstCount);

			msgPack.pack_array(3);
			_variant_t varLxfs = pRecord->GetCollect("lxfs");
			AddData(varLxfs, msgPack);
			_variant_t varZksl = pRecord01->GetCollect("num2");
			AddData(varZksl, msgPack);
			_variant_t varKysl = pRecord01->GetCollect("num1");
			AddData(varKysl, msgPack);

			ReleaseRecordset(pRecord);
			ReleaseRecordset(pRecord01);
			DealLast(sbuf, bobj);
		}
		break;

		case SSUBCMD_KHJL_KHINFO:
		{
			int nTag = (pObj++)->as<int>();
			int nStart = (nTag - 1) * 200;
			/*pSql = _T("SELECT COUNT(*) num FROM kh_tbl WHERE khjl='%s'");
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlmc.c_str());
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
			{
				DealLast(sbuf, bobj);
				return false;
			}*/

			pSql = _T("SELECT id,khmc,lxfs,khjl,xgsj,bz FROM kh_tbl WHERE khjl='%s' LIMIT %d,200");
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlmc.c_str(), nStart);
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
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
			msgPack.pack(lRstCount);// ��������

			if (lRstCount - nStart >= 200)
				msgPack.pack_array(200);
			else
				msgPack.pack_array(lRstCount - nStart);

			// khmc, lxfs, khjl, gf, khjl1, xgsj, bz
			pRecord->Move(nStart);
			int nRecords = 200;
			VARIANT_BOOL bRt = pRecord->GetadoEOF(); // �ͻ���Ϣ
			while (!bRt)
			{
				msgPack.pack_array(6);

				_variant_t varId = pRecord->GetCollect("id");
				AddData(varId, msgPack);

				_variant_t varKhmc = pRecord->GetCollect("khmc");
				AddData(varKhmc, msgPack);

				_variant_t varLxfs = pRecord->GetCollect("lxfs");
				AddData(varLxfs, msgPack);

				_variant_t varKhjl = pRecord->GetCollect("khjl");
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
			int nStart = (nTag - 1) * 200;
			pSql = _T("SELECT xsrq,COUNT(*) num FROM sim_tbl WHERE khjl='%s' GROUP BY xsrq");// ������ȡ�ͻ�������ϸ�е� ���ۼ�¼
			memset(sql, 0x00, 256);
			_stprintf_s(sql, 256, pSql, strJlmc.c_str());
			_RecordsetPtr pRecord;
			if (!GetRecordSetDate(sql, pRecord, nCmd, nSubCmd, msgPack))
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
			msgPack.pack(lRstCount);// ��������

			if (lRstCount - nStart >= 200)
				msgPack.pack_array(200);
			else
				msgPack.pack_array(lRstCount - nStart);

			pRecord->Move(nStart);
			int nRecords = 200;
			VARIANT_BOOL bRt = pRecord->GetadoEOF(); // ���ۼ�¼
			while (!bRt && nRecords--)
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
		int nStart = 200 * (nTag - 1);
		//int nEnd = 200 * nTag;

		msgpack::sbuffer sbuf;
		msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
		sbuf.write("\xfb\xfc", 6);
		_RecordsetPtr pRecord;

		const TCHAR* pSql = _T("select * from khjl_tbl limit %d,200");// ������Χ
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