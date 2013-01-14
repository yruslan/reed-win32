#pragma once


// CDlgCheckPath dialog

class CDlgCheckPath : public CDialog
{
	DECLARE_DYNAMIC(CDlgCheckPath)

public:
	CDlgCheckPath(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCheckPath();

// Dialog Data
	enum { IDD = IDD_DLG_CHECK_PATHS };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CFont m_pBoldFont;

public:
	CString m_szSrcPath;
	CString m_szOrigDir;
	CString m_szOrigCount;
	CString m_szOrigSize;

	afx_msg void OnBnClickedBtnList();
	afx_msg void OnBnClickedBtnSrcPath();
};
