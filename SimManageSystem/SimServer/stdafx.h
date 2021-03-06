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
#include <icrsint.h>
#include <process.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#import "C:\Program Files (x86)\Common Files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF", "adoBOF")

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
	CMD_SQLTEST,		// 测试sql
}CMD_MSG;

typedef enum
{
	SUBCMD_USER_ADD = 0X01,	// 用户注册
	SUBCMD_USER_GET_01,		// 使用用户名查询用户
	SUBCMD_USER_GET_02,		// 批量查询用户
	SUBCMD_USER_LOGIN,		// 用户登陆
	SUBCMD_USER_CHECK_UNAME,	// 检查用户名是否可用
}SUBCMD_USER;

typedef enum
{
	SUBCMD_SIM_ADD = 0X01,	// 增加sim卡
	SUBCMD_SIM_GET_01,		// 通过接入号码查询卡信息
	SUBCMD_SIM_GET_02,		// 批量查询卡信息
	SUBCMD_SIM_GET_ICCID,	// 通过ICCID查询卡信息
	SUBCMD_SIM_GET_XSRQ,	// 通过销售日期查询卡信息
	SUBCMD_SIM_GET_TOTAL,	// 获取sim卡总数
}SUBCMD_SIM;

typedef enum
{
	SUBCMD_KH_ADD = 0X01,	// 增加客户
	SUBCMD_KH_GET_01,		// 通过用户名查询客户
	SUBCMD_KH_GET_02,		// 批量查询客户信息
	SUBCMD_KH_MODIFY,		// 修改客户信息
}SUBCMD_KH;

									typedef enum
									{
										SSUBCMD_KH_TOTAL = 0X01,	// 卡信息
										SSUBCMD_KH_XSINFO,			// 销售信息
										SSUBCMD_KH_KHTOTAL,			// 客户信息总数
										SSUBCMD_KH_XSTOTAL,			// 销售信息总数
									}SSUBCMD_KH;

typedef enum
{
	SUBCMD_KHJL_ADD = 0X01,	// 增加客户经理
	SUBCMD_KHJL_GET_01,		// 通过客户经理名称查询
	SUBCMD_KHJL_GET_02,		// 批量查询客户经理信息
}SUBCMD_KHJL;

									typedef enum
									{
										SSUBCMD_KHJL_TOTAL = 0X01,	// 查询客户经理名下的卡数量
										SSUBCMD_KHJL_KHINFO,		// 查询客户经理名下的客户信息
										SSUBCMD_KHJL_XSINFO,		// 查询客户经理名下的销售记录
									}SSUBCMD_KHJL;

typedef enum
{
	SUBCMD_LLC_ADD = 0X01,	// 增加流量池
	SUBCMD_LLC_GET_01,		// 通过流量池号码查询
	SUBCMD_LLC_GET_02,		// 批量查询流量池信息
}SUBCMD_LLC;

typedef enum
{
	SUBCMD_LLTC_ADD = 0X01,	// 增加流量套餐
	SUBCMD_LLTC_GET_01,		// 通过套餐名称查询套餐信息
	SUBCMD_LLTC_GET_02,		// 批量查询套餐信息
}SUBCMD_LLTC;

typedef enum
{
	SUBCMD_SSDQ_ADD = 0X01,	// 增加所属地区
	SUBCMD_SSDQ_GET_01,		
	SUBCMD_SSDQ_GET_02,		// 批量查询所属地区
}SUBCMD_SSDQ;

typedef enum
{
	SUBCMD_IMPORT_NEWCARD = 0X01,	// 新卡导入
	SUBCMD_IMPORT_SALENOTE,			// 销售清单导入
	SUBCMD_IMPORT_KHSTATE,			// 客户状态导入
	SUBCMD_IMPORT_PAYLIST,			// 续费清单导入
	SUBCMD_IMPORT_CARDRETURNED,		// 退卡信息导入
	SUBCMD_IMPORT_CARDCANCEL,		// 注销卡信息导入
}SUBCMD_IMPORT;

typedef enum
{
	SUBCMD_SQL_TEST = 0X01
}SUBCMD_SQL;


#include <unordered_map>
#include <iostream>
#include <string>
using namespace std;
using namespace stdext;



typedef struct st_sim ST_SIM;
typedef struct st_kh ST_KH;
typedef struct st_khjl ST_KHJL;

typedef unordered_map<unsigned int, ST_SIM*> tmp_sim;
typedef unordered_map<unsigned int, ST_KH*> tmp_kh;
typedef unordered_map<unsigned int, ST_KHJL*> tmp_khjl;

struct st_sim
{
	unsigned int id;
	string strJrhm;
	string strIccid;
};

struct st_kh
{
	unsigned int id;
	string strKhmc;
	string strLxfs;
	string strJlxm;
	string strXgsj;
	tmp_sim mp_sim;
};

struct st_khjl
{
	unsigned int id;
	string strJlxm;
	string strLxfs;
	string strXgsj;
	tmp_kh mp_kh;
};