#include "stdafx.h"
#include <msgpack.hpp>
#include "singleton_data.h"
#include "sql_test.h"
#include "db_operation.h"
#include "parse_data.h"

int ReturnRecordCount(const TCHAR* sql, _ConnectionPtr& conptr)
{
	return 0;
}

bool GetKhjlInfo01()
{
	const TCHAR* pSql = _T("CREATE TEMPORARY TABLE tmp_tbl SELECT * FROM sim_tbl WHERE khjl='%s'");
	_ConnectionPtr* conptr = GetTransConnection();
	bool bExec = ExcuteWithoutCheck(*conptr, pSql);

	_RecordsetPtr pRecord;

	pSql = _T("SELECT id FROM tmp_tbl WHERE dqrq>CURDATE() AND dqrq>DATE_ADD(CURDATE(),INTERVAL 1 MONTH");
	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
	int n1 = pRecord->GetRecordCount();

	pSql = _T("SELECT id FROM tmp_tbl WHERE dqrq>CURDATE() AND dqrq>DATE_ADD(CURDATE(),INTERVAL 15 DAY");
	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
	int n2 = pRecord->GetRecordCount();

	pSql = _T("SELECT id FROM tmp_tbl WHERE dqrq<CURDATE() AND dqrq<DATE_SUB(CURDATE(),INTERVAL 1 MONTH");
	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
	int n3 = pRecord->GetRecordCount();

	pSql = _T("SELECT id FROM tmp_tbl WHERE dqrq<CURDATE() AND dqrq<DATE_SUB(CURDATE(),INTERVAL 15 DAY");
	bExec = GetRecordSetWithoutCheck(*conptr, pSql, pRecord, adCmdText);
	int n4 = pRecord->GetRecordCount();

	pSql = _T("DROP TABLE tmp_tbl");
	bExec = ExcuteWithoutCheck(*conptr, pSql);
	
	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return bExec;
}

bool doSqlTest(msgpack::unpacked& result_, BUFFER_OBJ* bobj)
{
	msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);

	_ConnectionPtr *conptr = GetTransConnection();

	const TCHAR* pSql = _T("CREATE TEMPORARY TABLE tmp_tbl SELECT * FROM sim_tbl WHERE khjl='%s'");
	VARIANT nRecordAffected = { 0 };
	(*conptr)->Execute(_bstr_t(pSql), &nRecordAffected, adCmdText);


	pSql = _T("select * from tmp_tbl where id > 7125");
	_RecordsetPtr pRecord;

	HRESULT hr = pRecord.CreateInstance(__uuidof(Recordset));
	if (SUCCEEDED(hr))
	{
		pRecord->CursorLocation = adUseClient;
		hr = pRecord->Open(_bstr_t(pSql), _variant_t((IDispatch*)(*conptr)), adOpenDynamic, adLockOptimistic, adCmdText);
		if (FAILED(hr))
			return false;
	}

	//if (!GetRecordSetDate(pSql, pRecord, 9, 1, msgPack))
	//{
	//	DealLast(sbuf, bobj);
	//	return false;
	//}

	int n = pRecord->GetRecordCount();

	_T("DROP TABLE tmp_tbl");

	DealLast(sbuf, bobj);
	return true;
}