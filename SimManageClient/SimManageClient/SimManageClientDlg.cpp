
// SimManageClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SimManageClient.h"
#include "SimManageClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSimManageClientDlg 对话框



CSimManageClientDlg::CSimManageClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSimManageClientDlg::IDD, pParent)
	, m_strUsername(_T(""))
	, m_strPassword(_T(""))
	, s(INVALID_SOCKET)
	, m_iTag(0)
	, m_strNewpassword(_T(""))
	, m_iCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSimManageClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUsername);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_EDIT_TAG, m_iTag);
	DDX_Text(pDX, IDC_EDIT_NEWPASSWORD, m_strNewpassword);
	DDX_Text(pDX, IDC_EDIT_COUNT, m_iCount);
}

BEGIN_MESSAGE_MAP(CSimManageClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LINKSERVER, &CSimManageClientDlg::OnBnClickedBtnLinkserver)
	ON_BN_CLICKED(IDC_BTN_USERREGISTER, &CSimManageClientDlg::OnBnClickedBtnUserregister)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CSimManageClientDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_SELECTBYNAME, &CSimManageClientDlg::OnBnClickedBtnSelectbyname)
	ON_BN_CLICKED(IDC_BTN_SELECTBYTAG, &CSimManageClientDlg::OnBnClickedBtnSelectbytag)
	ON_BN_CLICKED(IDC_BTN_MODIFYPASSWORD, &CSimManageClientDlg::OnBnClickedBtnModifypassword)
	ON_BN_CLICKED(IDC_BTN_TEST, &CSimManageClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_ONE, &CSimManageClientDlg::OnBnClickedBtnOne)
	ON_BN_CLICKED(IDC_BUTTON2, &CSimManageClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSimManageClientDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CSimManageClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON4, &CSimManageClientDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CSimManageClientDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CSimManageClientDlg 消息处理程序

BOOL CSimManageClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSimManageClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSimManageClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSimManageClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BYTE csum(unsigned char *addr, int count)
{
	BYTE sum = 0;
	for (int i = 0; i< count; i++)
	{
		sum += (BYTE)addr[i];
	}
	return sum;
}

bool CSimManageClientDlg::CheckLink()
{
	CString strErr;
	if (s == INVALID_SOCKET)
	{
		strErr.Format(_T("请先连接服务器"));
		return false;
	}

	return true;
}

void CSimManageClientDlg::DealLast(msgpack::sbuffer& sBuf)
{
	CString strErr;

	char* pCh = sBuf.data();
	int nLen = sBuf.size();
	BYTE pData[1024 * 10];
	memset(pData, 0, 1024 * 10);
	memcpy(pData, pCh + 6, nLen - 6);
	BYTE nSum = csum(pData, nLen - 6);
	sBuf.write("\x00", 1);
	memcpy(pCh + nLen, &nSum, 1);
	sBuf.write("\x0d", 1);
	nLen = sBuf.size();
	nLen -= 8;
	memcpy(pCh + 2, &nLen, 4);
	nLen += 8;

	send(s, pCh, nLen, 0);

	char pRequest[1024] = { 0 };
	nLen = 0;
	nLen = recv(s, pRequest, 1024, 0);

	if (nLen < 8)
	{
		strErr.Format(_T("接收到的数据长度< 8"));
		MessageBox(strErr);
		return;
	}
	if ((UCHAR)pRequest[0] != 0xfb || (UCHAR)pRequest[1] != 0xfc)//  没有数据开始标志
	{
		strErr.Format(_T("接收到的数据没有开始标志"));
		MessageBox(strErr);
		return;
	}
	int nFrameLen = *(INT*)(pRequest + 2);
	if (nLen < (nFrameLen + 8))
	{
		strErr.Format(_T("接收到的数据长度错误"));
		MessageBox(strErr);
		return;
	}
	nSum = pRequest[6 + nFrameLen];// 检验和
	if (nSum != csum((unsigned char*)pRequest + 6, nFrameLen))
	{
		strErr.Format(_T("接收到的数据校验和错误"));
		MessageBox(strErr);
		return;
	}

	if (0x0d != pRequest[nFrameLen + 7])
	{
		strErr.Format(_T("接收到的数据结尾错误"));
		MessageBox(strErr);
		return;
	}

	try
	{
		msgpack::unpacker unpack_;
		msgpack::object_handle result_;
		unpack_.reserve_buffer(nLen);
		memcpy_s(unpack_.buffer(), nLen, pRequest + 6, nFrameLen);
		unpack_.buffer_consumed(nFrameLen);
		unpack_.next(result_);
		//msgpack::object ddd = result_.get();
		int nSize = result_.get().via.array.size;
		msgpack::object* pObj = result_.get().via.array.ptr;
		if (msgpack::type::ARRAY != result_.get().type)
		{
			strErr.Format(_T("接收到的数据不是ARRAY"));
			MessageBox(strErr);
			return;
		}
		int cmd = (pObj++)->as<int>();
		int subcmd = (pObj++)->as<int>();
		int res = (pObj++)->as<int>();
		strErr.Format(_T("[%d,%d,%d]"), cmd, subcmd, res);
		MessageBox(strErr);
	}
	catch (...)
	{
		strErr.Format(_T("处理中出现异常"));
		MessageBox(strErr);
		return;
	}
}

void CSimManageClientDlg::OnBnClickedBtnLinkserver()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strErr;
	s = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sAddr;
	ZeroMemory(&sAddr, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(6088);
	sAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == connect(s, (sockaddr*)&sAddr, sizeof(sAddr)))
	{
		strErr.Format(_T("连接服务器失败:%d"), WSAGetLastError());
		MessageBox(strErr);
		return;
	}
}


