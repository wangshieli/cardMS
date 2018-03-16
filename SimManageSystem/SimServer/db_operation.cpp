#include "stdafx.h"
#include "db_operation.h"

#define DB_NAME _T("cardb")
#define DB_USER _T("baolan123")
#define DB_PWD _T("baolan123")
#define DB_DSN _T("mysql_db_a")

_ConnectionPtr g_pDBConnection;

BOOL DBConnect()
{
	HRESULT hr = S_OK;
	hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		_tprintf(_T("ado��ʼ��ʧ��, errCode = %d\n"), GetLastError());
		return false;
	}

	try
	{
		hr = g_pDBConnection.CreateInstance(__uuidof(Connection));
		if (FAILED(hr))
		{
			_tprintf(_T("�������ݿ����Ӷ���ʧ��, errCode = %d\n"), GetLastError());
			::CoUninitialize();
			return false;
		}

		TCHAR strConn[256];
		memset(strConn, 0x00, sizeof(strConn));
		_stprintf_s(strConn, 256, _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s"), DB_NAME, DB_DSN, DB_PWD, DB_USER);
		_tprintf(_T("�����ַ���:%s\n"), strConn);
		hr = g_pDBConnection->Open(_bstr_t(strConn), "", "", adModeUnknown);
		if (FAILED(hr))
		{
			_tprintf(_T("�������ݿ�ʧ��, errCode = %d\n"), GetLastError());
			::CoUninitialize();
			return false;
		}
	}
	catch (_com_error& e)
	{
		_tprintf(_T("���ݿ����ʧ��, �ļ�: %s, ��: %d, ������Ϣ: %s\n"), __FILE__, __LINE__, e.ErrorMessage());
		if (g_pDBConnection->State == adStateOpen)
			g_pDBConnection->Close();
		::CoUninitialize();
		return false;
	}

	return true;
}

_RecordsetPtr DBSelect(const TCHAR* pSql)
{
	//const TCHAR* p = _T("insert into test_tbl (id,name) VALUES(76,'WANG')");
	//g_pDBConnection->Execute(_bstr_t(p), NULL, adCmdText);

	_RecordsetPtr pRs;
	HRESULT hr = S_OK;
	hr = pRs.CreateInstance(__uuidof(Recordset));
	if (FAILED(hr))
	{
		_tprintf(_T("����������ʧ��\n"));
		return pRs;
	}

	hr = pRs->Open(_bstr_t("select * from test_tbl"), g_pDBConnection.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);
	if (FAILED(hr))
	{
		_tprintf(_T("ִ�� sql : %s ʧ��\n"), pSql);
		return pRs;
	}

	/************************/
	VARIANT_BOOL bRt = pRs->GetadoEOF();
	while (!bRt)
	{
		_variant_t varUserID = pRs->GetCollect("id");
		if (varUserID.vt == VT_BSTR)
		{
			const TCHAR* pData = (const TCHAR*)varUserID.bstrVal;
			_tprintf(_T("id = %s\n"), pData);
		}
		_variant_t varUserName = pRs->GetCollect("name");
		if (varUserName.vt == VT_BSTR)
		{
			const TCHAR* pData = (const TCHAR*)varUserName.bstrVal;
			_tprintf(_T("UserName = %s\n"), pData);
		}

		pRs->MoveNext();
		bRt = pRs->GetadoEOF();
	}
	/************************/

	return pRs;
}

int main()
{
	DBConnect();

	_RecordsetPtr pRs = DBSelect(_T("select * from test_tbl"));
	pRs->MoveFirst();
	VARIANT_BOOL bRt = pRs->GetadoEOF();
	while (!bRt)
	{
		_variant_t varUserID = pRs->GetCollect("id");
		if (varUserID.vt == VT_BSTR)
		{
			const TCHAR* pData = (const TCHAR*)varUserID.bstrVal;
			_tprintf(_T("id = %s\n"), pData);
		}
		_variant_t varUserName = pRs->GetCollect("name");
		if (varUserName.vt == VT_BSTR)
		{
			const TCHAR* pData = (const TCHAR*)varUserName.bstrVal;
			_tprintf(_T("UserName = %s\n"), pData);
		}

		pRs->MoveNext();
		bRt = pRs->GetadoEOF();
	}

	if ((BOOL)pRs == TRUE)
	{
		if (pRs->State == adStateOpen)
			pRs->Close();
		pRs.Release();
		pRs = NULL;
	}
	
	system("pause");
	return 0;
}