#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "sql_test.h"
#include "db_operation.h"
#include "parse_data.h"

//int ReturnRecordCount(const TCHAR* sql, _ConnectionPtr& conptr)
//{
//	return 0;
//}
//
//bool GetKhjlInfo01()
//{
//	const TCHAR* pSql = _T("CREATE TEMPORARY TABLE tmp_tbl SELECT * FROM sim_tbl WHERE khjl='%s'");
//	_ConnectionPtr* conptr = GetTransConnection();
//	bool bExec = ExcuteWithoutCheck(*conptr, pSql);
//	_RecordsetPtr pRecord;
//	_variant_t nRecordAffected = { 0 };
//	//try
//	//{
//	//	pSql = _T("insert into");
//	//	(*conptr)->Execute(_bstr_t(pSql), &nRecordAffected, adCmdTable);
//	//}
//	//catch (_com_error e)
//	//{
//	//	if (3119 == e.WCode)
//	//		_tprintf(_T("已有该记录\n"));
//	//}
//
//	//try
//	//{
//	//	pRecord.CreateInstance(__uuidof(Recordset));
//
//	//	pRecord->Open(_bstr_t(pSql), _variant_t((IDispatch*)conptr), adOpenDynamic, adLockOptimistic, adCmdText);
//	//	if (!pRecord->adoEOF)
//	//	{
//
//	//	}
//	//	else
//	//	{
//
//	//	}
//
//	//	if (pRecord->State == adStateOpen) pRecord->Close();
//	//	if (!pRecord->adoEOF)
//	//	{
//
//	//	}
//	//	else
//	//	{
//
//	//	}
//	//}
//	//catch (_com_error e)
//	//{
//	//	const TCHAR* pError = e.ErrorMessage();
//	//	//TRACE("FILE:%s,Line Number:%d,Error:%s", __FILE__, __LINE__, pError);
//	//}
//
//	pSql = _T("SELECT COUNT(*) num01 FROM tmp_tbl WHERE dqrq>CURDATE() AND dqrq>DATE_ADD(CURDATE(),INTERVAL 1 MONTH");
//	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
//	int n1 = pRecord->GetRecordCount();
//
//	pSql = _T("SELECT COUNT(*) num02 FROM tmp_tbl WHERE dqrq>CURDATE() AND dqrq>DATE_ADD(CURDATE(),INTERVAL 15 DAY");
//	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
//	int n2 = pRecord->GetRecordCount();
//
//	pSql = _T("SELECT COUNT(*) num03 FROM tmp_tbl WHERE dqrq<CURDATE() AND dqrq<DATE_SUB(CURDATE(),INTERVAL 1 MONTH");
//	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
//	int n3 = pRecord->GetRecordCount();
//
//	pSql = _T("SELECT COUNT(*) num04 FROM tmp_tbl WHERE dqrq<CURDATE() AND dqrq<DATE_SUB(CURDATE(),INTERVAL 15 DAY");
//	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
//	int n4 = pRecord->GetRecordCount();
//
//	pSql = _T("DROP TABLE tmp_tbl");
//	bExec = ExcuteWithoutCheck(*conptr, pSql);
//	
//	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
//	return bExec;
//}

#define DB_NAME _T("cardb")
#define DB_USER _T("baolan123")
#define DB_PWD _T("baolan123")
#define DB_DSN _T("mysql_db_a")
bool doSqlTest(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	_ConnectionPtr* conptr = new _ConnectionPtr;
	delete conptr;
	(*conptr).CreateInstance(__uuidof(Connection));
	TCHAR strConn[256];
	memset(strConn, 0x00, sizeof(strConn));
	_stprintf_s(strConn, 256, _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s"), DB_NAME, DB_DSN, DB_PWD, DB_USER);
	//(*conptr)->Open(_bstr_t(strConn), "", "", adModeUnknown);
	if ((*conptr) == NULL)
	{
		_tprintf(_T("123123\n"));
	}
	//if ((*conptr)->State == adStateOpen)
	{
		//(*conptr)->Close();
		(*conptr)->Release();
		(*conptr) = NULL;
	}
	//else
	{
		(*conptr)->Close();
		(*conptr)->Release();
		delete conptr;
		conptr = NULL;
	}
		
	//
	

	//msgpack::sbuffer sbuf;
	//msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	//sbuf.write("\xfb\xfc", 6);

	//_ConnectionPtr *conptr = GetTransConnection();

	//const TCHAR* pSql = _T("CREATE TEMPORARY TABLE tmp_tbl SELECT * FROM sim_tbl WHERE khjl='%s'");
	//VARIANT nRecordAffected = { 0 };
	//(*conptr)->Execute(_bstr_t(pSql), &nRecordAffected, adCmdText);


	//pSql = _T("select * from tmp_tbl where id > 7125");
	//_RecordsetPtr pRecord;

	//HRESULT hr = pRecord.CreateInstance(__uuidof(Recordset));
	//if (SUCCEEDED(hr))
	//{
	//	pRecord->CursorLocation = adUseClient;
	//	hr = pRecord->Open(_bstr_t(pSql), _variant_t((IDispatch*)(*conptr)), adOpenDynamic, adLockOptimistic, adCmdText);
	//	if (FAILED(hr))
	//		return false;
	//}

	////if (!GetRecordSetDate(pSql, pRecord, 9, 1, msgPack))
	////{
	////	DealLast(sbuf, bobj);
	////	return false;
	////}

	//int n = pRecord->GetRecordCount();

	//_T("DROP TABLE tmp_tbl");

	//DealLast(sbuf, bobj);
	return true;
}