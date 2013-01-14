#pragma once

// CDlgRecover dialog

class CDlgRecover : public CDialog
{
	DECLARE_DYNAMIC(CDlgRecover)

public:
	CDlgRecover(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRecover();

	CListCtrl	m_lstFiles;

// Dialog Data
	enum { IDD = IDD_DLG_CHECK_RECOVER };

	void UpdateList();

	CString m_szRecoverPath;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void InvalidateControls();

public:
	afx_msg void OnBnClickedBtnRecoverPath();
	BOOL m_bInPlace;
	BOOL m_bSubdirs;
	afx_msg void OnBnClickedChkInplace();
};
