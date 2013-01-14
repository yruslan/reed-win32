#pragma once


// CDlgCheckFiles dialog

class CDlgCheckFiles : public CDialog
{
	DECLARE_DYNAMIC(CDlgCheckFiles)

public:
	CDlgCheckFiles(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCheckFiles();

// Dialog Data
	enum { IDD = IDD_DLG_CHECK_FILES };

	CListCtrl	m_lstFiles;

	void UpdateList();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnQuickCheck();
	afx_msg void OnBnClickedBtnBack();
};
