#pragma once


// CPPRecoverySize dialog
#include "reed.h"

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

public:
	int m_nRadio;

	void InvalidateControls();

	CString m_szFolder;
	CString m_szFiles;
	CString m_szSize;
	CString m_szRecFile;
	int m_nRecSizeMB;
	int m_nRecSizePercent;

	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnEnKillfocusEditRecSize();
	afx_msg void OnEnKillfocusEditPercentSize();
	afx_msg void OnBnClickedBtnRecoverPath();
};
