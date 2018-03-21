#pragma once

byte csum(unsigned char *addr, int count);

void DealLast(msgpack::sbuffer& sBuf, BUFFER_OBJ* bobj);