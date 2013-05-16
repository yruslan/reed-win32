/*
Copyright (c) 2009-2013, Ruslan Yushchenko
All rights reserved.

Distributed via 3-clause BSD (aka New BSD) license

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Ruslan Yushchenko nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL RUSLAN YUSHCHENKO BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "reed.h"
#include "utils.h"
#include "PPRecoverySize.h"
#include "DlgListFiles.h"

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
	m_pBoldFont.DeleteObject();
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
	ON_BN_CLICKED(IDC_BTN_LIST2, &CPPRecoverySize::OnBnClickedBtnList2)
END_MESSAGE_MAP()


// CPPRecoverySize message handlers

BOOL CPPRecoverySize::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//Setting up bold fonts for values
	LOGFONT logfont;
	GetDlgItem(IDC_STATIC_DIR)->GetFont()->GetLogFont(&logfont);
	logfont.lfWeight=FW_BOLD;
	m_pBoldFont.CreateFontIndirect(&logfont);

	GetDlgItem(IDC_STATIC_DIR)->SetFont(&m_pBoldFont, TRUE);
	GetDlgItem(IDC_STATIC_FILES)->SetFont(&m_pBoldFont, TRUE);
	GetDlgItem(IDC_STATIC_SIZE)->SetFont(&m_pBoldFont, TRUE);
	
	m_cSliderPercent.SetRange(1, 50);
	m_nSliderPercent = 10;
	m_nRecSizeMB = int(double(g_Protector.m_nTotalSize)*(10.5)*0.01/double(1024*1024));
	m_szFolder = g_Protector.m_szPath;
	if (m_szFolder!=_T(""))
		m_szRecFile = m_szFolder + "Directory.rcv";
	else
	{
		//Path of a file
		if (g_Protector.m_arFiles.GetSize()>0)
		{
			CString szFile0 = g_Protector.m_arFiles[0].szName;
			int pos = szFile0.ReverseFind('\\');
			CString szXPath=_T(".\\");
			if (pos>0)
				szXPath = szFile0.Left(pos+1);

			m_szFolder = szXPath;
		}

		if (g_Protector.m_arFiles.GetSize()==1)
			m_szRecFile = CString(g_Protector.m_arFiles[0].szName) + _T(".rcv");
		else
			m_szRecFile = m_szFolder + _T("Files.rcv");
	}

	UpdateData(FALSE);

	return TRUE;	
}

// CStylePage is the first wizard property page.  Disable the Back 
// button but enable the Next button.
BOOL CPPRecoverySize::OnSetActive() 
{
	m_szFiles.Format(_T("%d"), g_Protector.m_arFiles.GetSize());
	//m_szSize.Format(_T("%d MB"), int(double(g_Protector.m_nTotalSize)/double(1024*1024)));
	int nMB = int(double(g_Protector.m_nTotalSize)/double(1024*1024));
	HumanReadableMegabytes(nMB, m_szSize);
	
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

void CPPRecoverySize::OnBnClickedBtnList2()
{
	CDlgListFiles dlg;
	dlg.DoModal();
}
