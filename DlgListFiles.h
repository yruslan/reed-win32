#pragma once


// CDlgListFiles dialog

class CDlgListFiles : public CDialog
{
	DECLARE_DYNAMIC(CDlgListFiles)

public:
	CDlgListFiles(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgListFiles();

	CListCtrl	m_lstFiles;

// Dialog Data
	enum { IDD = IDD_DLG_LIST };

	void UpdateList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
