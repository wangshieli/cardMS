
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��

#include <iostream>
#include <string>

#include <msgpack.hpp>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using namespace msgpack;
using namespace std;

typedef enum
{
	CMD_USER = 0X01,	// �û���
	CMD_SIM,			// sim��
	CMD_KH,				// �ͻ���
	CMD_KHJL,			// �ͻ������
	CMD_LLC,			// �����ر�
	CMD_LLTC,			// �����ײͱ�
	CMD_SSDQ			// ����������
}CMD_MSG;

// ͨ��������
typedef enum
{
	SUBCMD_ADD = 0X01,		// ����������
	SUBCMD_MODIFY,			// �޸�����
	SUBCMD_SELECT_BY_KEY,	// ʹ��ָ��������Ϣ��ѯ
	SUBCMD_SELECT_BY_TAG	// ����������ѯ
}SUBCMD_MSG;

typedef enum
{
	USER_REGISTER = 0X01,		// �û�ע��
	USER_LOGIN,					// �û���½
	USER_SELECT_BY_USERNAME,	// ʹ���û�����ѯ
	USER_SELECT_BY_TAG,			// ����������ѯ
	USER_MODIFY_PASSWORD,		// �޸��û�����
	USER_MODIFY_PRIVILEGE,		// �޸��û�Ȩ��
}SUBCMD_USER;

// ���������
typedef enum
{
	SIM_SELECT_BY_ICCIC = 0X05,	// ʹ��ICCID��ѯ
	SIM_SELECT_BY_KH,			// �г��ͻ�����ϸ
	SIM_STOP,					// ͣ��
	SIM_START,					// ����
	SIM_STATE_UPDATE,			// ���¿�״̬
	SIM_NEWCARD_LEAD_IN,		// �����¿�����
	SIM_USECARD_LEAD_IN,		// �����¿����������
	SIM_CANCELDATA_LEAD_IN,		// ����ע���嵥
	SIM_RENEWDATE_LEAD_IN,		// �������������嵥
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


