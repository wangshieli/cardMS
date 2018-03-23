// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <process.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define MSG_USER_0X0A	0X0A
#define MSG_SIM_0X0B	0X0B
#define MSG_KH_0X0C		0X0C
#define MSG_KHJL_0X0E	0X0E
#define MSG_LLC_0X09	0X09
#define MSG_LLTC_0X08	0X08
#define MSG_SSDQ_0X07	0X07

#define B_MSG_USER_0XAB	0XAB
#define B_MSG_SIM_0XBB	0XBB
#define B_MSG_KH_0XCB	0XCB
#define B_MSG_KHJL_0XEB	0XEB
#define B_MSG_LLC_0X9B	0X9B
#define B_MSG_LLTC_0X8B	0X8B
#define B_MSG_SSDQ_0X7B	0X7B

#define DO_INSERT_DATA		0X01
#define DO_SELECT_BY_KEY	0X02
#define DO_SELECT_BY_ID		0X03
#define DO_UPDATE_DATA		0X04
#define DO_SELECT_BY_USER	0X05 // 根据用户名，查询sim卡信息
#define DO_LOGIN			0X06
#define DO_MODIFY_PASSWORD	0X07

#include <icrsint.h>

#import "C:\Program Files (x86)\Common Files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF", "adoBOF")
