#pragma once

#define WM_RELEASE_DBLINK WM_USER + 100

extern unsigned int nThreadID;

// 初始化连接池需要使用的资源
void InitDBPool();

// 创建最小数量的连接
void InitConnection(const int iMix, const int iMax);

// 获取vector中剩余的连接
_ConnectionPtr *GetConnectionPtr();

// 获取vector中剩余的连接，并判断连接状态
_ConnectionPtr* GetTransConnection();

// 执行sql语句
bool ExcuteSql(const TCHAR* bSql, bool bCheck);

// 执行sql语句，返回数据集
bool GetRecordSet(const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/, bool bCheck);

// 释放资源
void ReleaseDBPool();

void ReleaseRecordset(_RecordsetPtr& pRecord);

bool CreateDBConnection(_ConnectionPtr& conptr);
void ReleaseConnectionPtr(_ConnectionPtr& conptr);
bool ExcuteWithoutCheck(_ConnectionPtr& conptr, const TCHAR* bSql);
bool GetRecordSetWithoutCheck(_ConnectionPtr &conptr, const TCHAR* bSql, _RecordsetPtr& pRecord, long lOption /*= adCmdText*/);