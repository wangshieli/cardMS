// DBConnPool.h: interface for the DBConnPool class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_DBCONNPOOL_H__42089E9A_FD85_4DD4_A973_64A4980332A5__INCLUDED_)
#define AFX_DBCONNPOOL_H__42089E9A_FD85_4DD4_A973_64A4980332A5__INCLUDED_
#define  SQL_BUFFER_LEN 1024
#import "C:/Program Files/Common Files/System/ado/msado15.dll" rename("EOF","ADOEOF")
#include <list>
#include <windows.h>
#include <string>
#include <vector>
#include  "DBDefine.h"
using namespace ADODB;
using namespace std;
class RFIDAPI DBConnect
{
public:
	DBConnect(LPCSTR strDstAddress,
		LPCSTR strUsername,
		LPCSTR strPassword,
		LPCSTR strDBName,
		BOOL &state);
	~DBConnect();
public:
	// ���ӵ����ݿ�

	int Open(LPCTSTR strDstAddress, LPCTSTR strUsername, LPCTSTR strPassword, LPCTSTR strDBName);
	// �ر����ݿ�

	int Close();
	// ���ݿ��Ƿ�������

	BOOL IsOpen() const;
public:
private:
	ADODB::_ConnectionPtr           _connection_ptr;   //ADO�����ݿ���������ָ��
	bool                            _isAdoValid;       //ADO�����Ƿ��Ѿ������ɹ���־��
	bool							m_bDBOpen;

	LPCSTR							_strDstAddress;    //����Դ��ַ�������
	LPCSTR							_strUsername;      //���ݿ��û���
	LPCSTR							_strPassword;      //���ݿ�����
	LPCSTR							_strDBName;        //���ݿ�����
	void         VarientToString(_variant_t var, string& str);
	//���⹫���ӿ�
public:
	int          GetSubmitInfo(vector<SOAP_SUBMIT_SMS> &vecsoapSms);

	int          InsertHistory(int id);
	int          UserLogin(LPCSTR name, LPCSTR password);
};
typedef std::list<DBConnect*> DBConnectList;
class  DBConnPool
{
public:
	DBConnPool();
	virtual ~DBConnPool();
	// ��ȡʵ��ָ��
	static DBConnPool * Instanse();
	// ��ʼ����������
	int InitializeAllDBConnections();

	// �ر���������
	void DestroyAllDBConnections();
	// ��ȡһ����������
	DBConnect* GetAConnection();
	// �������Ӹ����ж���
	int RestoreAConnection(DBConnect* pDBEngine);
	void SetDBInfo(LPCSTR strDstAddress, LPCSTR strUsername, LPCSTR strPassword, LPCSTR strDBName, int minConn, int maxConn);
private:

	// ����һ������
	int InitializeAConnection();
	// �ر�һ������
	void CloseAConnection(DBConnect* pDBEngine);
	// ֹͣ�����߳�
	void StopThread();

	// �ж��Ƿ���Ҫֹͣ
	BOOL IsNeedStop();
	BOOL IsNeedConnection();
	// ����������Ϊ���ӳ������Ԫ��
	friend class ConnGuard;
	// Ψһʵ��
	static DBConnPool *m_pInstanse;

	// �������ݿ����Ӷ���
	DBConnectList m_listIdleConnection;
	// ��ʹ�õ����ݿ�����
	DBConnectList m_listBusyConnection;

	// ���б������ٽ���
	CRITICAL_SECTION m_csIdleConnList;
	CRITICAL_SECTION m_csBusyConnList;

	// ������������������ָ�꣺�����С
	int m_nMaxCount;
	int m_nMinCount;

	// 	// ���ݿ���Ϣ
	LPCSTR							_strDstAddress;    //����Դ��ַ�������
	LPCSTR							_strUsername;      //���ݿ��û���
	LPCSTR							_strPassword;      //���ݿ�����
	LPCSTR							_strDBName;        //���ݿ�����

													   // ά���߳�
	HANDLE m_hMaintanceThread; // �߳̾��
	HANDLE m_hHaveData; // �ź�

	BOOL m_bNeedStop; // �����߳���ͣ�ı�־λ
	BOOL m_bNeedConnection; // ��Ҫ�������ӵı�־
	static	DWORD	WINAPI	thread_run(LPVOID pdata);
};
// �����࣬���ù��������������֤����ȡ���͹黹����ɶԣ���ֹ��Դй¶
class  DBConnGuard
{
public:
	DBConnGuard(DBConnect*& DBConn)
	{
		DBConn = DBConnPool::Instanse()->GetAConnection();
		m_pDBConn = DBConn;
	}
	virtual ~DBConnGuard()
	{
		DBConnPool::Instanse()->RestoreAConnection(m_pDBConn);
	}
private:
	DBConnect * m_pDBConn;
};
RFIDAPI void  InitDBIterface(LPCSTR strDstAddress, LPCSTR strUsername, LPCSTR strPassword, LPCSTR strDBName, int minConn, int maxConn);
RFIDAPI DBConnect * GetAConnect();
#endif // !defined(AFX_DBCONNPOOL_H__42089E9A_FD85_4DD4_A973_64A4980332A5__INCLUDED_)
