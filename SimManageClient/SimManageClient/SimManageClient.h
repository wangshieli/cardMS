
// SimManageClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSimManageClientApp:
// �йش����ʵ�֣������ SimManageClient.cpp
//

class CSimManageClientApp : public CWinApp
{
public:
	CSimManageClientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSimManageClientApp theApp;