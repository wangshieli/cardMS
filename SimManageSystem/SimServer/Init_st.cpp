#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "Init_st.h"
#include "db_operation.h"
#include "parse_data.h"

#define DB_NAME _T("cardb")
#define DB_USER _T("baolan123")
#define DB_PWD _T("baolan123")
#define DB_DSN _T("mysql_db_a")

tmp_sim mp_sim;
tmp_kh mp_kh;
tmp_khjl mp_khjl;

void AddDate01(ST_KHJL* pKhjl, _variant_t var)
{
}

int test()
{
	mp_sim.clear();
	mp_kh.clear();
	mp_khjl.clear();

	::CoInitialize(NULL);
	_ConnectionPtr conptr;
	conptr.CreateInstance(__uuidof(Connection));
	TCHAR strConn[256];
	memset(strConn, 0x00, sizeof(strConn));
	_stprintf_s(strConn, 256, _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s"), DB_NAME, DB_DSN, DB_PWD, DB_USER);
	HRESULT hr = conptr->Open(_bstr_t(strConn), "", "", adModeUnknown);

	TCHAR sql[256];
	const TCHAR* pSql = _T("SELECT * FROM khjl_tbl");
	_RecordsetPtr pRecord;
	pRecord.CreateInstance(__uuidof(Recordset));
	if (pRecord->State == adStateOpen) pRecord->Close();
	pRecord->Open(_bstr_t(pSql), (IDispatch*)conptr, adOpenDynamic, adLockOptimistic, adCmdText);
	VARIANT_BOOL bRt = pRecord->GetadoEOF();
	while (!bRt)
	{
		ST_KHJL* pKhjl = new ST_KHJL;
		_variant_t varId = pRecord->GetCollect("id");
		pKhjl->id = (int)varId.dblVal;

		_variant_t varJlxm = pRecord->GetCollect("jlxm");
		pKhjl->strJlxm = (const TCHAR*)(_bstr_t)varJlxm;

		_variant_t varLxfs = pRecord->GetCollect("lxfs");
		pKhjl->strLxfs = (const TCHAR*)(_bstr_t)varLxfs;

		_variant_t varXgsj = pRecord->GetCollect("xgsj");
		SYSTEMTIME st;
		VariantTimeToSystemTime(varXgsj.date, &st);
		TCHAR date[32];
		memset(date, 0x00, sizeof(date));
		_stprintf_s(date, 32, _T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		pKhjl->strXgsj = date;

		pSql = _T("SELECT * FROM kh_tbl WHERE jlxm='%s'");
		memset(sql, 0x00, sizeof(sql));
		_stprintf_s(sql, 256, pSql, pKhjl->strJlxm.c_str());
		_RecordsetPtr pRecordKh;
		pRecordKh.CreateInstance(__uuidof(Recordset));
		pRecordKh->Open(_bstr_t(sql), (IDispatch*)conptr, adOpenDynamic, adLockOptimistic, adCmdText);
		VARIANT_BOOL bRtKh = pRecordKh->GetadoEOF();
		while (!bRtKh)
		{
			ST_KH* pKh = new ST_KH;
			_variant_t varId = pRecordKh->GetCollect("id");
			pKh->id = (int)varId.dblVal;

			_variant_t varKhmc = pRecordKh->GetCollect("khmc");
			pKh->strKhmc = (const TCHAR*)(_bstr_t)varKhmc;

			_variant_t varLxfs = pRecordKh->GetCollect("lxfs");
			pKh->strLxfs = (const TCHAR*)(_bstr_t)varLxfs;

			_variant_t varJlxm = pRecordKh->GetCollect("jlxm");
			pKh->strJlxm = (const TCHAR*)(_bstr_t)varJlxm;

			_variant_t varXgsj = pRecordKh->GetCollect("xgsj");
			SYSTEMTIME st;
			VariantTimeToSystemTime(varXgsj.date, &st);
			TCHAR date[32];
			memset(date, 0x00, sizeof(date));
			_stprintf_s(date, 32, _T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			pKh->strXgsj = date;

			pSql = _T("SELECT * FROM sim_tbl WHERE khmc='%s'");
			memset(sql, 0x00, sizeof(sql));
			_stprintf_s(sql, 256, pSql, pKh->strKhmc.c_str());
			_RecordsetPtr pRecordSim;
			pRecordSim.CreateInstance(__uuidof(Recordset));
			pRecordSim->Open(_bstr_t(sql), (IDispatch*)conptr, adOpenDynamic, adLockOptimistic, adCmdText);
			VARIANT_BOOL bRtSim = pRecordSim->GetadoEOF();
			while (!bRtSim)
			{
				ST_SIM* pSim = new ST_SIM;
				_variant_t varId = pRecordSim->GetCollect("id");
				pSim->id = (int)varId.dblVal;

				_variant_t varJrhm = pRecordSim->GetCollect("jrhm");
				pSim->strJrhm = (const TCHAR*)(_bstr_t)varJrhm;

				_variant_t varIccid = pRecordSim->GetCollect("iccid");
				pSim->strIccid = (const TCHAR*)(_bstr_t)varIccid;

				mp_sim.insert(make_pair(pSim->id, pSim));

				pRecordSim->MoveNext();
				bRtSim = pRecordSim->GetadoEOF();
			}
			if (pRecordSim->State == adStateOpen)
			{
				pRecordSim->Close();
				pRecordSim.Release();
				pRecordSim = NULL;
			}

			mp_kh.insert(make_pair(pKh->id, pKh));

			pRecordKh->MoveNext();
			bRtKh = pRecordKh->GetadoEOF();
		}
		if (pRecordKh->State == adStateOpen)
		{
			pRecordKh->Close();
			pRecordKh.Release();
			pRecordKh = NULL;
		}

		mp_khjl.insert(make_pair(pKhjl->id, pKhjl));

		pRecord->MoveNext();
		bRt = pRecord->GetadoEOF();
	}

	if (pRecord->State == adStateOpen)
	{
		pRecord->Close();
		pRecord.Release();
		pRecord = NULL;
	}

	//system("pause");
	return 0;
}

int main()
{
	test();

	for (tmp_khjl::iterator iter = mp_khjl.begin(); iter != mp_khjl.end(); iter++)
	{
		cout << "{" << iter->second->id << "," << iter->second->strJlxm << "," << iter->second->strLxfs << "}" << endl;
		for (tmp_kh::iterator iter = mp_kh.begin(); iter != mp_kh.end(); iter++)
		{
			cout << "\t" << "{" << iter->second->id << "," << iter->second->strKhmc << "," << iter->second->strLxfs << "," << iter->second->strJlxm << "}" << endl;
			for (tmp_sim::iterator iter = mp_sim.begin(); iter != mp_sim.end(); iter++)
			{
				cout << "\t\t" << "{" << iter->second->id << "," << iter->second->strJrhm << "," << iter->second->strIccid << "}" << endl;
			}
		}
	}

	system("pause");
	return 0;
}