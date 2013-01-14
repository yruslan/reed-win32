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
	, m_nSliderPercent(0)
{
	m_szFolder = _T("");
	m_szFiles = _T("");
	m_szSize = _T("");
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
	DDX_Slider(pDX, IDC_SLIDER_PERCENT, m_nSliderPercent);
	DDX_Control(pDX, IDC_SLIDER_PERCENT, m_cSliderPercent);
}


BEGIN_MESSAGE_MAP(CPPRecoverySize, CPropertyPage)
	ON_EN_KILLFOCUS(IDC_EDIT_REC_SIZE, &CPPRecoverySize::OnEnKillfocusEditRecSize)
	ON_EN_KILLFOCUS(IDC_EDIT_PERCENT_SIZE, &CPPRecoverySize::OnEnKillfocusEditPercentSize)
	ON_BN_CLICKED(IDC_BTN_RECOVER_PATH, &CPPRecoverySize::OnBnClickedBtnRecoverPath)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_PERCENT, &CPPRecoverySize::OnNMCustomdrawSliderPercent)
END_MESSAGE_MAP()


// CPPRecoverySize message handlers

BOOL CPPRecoverySize::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	
	m_cSliderPercent.SetRange(1, 50);
	m_nSliderPercent = 10;
	m_nRecSizeMB = int(double(g_Protector.m_nTotalSize)*(10.5)*0.01/double(1024*1024));
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

	if (g_Protector.m_nTotalSize>0)
	{
		CString szPercent;
		double percent = (double(m_nRecSizeMB)/(double(g_Protector.m_nTotalSize)/double(1024*1024)))*100.;
		int nPercent = int(percent+0.5);
		if (nPercent!=m_nSliderPercent)
		{
			if (nPercent>1 && nPercent<=50)
			{
				m_nSliderPercent = nPercent;
			}
			else
			{
				if (nPercent<=1)
					m_nSliderPercent = 1;
				if (nPercent>=50)
					m_nSliderPercent = 50;				
			}
		}
		szPercent.Format(_T("%d %%"), nPercent);
		GetDlgItem(IDC_STATIC_PERCENT)->SetWindowText(szPercent);
	}

	UpdateData(FALSE);
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

void CPPRecoverySize::OnNMCustomdrawSliderPercent(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*pResult = 0;

	if (g_Protector.m_nTotalSize>0)
	{
		UpdateData(TRUE);
		double percent = (double(m_nSliderPercent)+0.5)*0.01;
		m_nRecSizeMB = int((double(g_Protector.m_nTotalSize)*percent)/double(1024*1024));
		if (m_nRecSizeMB==0) m_nRecSizeMB=1;

		CString szPercent;
		szPercent.Format(_T("%d %%"), m_nSliderPercent);
		GetDlgItem(IDC_STATIC_PERCENT)->SetWindowText(szPercent);
		UpdateData(FALSE);		
	}
}