#pragma once
#include "afxcmn.h"


// CDlgProgress dialog

class CDlgProgress : public CDialog
{
	DECLARE_DYNAMIC(CDlgProgress)

public:
	CDlgProgress(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgProgress();

// Dialog Data
	enum { IDD = IDD_PROGRESS };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_szOperation;
	CString m_szFileName;
	bool m_bNeedTerminate;

	CFont m_fntBold;
	CFont m_fntBoldSmall;

	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnUpdateNow();

	CProgressCtrl m_cProgress;
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
