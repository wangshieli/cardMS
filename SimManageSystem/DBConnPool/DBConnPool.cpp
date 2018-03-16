#include "DBConnPool.h"
DBConnect * pDBConn = NULL;
void InitDBIterface(LPCSTR strDstAddress, LPCSTR strUsername, LPCSTR strPassword, LPCSTR strDBName, int minConn, int maxConn)
{
	DBConnPool::Instanse()->SetDBInfo(strDstAddress, strUsername, strPassword, strDBName, minConn, maxConn);
	int cout = DBConnPool::Instanse()->InitializeAllDBConnections();
}
DBConnect * GetAConnect()
{
	DBConnGuard oConnGuard(pDBConn);
	return pDBConn;
}
//////////////////////////////////////DBConnect/////////////////////////
DBConnect::DBConnect(LPCTSTR strDstAddress, LPCTSTR strUsername, LPCTSTR strPassword, LPCTSTR strDBName, BOOL &state)
{
	m_bDBOpen = false;
	if (0 == Open(strDstAddress, strUsername, strPassword, strDBName))
	{
		state = true;
	}
	else
	{
		state = false;
	}
}
DBConnect::~DBConnect()
{
	Close();
}
int DBConnect::Open(LPCTSTR strDstAddress, LPCTSTR strUsername, LPCTSTR strPassword, LPCTSTR strDBName)
{
	_strDstAddress = strDstAddress;
	_strDBName = strDBName;
	_strUsername = strUsername;
	_strPassword = strPassword;

	HRESULT comhr = ::CoInitialize(NULL);
	if (FAILED(comhr))

	{
		return -1;
	}
	HRESULT hr = _connection_ptr.CreateInstance(__uuidof(Connection));

	if (FAILED(hr))
	{
		return -1;
	}

	char szSQL[SQL_BUFFER_LEN] = { 0 };
	memset(szSQL, 0, SQL_BUFFER_LEN);
	sprintf(szSQL, "Driver=SQL Server;Server=%s;DATABASE=%s", strDstAddress, strDBName);

	try
	{
		// ���ӵ������������ݿ�
		_connection_ptr->Open(szSQL, strUsername, strPassword, adModeUnknown);
		if (FAILED(hr))
			return -1;
	}
	catch (_com_error &err)
	{
		TRACE(_T("���ݿ����ʧ��! ������Ϣ:%s, �ļ�:%s, ��:%d./n"), err.ErrorMessage(), __FILE__, __LINE__);

		return -1;
	}

	m_bDBOpen = TRUE;

	return 0;
}
int DBConnect::Close()
{
	if (m_bDBOpen)
	{
		HRESULT hr = _connection_ptr->Close();

		if (FAILED(hr))
		{
			return -1;
		}
		::CoUninitialize();
		m_bDBOpen = FALSE;
	}

	return 0;
}
int DBConnect::UserLogin(LPCSTR name, LPCSTR password)
{
	if (!m_bDBOpen)
	{
		return -1;
	}
	// ����Command����
	_CommandPtr cmd;
	HRESULT hr = cmd.CreateInstance(__uuidof(Command));
	if (FAILED(hr))
	{
		return -1;
	}

	char szSQL[SQL_BUFFER_LEN] = { 0 };
	sprintf(szSQL, "select count(*) as count from t_user where name =/'%s/' and password =/'%s/'",
		name, password);
	cmd->ActiveConnection = _connection_ptr;
	cmd->CommandText = _bstr_t(szSQL);
	cmd->CommandType = adCmdText;
	try
	{
		_RecordsetPtr rs = cmd->Execute(NULL, NULL, adCmdUnknown);

		if (FAILED(hr))
		{
			return -1;
		}
		_variant_t count;
		while (!rs->ADOEOF)
		{
			count = rs->GetCollect("count");
			rs->MoveNext();
		}
		if (count.intVal == 0)
			return -1;
	}
	catch (_com_error &err)
	{
		TRACE(_T("���ݿ����ʧ��! ������Ϣ:%s, �ļ�:%s, ��:%d./n"), err.ErrorMessage(), __FILE__, __LINE__);

		return -1;
	}
	return 0;
}
int DBConnect::GetSubmitInfo(vector<SOAP_SUBMIT_SMS> &vecsoapSms)
{
	if (!m_bDBOpen)
	{
		return -1;
	}
	// ����Command����
	_CommandPtr cmd;
	HRESULT hr = cmd.CreateInstance(__uuidof(Command));
	if (FAILED(hr))
	{
		return -1;
	}
	string strSql = "exec SP_SMS_GETSUBMIT";
	cmd->ActiveConnection = _connection_ptr;
	cmd->CommandText = _bstr_t(strSql.c_str());
	cmd->CommandType = adCmdText;
	try
	{
		_RecordsetPtr rs = cmd->Execute(NULL, NULL, adCmdUnknown);

		if (FAILED(hr))
		{
			return -1;
		}
		while (!rs->ADOEOF)
		{
			_variant_t		id;
			_variant_t		recverid;
			_variant_t		recvertel;
			_variant_t		messagecontent;
			_variant_t		recverphonetype;
			_variant_t		inout;
			_variant_t		sendtime;
			_variant_t		cardtime;
			_variant_t		schoolid;
			_variant_t		classid;
			_variant_t		rfidno;

			id = rs->GetCollect("id");
			recverid = rs->GetCollect("recverid");
			recvertel = rs->GetCollect("recvertel");
			messagecontent = rs->GetCollect("messagecontent");
			recverphonetype = rs->GetCollect("recverphonetype");
			inout = rs->GetCollect("inout");
			sendtime = rs->GetCollect("sendtime");
			cardtime = rs->GetCollect("cardtime");
			schoolid = rs->GetCollect("schoolid");
			classid = rs->GetCollect("classid");
			rfidno = rs->GetCollect("rfidno");
			SOAP_SUBMIT_SMS submitsms;
			submitsms.id = (int)(long)id;
			VarientToString(recverid, submitsms.recverid);
			VarientToString(recvertel, submitsms.recvertel);
			VarientToString(messagecontent, submitsms.messagecontent);
			VarientToString(recverphonetype, submitsms.recverphonetype);
			submitsms.inout = (int)(long)inout;
			VarientToString(sendtime, submitsms.sendtime);
			VarientToString(cardtime, submitsms.cardtime);
			VarientToString(rfidno, submitsms.rfidno);
			submitsms.schoolid = (int)(long)schoolid;
			submitsms.classid = (int)(long)classid;
			vecsoapSms.push_back(submitsms);
			rs->MoveNext();
		}
	}
	catch (_com_error &err)
	{
		TRACE(_T("���ݿ����ʧ��! ������Ϣ:%s, �ļ�:%s, ��:%d./n"), err.ErrorMessage(), __FILE__, __LINE__);

		return -1;
	}
	return 0;
}
int DBConnect::InsertHistory(int id)
{
	if (!m_bDBOpen)
	{
		return -1;
	}
	// ����Command����
	_CommandPtr cmd;
	HRESULT hr = cmd.CreateInstance(__uuidof(Command));
	if (FAILED(hr))
	{
		return -1;
	}
	char szSQL[SQL_BUFFER_LEN] = { 0 };
	sprintf(szSQL, "exec SP_SMS_SUBMITRESULT %d", id);
	try
	{
		cmd->ActiveConnection = _connection_ptr;
		cmd->CommandText = _bstr_t(szSQL);
		cmd->CommandType = adCmdText;
		cmd->Execute(NULL, NULL, adCmdUnknown);
	}
	catch (_com_error &err)
	{
		TRACE(_T("���ݿ����ʧ��! ������Ϣ:%s, �ļ�:%s, ��:%d./n"), err.ErrorMessage(), __FILE__, __LINE__);

		return -1;
	}
	return 0;
}
void DBConnect::VarientToString(_variant_t var, string& str)
{
	if (VT_NULL == var.vt)
	{
		str = "";
	}
	else
	{
		str = (char*)_bstr_t(var);
	}
}
//////////////////////////////////////End//////////////////////////////
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DBConnPool* DBConnPool::m_pInstanse = NULL;
DBConnPool::DBConnPool()
{
	m_bNeedStop = FALSE;
	m_bNeedConnection = FALSE;
	m_hMaintanceThread = INVALID_HANDLE_VALUE;

	// �߳̿���
	m_hHaveData = CreateEvent(NULL, TRUE, FALSE, _T("DataConnPool"));


	InitializeCriticalSection(&m_csIdleConnList);
	InitializeCriticalSection(&m_csBusyConnList);
}
void DBConnPool::SetDBInfo(LPCSTR strDstAddress, LPCSTR strUsername, LPCSTR strPassword, LPCSTR strDBName, int minConn, int maxConn)
{
	_strDBName = strDBName;
	_strDstAddress = strDstAddress;
	_strPassword = strPassword;
	_strUsername = strUsername;
	m_nMaxCount = maxConn;
	m_nMinCount = minConn;
}
DBConnPool::~DBConnPool()
{
	m_hMaintanceThread = INVALID_HANDLE_VALUE;
	m_bNeedStop = TRUE;
	CloseHandle(m_hHaveData);
	CloseHandle(m_hMaintanceThread);

	DeleteCriticalSection(&m_csIdleConnList);
	DeleteCriticalSection(&m_csBusyConnList);
}
DBConnPool *DBConnPool::Instanse()
{
	if (NULL == m_pInstanse)
	{
		m_pInstanse = new DBConnPool();
	}
	return m_pInstanse;
}
int DBConnPool::InitializeAllDBConnections()
{
	// ���߹������е�����
	DestroyAllDBConnections();
	// ��ʼ������С������ʼ����
	DBConnect * pDBConnect = NULL;
	int nCount = 0;
	for (int i = 0; i < m_nMinCount; i++)
	{
		nCount = InitializeAConnection();
	}
	// ����һ�������̣߳���������һЩ��̨ά������
	if (INVALID_HANDLE_VALUE == m_hMaintanceThread)
	{
		m_hMaintanceThread = CreateThread(NULL, NULL, thread_run, (LPVOID)this, 0, NULL);
	}
	return nCount;
}
void DBConnPool::DestroyAllDBConnections()
{
	// �������ݿ����ӿ���ʹ�ô���
	EnterCriticalSection(&m_csIdleConnList);
	DBConnectList::iterator itIdle = m_listIdleConnection.begin();
	DBConnectList::iterator itIdleEnd = m_listIdleConnection.end();
	while (itIdle != itIdleEnd)
	{
		if (NULL != (*itIdle))
		{
			(*itIdle)->Close();
			delete (*itIdle);
		}
		// erase����ͻ�ѵ�����ָ����һ��
		itIdle = m_listIdleConnection.erase(itIdle);
	}
	LeaveCriticalSection(&m_csIdleConnList);
	// ����ʹ���е�����
	EnterCriticalSection(&m_csBusyConnList);
	DBConnectList::iterator itBusy = m_listBusyConnection.begin();
	DBConnectList::iterator itBusyEnd = m_listBusyConnection.end();
	while (itBusy != itBusyEnd)
	{
		if (NULL != (*itBusy))
		{
			(*itBusy)->Close();
			delete (*itBusy);
		}
		// erase����ͻ�ѵ�����ָ����һ��
		itBusy = m_listBusyConnection.erase(itBusy);
	}
	LeaveCriticalSection(&m_csBusyConnList);
}
int DBConnPool::InitializeAConnection()
{
	BOOL bSuccess = FALSE;
	DBConnect * pDBEngine = new DBConnect(_strDstAddress, _strUsername, _strPassword, _strDBName, bSuccess);
	if (bSuccess)
	{
		m_bNeedConnection = FALSE;
		return RestoreAConnection(pDBEngine);
	}
	else
	{
		delete pDBEngine;
		return m_listIdleConnection.size();
	}
}
void DBConnPool::CloseAConnection(DBConnect* pDBEngine)
{
	pDBEngine->Close();
	// �ӿ��ж��н���ɾ��
	EnterCriticalSection(&m_csIdleConnList);
	m_listIdleConnection.remove(pDBEngine);
	LeaveCriticalSection(&m_csIdleConnList);
}
DBConnect * DBConnPool::GetAConnection()
{
	DBConnect * pDBEngine = NULL;
	// ��һ��ѭ���������������ȡ���ӣ�ÿ�μ��1����
	int nTimes = 0;
	while ((m_listIdleConnection.size() <= 0) && (nTimes < 5))
	{
		Sleep(1000);
		nTimes++;
	}
	if (5 == nTimes)
	{
		// �����Ǳ��Ľ����϶���û�п��������ˣ���¼��־�˳�
		//	g_pSvrLog->AddRunLog(LL_ERROR, _T("Waiting for a connection for a long time, but failed."));
		return pDBEngine;
	}
	// �ӿ��ж�����ȡ�������Ҽ��뵽ʹ�ö�����
	EnterCriticalSection(&m_csIdleConnList);
	if (m_listIdleConnection.size() > 0)
	{
		pDBEngine = m_listIdleConnection.front();
		m_listIdleConnection.pop_front();
		// ����ʹ�õ����Ӷ���
		EnterCriticalSection(&m_csBusyConnList);
		m_listBusyConnection.push_back(pDBEngine);
		LeaveCriticalSection(&m_csBusyConnList);
	}
	LeaveCriticalSection(&m_csIdleConnList);
	if (m_listIdleConnection.size() <= 1)
	{
		// ʣ��������ӵ���ĿС�ڵ���1��ʱ����Ҫ��鿪ʼ����
		if ((m_listIdleConnection.size() + m_listBusyConnection.size()) < m_nMaxCount)
		{
			// ��С��������ƣ����Դ���
			SetEvent(m_hHaveData);
			m_bNeedConnection = TRUE;
		}
		else
		{
			// ���������ˣ�������¼��
			//g_pSvrLog->AddRunLog(LL_ERROR, _T("Database connection reached max count."));
		}
	}
	return pDBEngine;
}
int DBConnPool::RestoreAConnection(DBConnect* pDBEngine)
{
	if (NULL != pDBEngine)
	{
		// ��ʹ���еĶ���ȡ��
		EnterCriticalSection(&m_csBusyConnList);
		m_listBusyConnection.remove(pDBEngine);
		LeaveCriticalSection(&m_csBusyConnList);
		// ���뵽���ж�����
		EnterCriticalSection(&m_csIdleConnList);
		m_listIdleConnection.push_back(pDBEngine);
		LeaveCriticalSection(&m_csIdleConnList);
	}
	EnterCriticalSection(&m_csIdleConnList);
	int nCount = m_listIdleConnection.size();
	LeaveCriticalSection(&m_csIdleConnList);
	return nCount;
}
void DBConnPool::StopThread()
{
	m_bNeedStop = TRUE;
	// ��Ϊ�߳��������Ƶȴ��źŵģ����������Ȱѱ�־λ��Ϊֹͣ��Ȼ���ź����̼߳��
	SetEvent(m_hHaveData);
	// �ȴ��˳�
	WaitForSingleObject(m_hMaintanceThread, INFINITE);
	CloseHandle(m_hMaintanceThread);
}
BOOL DBConnPool::IsNeedStop()
{
	return m_bNeedStop;
}
BOOL DBConnPool::IsNeedConnection()
{
	return m_bNeedConnection;
}
/************************************************************************/
/* ά���߳�                                                             */
/************************************************************************/
DWORD WINAPI DBConnPool::thread_run(LPVOID pdata)
{
	DBConnPool * pConPool = (DBConnPool *)pdata;
	while (!pConPool->IsNeedStop())
	{
		// �����¼�Ϊ���ź�, ���������Ƶȴ�
		ResetEvent(pConPool->m_hHaveData);
		WaitForSingleObject(pConPool->m_hHaveData, INFINITE);
		if (pConPool->IsNeedConnection())
		{
			//	g_pSvrLog->AddRunLog(LL_DEBUG, _T("Create a new DB connection."));
			pConPool->InitializeAConnection();
		}
	}
	return 0;
}