void CSimManageClientDlg::OnBnClickedBtnUserregister()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack((int)CMD_USER);
	msgPack.pack((int)USER_REGISTER);
	msgPack.pack(m_strUsername.GetBuffer(0));
	msgPack.pack(m_strPassword.GetBuffer(0));
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnLogin()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack((int)CMD_USER);
	msgPack.pack((int)USER_LOGIN);
	msgPack.pack(m_strUsername.GetBuffer(0));
	msgPack.pack(m_strPassword.GetBuffer(0));
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnSelectbyname()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack((int)CMD_USER);
	msgPack.pack((int)USER_SELECT_BY_USERNAME);
	msgPack.pack(m_strUsername.GetBuffer(0));
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnSelectbytag()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(3);
	msgPack.pack((int)CMD_USER);
	msgPack.pack((int)USER_SELECT_BY_TAG);
	msgPack.pack(m_iTag);
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnModifypassword()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(5);
	msgPack.pack((int)CMD_USER);
	msgPack.pack((int)USER_MODIFY_PASSWORD);
	msgPack.pack(m_strUsername.GetBuffer(0));
	msgPack.pack(m_strPassword.GetBuffer(0));
	msgPack.pack(m_strNewpassword.GetBuffer(0));
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;
	/*for (int i = 0; i < 20; i++)
	{
		sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(6 + nCountOfData);
	msgPack.pack((int)CMD_SIM);
	msgPack.pack((int)SIM_LEAD_IN);
	msgPack.pack(m_iTag);
	msgPack.pack(nCountOfData);
	msgPack.pack("98654123");
	msgPack.pack("后向");
	for (int i = 0; i < nCountOfData; i++)
	{
		msgPack.pack_array(3);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		CString strIccid;
		strIccid.Format("iccid_%d", ++m);
		msgPack.pack(strIccid.GetBuffer(0));
		msgPack.pack("中国电信");
	}

	DealLast(sbuf);
	}*/
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(6);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_NEWCARD);
	msgPack.pack(m_iTag);
	msgPack.pack("123456789");
	msgPack.pack("后向");
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(3);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		CString strIccid;
		strIccid.Format("iccid_%d", ++m);
		msgPack.pack(strIccid.GetBuffer(0));
		msgPack.pack("中国电信");
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedBtnOne()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(6);
	msgPack.pack((int)CMD_SIM);
	msgPack.pack((int)SUBCMD_ADD);
	msgPack.pack("jrhm_1");
	msgPack.pack("iccid_1");
	msgPack.pack("中国电信");
	msgPack.pack("123456789");
	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;

	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_KHSTATE);
	msgPack.pack(m_iTag);
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(3);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		msgPack.pack("2018-03-26");// 开通日期
		msgPack.pack("在用");
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;

	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_CARDCANCEL);
	msgPack.pack(m_iTag);
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(3);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		msgPack.pack("2018-03-26");
		msgPack.pack("注销");
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;

	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(4);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_PAYLIST);
	msgPack.pack(m_iTag);
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(3);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		msgPack.pack("2019-04-26");// 续费日期
		msgPack.pack(2);// 到期日期
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;

	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(5);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_CARDRETURNED);
	msgPack.pack(m_iTag);
	msgPack.pack("客户01");
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(1);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));	
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}


void CSimManageClientDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedBtnLinkserver();
	UpdateData();
	int n = 0;
	int m = 0;

	sbuffer sbuf;
	packer<sbuffer> msgPack(&sbuf);
	sbuf.write("\xfb\xfc", 6);
	msgPack.pack_array(5);
	msgPack.pack((int)CMD_IMPORT);
	msgPack.pack((int)SUBCMD_IMPORT_SALENOTE);
	msgPack.pack(m_iTag);
	msgPack.pack("客户01");
	msgPack.pack_array(m_iCount);
	for (int i = 0; i < m_iCount; i++)
	{
		msgPack.pack_array(4);
		CString strJrhm;
		strJrhm.Format("jrhm_%d", ++n);
		msgPack.pack(strJrhm.GetBuffer(0));
		//msgPack.pack(1000);// 拿卡数量
		msgPack.pack("2019-03-27");// 销售日期
		msgPack.pack("销售员");
		msgPack.pack("备注");
	}

	DealLast(sbuf);
	closesocket(s);
	s = INVALID_SOCKET;
}
