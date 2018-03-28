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
#define DO_DEVICE_STOP		0X08
#define DO_DEVICE_START		0X09
#define DO_DEVICE_UPDATE	0X10

typedef enum
{
	CMD_USER = 0X01,	// 用户表
	CMD_SIM,			// sim表
	CMD_KH,				// 客户表
	CMD_KHJL,			// 客户经理表
	CMD_LLC,			// 流量池表
	CMD_LLTC,			// 流量套餐表
	CMD_SSDQ,			// 所属地区表
	CMD_IMPORT,			// 数据导入
}CMD_MSG;

typedef enum
{
	SUBCMD_IMPORT_NEWCARD = 0X01,	// 新卡导入
	SUBCMD_IMPORT_SALENOTE,			// 销售清单导入
	SUBCMD_IMPORT_KHSTATE,			// 客户状态导入
	SUBCMD_IMPORT_PAYLIST,			// 续费清单导入
	SUBCMD_IMPORT_CARDRETURNED,		// 退卡信息导入
	SUBCMD_IMPORT_CARDCANCEL,		// 注销卡信息导入
}SUBCMD_IMPORT;

// 通用子命令
typedef enum
{
	SUBCMD_ADD = 0X01,		// 增加新数据
	SUBCMD_MODIFY,			// 修改数据
	SUBCMD_SELECT_BY_KEY,	// 使用指定主键信息查询
	SUBCMD_SELECT_BY_TAG	// 按此批量查询
}SUBCMD_MSG;

typedef enum
{
	USER_REGISTER = 0X01,		// 用户注册
	USER_LOGIN,					// 用户登陆
	USER_SELECT_BY_USERNAME,	// 使用用户名查询
	USER_SELECT_BY_TAG,			// 按此批量查询
	USER_MODIFY_PASSWORD,		// 修改用户密码
	USER_MODIFY_PRIVILEGE,		// 修改用户权限
}SUBCMD_USER;

// 针对子命令
typedef enum
{
	SIM_SELECT_BY_ICCIC = 0X05,	// 使用ICCID查询
	SIM_SELECT_BY_KH,			// 列出客户卡明细
	SIM_STOP,					// 停机
	SIM_START,					// 复机
	SIM_STATE_UPDATE,			// 更新卡状态
}SUBCMD_SIM;

#include <icrsint.h>

#import "C:\Program Files (x86)\Common Files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF", "adoBOF")
