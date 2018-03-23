#pragma once

byte csum(unsigned char *addr, int count);

void DealLast(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);

void AddData(const _variant_t& var, msgpack::packer<msgpack::sbuffer>& msgPack);

void ReturnSimpleInfo(msgpack::packer<msgpack::sbuffer>& msgPack, int nCmd, int nSubCmd, int nSuccess, const TCHAR* pErrInfo);

bool GetRecordSetDate(const TCHAR* sql, _RecordsetPtr& pRecord, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack);

void CheckSqlResult(const TCHAR* sql, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack);