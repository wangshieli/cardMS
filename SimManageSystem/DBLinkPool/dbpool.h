#pragma once

typedef struct
{
	TCHAR db_ip[20];
	UINT32 db_port;
	TCHAR db_user[20];
	TCHAR db_pwd[32];
	TCHAR db_dbname[32];
}vos_dbxmlinfo_stru;

class CXCADOPOOL
{
protected:
	CXCADOPOOL();

public:
	virtual ~CXCADOPOOL(void);

public:
	void	InitConnection(const int iMin, const int iMax);
	bool	ExcuteSql(_bstr_t bSql, bool bCheck = true);
	bool	GetRecordSet(_bstr_t bSql, _RecordsetPtr& pRecord, long lOption = adCmdText, bool bCheck = true);

	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, int& nValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, UINT64& unValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, string& strValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, double& dValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, float& fValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, ULONG& nValue);

	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, short& sValue);
	bool	GetItemValue(_RecordsetPtr pRecord, long nIndex, unsigned char& nValue);
	bool	GetItemValue(_RecordsetPtr pRecord, string fieldname, string& strValue);

	template<class T>
	bool	GetItemValue(_RecordsetPtr pRecord, string fieldname, T& tValue);

	static CXCADOPOOL* Instance();
	_ConnectionPtr *GetTransConnection();
	void	SendTransCompMsg(_ConnectionPtr* pConptr);
	bool    ExecuteTransSql(_ConnectionPtr *pConptr, _bstr_t bSql);

private:
	bool    CreateDBConnection(_ConnectionPtr & conptr); //返回一个连接 
	void    GetConnectionString(string &strConnect);
	_ConnectionPtr * GetConnectionPtr();
	void    ReleaseConnectionPtr(_ConnectionPtr &conptr);
	void    InitDBConfig();
	bool    ExcuteWithoutCheck(_ConnectionPtr &conptr, _bstr_t bSql);
	bool    GetRecordSetWithoutCheck(_ConnectionPtr &conptr, _bstr_t bSql, _RecordsetPtr& pRecord, long lOption = adCmdText);
	static DWORD WINAPI IdleConnThreadFunc(LPVOID lParam);

private:

	queue<_ConnectionPtr *> m_qConn;
	int m_MinConNum;    //最小连接数
	int m_MaxConNum;    //最大连接数
	int m_CurrentNum;    //当前连接数

	HANDLE				m_Mutex;
	HANDLE				m_hEvent;
	HANDLE				m_hThread;
	DWORD				m_dwThreadId;
	HANDLE				m_hThreadEvent;
	string				m_strConnect;
	static CXCADOPOOL* _instance;
public:
	vos_dbxmlinfo_stru    m_stDBInfo;
};

template<class T>
bool CXCADOPOOL::GetItemValue(_RecordsetPtr pRecord, string fieldname, T& tValue)
{
	try
	{
		//ASSERT_RECORDSET(pRecord);
		assert(pRecord != NULL);
		_variant_t vart = pRecord->GetCollect(_variant_t(fieldname.c_str()));
		(tValue = (T)(vart));
	}
	catch (_com_error &)
	{
		return false;
	}
	return true;
}
extern CXCADOPOOL *pAdoPool;