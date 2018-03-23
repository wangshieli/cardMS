#pragma once

#define WM_RELEASE_DBLINK WM_USER + 100

extern unsigned int nThreadID;

// ��ʼ�����ӳ���Ҫʹ�õ���Դ
void InitDBPool();

// ������С����������
void InitConnection(const int iMix, const int iMax);

// ��ȡvector��ʣ�������
_ConnectionPtr *GetConnectionPtr();

// ��ȡvector��ʣ������ӣ����ж�����״̬
_ConnectionPtr* GetTransConnection();

// ִ��sql���
bool ExcuteSql(const TCHAR* bSql, bool bCheck);

// ִ��sql��䣬�������ݼ�
bool GetRecordSet(const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/, bool bCheck);

// �ͷ���Դ
void ReleaseDBPool();

void ReleaseRecordset(_RecordsetPtr& pRecord);