// DBLinkPool.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

static const _TCHAR* pDBName = _T("cardb");
static const _TCHAR* pUserName = _T("baolan123");
static const _TCHAR* pDBpwd = _T("baolan123");
static const _TCHAR* pDSN = _T("mysql_db_a");

class DBConnect
{
public:
	DBConnect(const _TCHAR* strDstAddress,
		const _TCHAR* strUserName = _T("baolan123"),
		const _TCHAR* strPassword = _T("baolan123"),
		const _TCHAR* strDBName = _T("cardb"),
		BOOL bstate = false):_strDstAddress(strDstAddress),
		_strUserName(strUserName),
		_strPassWord(strPassword),
		_strDBName(strDBName)
	{
		::CoInitialize(NULL);
	}
	~DBConnect()
	{
		::CoUninitialize();
	}

public:
	int Open(const _TCHAR* strDstAddress,
		const _TCHAR* strUserName,
		const _TCHAR* strPassword,
		const _TCHAR* strDBName);

	int Close();

	BOOL IsOpen() const;

private:
	_ConnectionPtr _connection_ptr;
	bool _isAdoValid;
	bool m_bDBOpen;
	const _TCHAR* _strDstAddress;
	const _TCHAR* _strUserName;
	const _TCHAR* _strPassWord;
	const _TCHAR* _strDBName;

public:
	int UserLogin(const TCHAR* name, const TCHAR* password);
	int CheckLinkState();
};

int DBConnect::Open(const _TCHAR* strDstAddress,
	const _TCHAR* strUserName,
	const _TCHAR* strPassword,
	const _TCHAR* strDBName)
{
	HRESULT hr = S_OK;
	hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		_tprintf(_T("初始化ado失败, code = %d\n"), GetLastError());
		return -1;
	}

	try
	{
		hr = _connection_ptr.CreateInstance(__uuidof(Connection));
		if (FAILED(hr))
		{
			_tprintf(_T("创建mysql连接对象失败, code = %d\n"), GetLastError());
			return -1;
		}
		
		_TCHAR strConn[256];
		memset(strConn, 0x00, sizeof(strConn));
		_stprintf_s(strConn, 256, _T("DATABASE=%s;DSN=%s;OPTION=0;PWD=%s;PORT=0;SERVER=localhost;UID=%s"), _strDBName, pDSN, _strPassWord, _strUserName);
		_tprintf(_T("连接字符串:%s"), strConn);
		hr = _connection_ptr->Open(_bstr_t(strConn), "", "", adModeUnknown);
		if (FAILED(hr))
		{
			_tprintf(_T("连接数据库失败, code = %d\n"), GetLastError());
			return -1;
		}
	}
	catch (_com_error& e)
	{
		_tprintf(_T("数据库操作失败!错误信息：%s, 文件：%s, 行：%d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return -1;
	}

	return 0;
}

int DBConnect::Close()
{
	HRESULT hr = _connection_ptr->Close();
	if (FAILED(hr))
	{
		_tprintf(_T("关闭数据库连接失败\n"));
		return -1;
	}

	::CoUninitialize();
	return 0;
}

int DBConnect::UserLogin(const TCHAR* name, const TCHAR* password)
{
	_CommandPtr cmd;
	HRESULT hr = cmd.CreateInstance(__uuidof(Command));
	if (FAILED(hr))
	{
		_tprintf(_T("创建Command失败\n"));
		return -1;
	}

	TCHAR szSql[256];
	memset(szSql, 0x00, sizeof(szSql));
	_stprintf_s(szSql, _T("select count(*) as count from t_user where name = /'%s/' and password = /'%s/'"),
		name, password);
	cmd->ActiveConnection = _connection_ptr;
	cmd->CommandText = _bstr_t(szSql);
	cmd->CommandType = adCmdText;

	try
	{
		_RecordsetPtr rs = cmd->Execute(NULL, NULL, adCmdUnknown);
		_variant_t count;
		while (!rs->adoEOF)
		{
			count = rs->GetCollect("count");
			rs->MoveNext();
		}

		if (count.intVal == 0)
			return -1;
	}
	catch (_com_error& e)
	{
		_tprintf(_T("sql操作失败!错误信息：%s, 文件：%s, 行：%d\n"), e.ErrorMessage(), __FILE__, __LINE__);
		return -1;
	}

	return 0;
}

int DBConnect::CheckLinkState()
{
	if (_connection_ptr->State)
	{
		_tprintf(_T("数据库连接正常\n"));
		return 0;
	}
	else
	{
		_tprintf(_T("数据库连接异常\n"));
		return -1;
	}
}

int main()
{
    return 0;
}

