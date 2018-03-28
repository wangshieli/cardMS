#pragma once

byte csum(unsigned char *addr, int count);

void DealLast(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);

void AddData(const _variant_t& var, msgpack::packer<msgpack::sbuffer>& msgPack);

void AddYYYYMMDD(const _variant_t& var, msgpack::packer<msgpack::sbuffer>& msgPack);

void ReturnSimpleInfo(msgpack::packer<msgpack::sbuffer>& msgPack, int nCmd, int nSubCmd, int nSuccess);

void ReturnSimpleInfo(msgpack::packer<msgpack::sbuffer>& msgPack, int nCmd, int nSubCmd, int nTag, int nSuccess);

bool GetRecordSetDate(const TCHAR* sql, _RecordsetPtr& pRecord, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack);

bool GetRecordSetDate(const TCHAR* sql, _RecordsetPtr& pRecord, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack, bool bGlag);

void CheckSqlResult(const TCHAR* sql, int nCmd, int nSubCmd, msgpack::packer<msgpack::sbuffer>& msgPack);

bool DoTrans(int nCount, ...);