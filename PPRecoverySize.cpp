// PPRecoverySize.cpp : implementation file
//

#include "stdafx.h"
#include "reed.h"
#include "PPRecoverySize.h"


// CPPRecoverySize dialog

IMPLEMENT_DYNAMIC(CPPRecoverySize, CPropertyPage)

CPPRecoverySize::CPPRecoverySize()
	: CPropertyPage(CPPRecoverySize::IDD)
	, m_szRecFile(_T(""))
	, m_nRecSizeMB(100)
	, m_nRecSizePercent(10)
{
	m_szFolder = _T("");
	m_szFiles = _T("");
	m_szSize = _T("");
	m_nRadio = 0;
	m_nRecSizePercent = 10;
	m_nRecSizeMB = 100;
}

CPPRecoverySize::~CPPRecoverySize()
{
}

void CPPRecoverySize::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_DIR, m_szFolder);
	DDX_Text(pDX, IDC_STATIC_FILES, m_szFiles);
	DDX_Text(pDX, IDC_STATIC_SIZE, m_szSize);
	DDX_Text(pDX, IDC_EDIT_RECOVER_PATH2, m_szRecFile);
	DDX_Text(pDX, IDC_EDIT_REC_SIZE, m_nRecSizeMB);
	DDX_Text(pDX, IDC_EDIT_PERCENT_SIZE, m_nRecSizePercent);
	DDX_Radio(pDX, IDC_RADIO1, m_nRadio);
}


BEGIN_MESSAGE_MAP(CPPRecoverySize, CPropertyPage)
	ON_BN_CLICKED(IDC_RADIO1, &CPPRecoverySize::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPPRecoverySize::OnBnClickedRadio2)
	ON_EN_KILLFOCUS(IDC_EDIT_REC_SIZE, &CPPRecoverySize::OnEnKillfocusEditRecSize)
	ON_EN_KILLFOCUS(IDC_EDIT_PERCENT_SIZE, &CPPRecoverySize::OnEnKillfocusEditPercentSize)
	ON_BN_CLICKED(IDC_BTN_RECOVER_PATH, &CPPRecoverySize::OnBnClickedBtnRecoverPath)
END_MESSAGE_MAP()


// CPPRecoverySize message handlers

BOOL CPPRecoverySize::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	m_nRecSizeMB = int(double(g_Protector.m_nTotalSize)*(double(m_nRecSizePercent)+0.5)*0.01/double(1024*1024));
	m_szFolder = g_Protector.m_szPath;
	m_szRecFile = m_szFolder + "Directory.rcv";

	UpdateData(FALSE);

	return TRUE;	
}

// CStylePage is the first wizard property page.  Disable the Back 
// button but enable the Next button.
BOOL CPPRecoverySize::OnSetActive() 
{
	m_szFiles.Format(_T("%d"), g_Protector.m_arFiles.GetSize());
	m_szSize.Format(_T("%d MB"), int(double(g_Protector.m_nTotalSize)/double(1024*1024)));
	
	UpdateData(FALSE);

	InvalidateControls();
	return CPropertyPage::OnSetActive();
}

void CPPRecoverySize::InvalidateControls()
{
	UpdateData(TRUE);

	CPropertySheet* psheet = (CPropertySheet*) GetParent();   

	if (g_Protector.m_arFiles.GetSize()==0)
		psheet->SetWizardButtons(PSWIZB_DISABLEDFINISH);
	else
		psheet->SetWizardButtons(PSWIZB_FINISH);

	if (m_nRadio==0)
	{
		GetDlgItem(IDC_EDIT_REC_SIZE)->EnableWindow(TRUE);
		GetDlgItem(IDC_SLIDER_PERCENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PERCENT_SIZE)->EnableWindow(FALSE);

		if (g_Protector.m_nTotalSize>0)
		{
			double percent = (double(m_nRecSizeMB)/(double(g_Protector.m_nTotalSize)/double(1024*1024)))*100.;
			m_nRecSizePercent = int(percent+0.5);
		}
	}
	else
	{
		GetDlgItem(IDC_EDIT_REC_SIZE)->EnableWindow(FALSE);
		GetDlgItem(IDC_SLIDER_PERCENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PERCENT_SIZE)->EnableWindow(FALSE);

		if (m_nRecSizePercent>50) m_nRecSizePercent = 50;

		if (g_Protector.m_nTotalSize>0)
		{
			double percent = (double(m_nRecSizePercent)+0.5)*0.01;
			m_nRecSizeMB = int((double(g_Protector.m_nTotalSize)*percent)/double(1024*1024));
			if (m_nRecSizeMB==0) m_nRecSizeMB=1;
		}
	}
	UpdateData(FALSE);
}


void CPPRecoverySize::OnBnClickedRadio1()
{
	InvalidateControls();
}

void CPPRecoverySize::OnBnClickedRadio2()
{
	InvalidateControls();
}

void CPPRecoverySize::OnEnKillfocusEditRecSize()
{
	InvalidateControls();
}

void CPPRecoverySize::OnEnKillfocusEditPercentSize()
{
	InvalidateControls();
}

void CPPRecoverySize::OnBnClickedBtnRecoverPath()
{
	UpdateData(TRUE);
	
	CFileDialog dlg(FALSE, _T("rcv"), (m_szRecFile==_T("") ? NULL : m_szRecFile.GetBuffer(1)), OFN_OVERWRITEPROMPT, _T("Recovery Info Files|*.rcv|All Files(*.*)|*.*||"));
	m_szRecFile.ReleaseBuffer();
	if ( dlg.DoModal() != IDOK )
		return;
	
	m_szRecFile = dlg.GetPathName();
	UpdateData(FALSE);

}
