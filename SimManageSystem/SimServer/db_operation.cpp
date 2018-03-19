#include "stdafx.h"
#include "db_operation.h"

#include <vector>
using std::vector;

#define DB_NAME _T("cardb")
#define DB_USER _T("baolan123")
#define DB_PWD _T("baolan123")
#define DB_DSN _T("mysql_db_a")

#define WM_RELEASE_DBLINK WM_USER + 100

int nMinConNum;
int nMaxConNum;
int nCurrentNum;

HANDLE hThreadStart;
HANDLE hEmptyEvent;
CRITICAL_SECTION cs_DBLinkVector;

vector<_ConnectionPtr *> v_DBLinkVector;

unsigned int nThreadID;
unsigned int _stdcall tfunc_dblink(LPVOID pVoid);

bool CreateDBConnection(_ConnectionPtr& conptr);
void ReleaseConnectionPtr(_ConnectionPtr& conptr);
bool ExcuteWithoutCheck(_ConnectionPtr& conptr, const TCHAR* bSql);
bool GetRecordSetWithoutCheck(_ConnectionPtr &conptr, const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/);

void InitDBPool()
{
	::CoInitialize(NULL);

	nMinConNum = 0;
	nMaxConNum = 0;
	nCurrentNum = 0;
	hThreadStart = CreateEvent(NULL, FALSE, FALSE, NULL);
	hEmptyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&cs_DBLinkVector);
	nThreadID = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, tfunc_dblink, NULL, 0, &nThreadID);
	WaitForSingleObject(hThreadStart, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hThreadStart);
}

void ReleaseDBPool()
{
	::CoUninitialize();
}

void InitConnection(const int iMix, const int iMax)
{
	static bool bInitial = true; // 防止多次创建初始化
	if (bInitial)
	{
		nMinConNum = iMix;
		nMaxConNum = iMax;
		for (int i = 0; i < iMix; i++)
		{
			_ConnectionPtr* conptr = new _ConnectionPtr();
			if (CreateDBConnection(*conptr))
			{
				EnterCriticalSection(&cs_DBLinkVector);
				v_DBLinkVector.push_back(conptr);
				nCurrentNum++;
				LeaveCriticalSection(&cs_DBLinkVector);
			}
		}
		bInitial = false;
	}
}

_ConnectionPtr* GetTransConnection()
{
	_ConnectionPtr* conptr = GetConnectionPtr();
	if ((*conptr)->State != adStateOpen)
	{
		ReleaseConnectionPtr(*conptr);
		CreateDBConnection(*conptr);
	}
	return conptr;
}

bool ExcuteSql(const TCHAR* bSql, bool bCheck)
{
	_ConnectionPtr* conptr = GetTransConnection();
	bool bExec = ExcuteWithoutCheck(*conptr, bSql);
	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return bExec;
}

bool GetRecordSet(const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/, bool bCheck)
{
	_ConnectionPtr *conptr = GetTransConnection();
	bool bExec = GetRecordSetWithoutCheck(*conptr, bSql, pRecord, lOption);
	PostThreadMessage(nThreadID, WM_RELEASE_DBLINK, (WPARAM)conptr, NULL);
	return bExec;
}

int main02()
{
	InitDBPool();
	InitConnection(10, 50);

	TCHAR strInsert[256];
	memset(strInsert, 0x00, sizeof(strInsert));

	_stprintf_s(strInsert, 256, _T("insert into teacher (id,name,address,year) value(3,'往事了', '周口', '2018-01-23')"));
	ExcuteSql(strInsert, true);

	_RecordsetPtr pRs;
	GetRecordSet(_T("select * from teacher"), pRs, adCmdText, false);
	
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

	system("pause");
	return 0;
}

unsigned int _stdcall tfunc_dblink(LPVOID pVoid)
{
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(hThreadStart);

	while (1)
	{
		if (GetMessage(&msg, 0, 0, 0))
		{
			switch (msg.message)
			{
			case WM_RELEASE_DBLINK:
			{
				_ConnectionPtr* conptr = (_ConnectionPtr*)(msg.wParam);
				EnterCriticalSection(&cs_DBLinkVector);
				v_DBLinkVector.push_back(conptr);
				LeaveCriticalSection(&cs_DBLinkVector);
				SetEvent(hEmptyEvent);
			}
			break;

			default:
				break;
			}
		}
	}

	return 0;
}

bool CreateDBConnection(_ConnectionPtr& conptr)
{
	try
	{
		conptr.CreateInstance(__uuidof(Connection));
	//	const TCHAR* strConn = _T("");
		TCHAR strConn[256];
		memset(strConn, 0x00, sizeof(strConn));
		_stprintf_s(strConn, 256, _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s"), DB_NAME, DB_DSN, DB_PWD, DB_USER);
		_tprintf(_T("连接字符串:%s\n"), strConn);
		HRESULT hr = conptr->Open(_bstr_t(strConn), "", "", adModeUnknown);
		if (FAILED(hr))
			return false;
	}
	catch (_com_error& e)
	{
		_tprintf(_T("123\n"));
	}
	return true;
}

_ConnectionPtr *GetConnectionPtr()
{
	while (1)
	{
		EnterCriticalSection(&cs_DBLinkVector);
		_ConnectionPtr* conptr;
		if (v_DBLinkVector.empty())
		{
			if (nCurrentNum < nMaxConNum)
			{
				conptr = new _ConnectionPtr;
				if (CreateDBConnection(*conptr))
					nCurrentNum++;
			}
			else
			{
				ResetEvent(hEmptyEvent);
				LeaveCriticalSection(&cs_DBLinkVector);
				WaitForSingleObject(hEmptyEvent, INFINITE);
				continue;
			}
		}
		else 
		{
			conptr = v_DBLinkVector.back();
			v_DBLinkVector.pop_back();
		}
		LeaveCriticalSection(&cs_DBLinkVector);
		return conptr;
	}
}

void ReleaseConnectionPtr(_ConnectionPtr& conptr)
{
	if (conptr != NULL)
	{
		conptr->Close();// 关闭连接
		conptr.Release();// 释放内存
		conptr = NULL;
	}
}

bool ExcuteWithoutCheck(_ConnectionPtr& conptr, const TCHAR* bSql)
{
	int i = 0;
	while (i < 3)
	{
		try
		{
			if (0 != i)
			{
				ReleaseConnectionPtr(conptr);
				CreateDBConnection(conptr);
			}
			++i;
			VARIANT nRecordAffected = { 0 };
			conptr->Execute(_bstr_t(bSql), &nRecordAffected, adCmdText);
			if (nRecordAffected.date < 0)
				return false;
			break;
		}
		catch (_com_error& e)
		{

		}
		catch (...)
		{

		}
	}
	if (3 == i)
		return false;

	return true;
}

bool GetRecordSetWithoutCheck(_ConnectionPtr &conptr, const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/)
{
	for (int i = 0; i < 3; i++)
	{
		try
		{
			if (0 != i)
			{
				ReleaseConnectionPtr(conptr);
				CreateDBConnection(conptr);
			}
			HRESULT hr = pRecord.CreateInstance(__uuidof(Recordset));
			if (SUCCEEDED(hr))
			{
				pRecord->CursorLocation = adUseClient;
				hr = pRecord->Open(_bstr_t(bSql), _variant_t((IDispatch*)conptr), adOpenDynamic, adLockOptimistic, lOption);
				return SUCCEEDED(hr);
			}

			return false;
		}
		catch (_com_error& e)
		{

		}
		catch (...)
		{

		}
	}
	return false;
}