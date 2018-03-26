
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持

#include <iostream>
#include <string>

#include <msgpack.hpp>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace msgpack;
using namespace std;

typedef enum
{
	CMD_USER = 0X01,	// 用户表
	CMD_SIM,			// sim表
	CMD_KH,				// 客户表
	CMD_KHJL,			// 客户经理表
	CMD_LLC,			// 流量池表
	CMD_LLTC,			// 流量套餐表
	CMD_SSDQ			// 所属地区表
}CMD_MSG;

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
	SIM_NEWCARD_LEAD_IN,		// 导入新卡数据
	SIM_USECARD_LEAD_IN,		// 导入新卡分配后数据
	SIM_CANCELDATA_LEAD_IN,		// 导入注销清单
	SIM_RENEWDATE_LEAD_IN,		// 导入续费日期清单
}SUBCMD_SIM;





#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


