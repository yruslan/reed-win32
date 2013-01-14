#pragma once


// CPPRecoverySize dialog
#include "reed.h"
#include "afxcmn.h"

class CPPRecoverySize : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPRecoverySize)

public:
	CPPRecoverySize();
	virtual ~CPPRecoverySize();

// Dialog Data
	enum { IDD = IDD_PP_PROTECT_SIZE };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()

	CFont m_pBoldFont;

public:
	void InvalidateControls();

	CString m_szFolder;
	CString m_szFiles;
	CString m_szSize;
	CString m_szRecFile;
	int m_nRecSizeMB;
	int m_nSliderPercent;
	CSliderCtrl m_cSliderPercent;

	afx_msg void OnEnKillfocusEditRecSize();
	afx_msg void OnEnKillfocusEditPercentSize();
	afx_msg void OnBnClickedBtnRecoverPath();
	afx_msg void OnNMCustomdrawSliderPercent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnList2();
};
