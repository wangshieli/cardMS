
// SimManageClientDlg.h : 头文件
//

#pragma once


// CSimManageClientDlg 对话框
class CSimManageClientDlg : public CDialogEx
{
// 构造
public:
	CSimManageClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SIMMANAGECLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLinkserver();
	CString m_strUsername;
	CString m_strPassword;
	afx_msg void OnBnClickedBtnUserregister();
	SOCKET s;

private:
	void DealLast(msgpack::sbuffer& sBuf);
	bool CheckLink();
public:
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnSelectbyname();
	afx_msg void OnBnClickedBtnSelectbytag();
	int m_iTag;
	CString m_strNewpassword;
	afx_msg void OnBnClickedBtnModifypassword();
	afx_msg void OnBnClickedBtnTest();
	int m_iCount;
	afx_msg void OnBnClickedBtnOne();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
