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
#include "mfc_helpers.h"
#include "DlgCheckPath.h"
#include "DlgListFiles.h"

// CDlgCheckPath dialog

IMPLEMENT_DYNAMIC(CDlgCheckPath, CDialog)

CDlgCheckPath::CDlgCheckPath(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCheckPath::IDD, pParent)
	, m_szSrcPath(_T(""))
	, m_szOrigDir(_T(""))
	, m_szOrigCount(_T(""))
	, m_szOrigSize(_T(""))
{

}

CDlgCheckPath::~CDlgCheckPath()
{
	m_pBoldFont.DeleteObject();
}

void CDlgCheckPath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SRC_PATH, m_szSrcPath);
	DDX_Text(pDX, IDC_STATIC_DIR, m_szOrigDir);
	DDX_Text(pDX, IDC_STATIC_FILES, m_szOrigCount);
	DDX_Text(pDX, IDC_STATIC_SIZE, m_szOrigSize);
}


BEGIN_MESSAGE_MAP(CDlgCheckPath, CDialog)
	ON_BN_CLICKED(IDC_BTN_LIST, &CDlgCheckPath::OnBnClickedBtnList)
	ON_BN_CLICKED(IDC_BTN_SRC_PATH, &CDlgCheckPath::OnBnClickedBtnSrcPath)
END_MESSAGE_MAP()


// CDlgCheckPath message handlers

BOOL CDlgCheckPath::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Setting up bold fonts for values
	LOGFONT logfont;
	GetDlgItem(IDC_STATIC_DIR)->GetFont()->GetLogFont(&logfont);
	logfont.lfWeight=FW_BOLD;
	m_pBoldFont.CreateFontIndirect(&logfont);

	GetDlgItem(IDC_STATIC_DIR)->SetFont(&m_pBoldFont, TRUE);
	GetDlgItem(IDC_STATIC_FILES)->SetFont(&m_pBoldFont, TRUE);
	GetDlgItem(IDC_STATIC_SIZE)->SetFont(&m_pBoldFont, TRUE);


	if (g_Protector.m_nTotalSize>10000000)
	{
		int nOrigSize = (int)(g_Protector.m_nTotalSize/(1024*1024));
		HumanReadableMegabytes(nOrigSize, m_szOrigSize);
	}
	else
		NumbersToBytes((int)g_Protector.m_nTotalSize, 1, m_szOrigSize);

	//m_szOrigSize.Format(_T("%d MB"), nOrigSize);
	if (g_Protector.m_szPath!=_T(""))
	{
		m_szOrigDir = g_Protector.m_szPath;
		m_szSrcPath = m_szOrigDir;
	}
	else
		m_szOrigDir = _T("N/A");

	m_szOrigCount.Format(_T("%d"), g_Protector.m_arFiles.GetSize());


	CString szTmpPath = g_Protector.m_szPath;
	CString szRcvPath = g_Protector.m_szRecFileName;

	int pos = szRcvPath.ReverseFind('\\');
	CString szXPath = _T(".\\");
	if (pos > 0)
		szXPath = szRcvPath.Left(pos + 1);

/*	if (g_Protector.isSpecificPathCorrect(szXPath))
	{
		_tcscpy(g_Protector.m_szPath, szXPath);
		m_szSrcPath = szXPath;
	} else if (g_Protector.isOrigPathCorrect())
	{
		GetDlgItem(IDC_EDIT_SRC_PATH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_SRC_PATH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText(_T("The original path is correct. No need to specify a path"));
	}
	else
	{
		_tcscpy(g_Protector.m_szPath, szXPath);
		m_szSrcPath = szXPath;
	}*/

	_tcscpy(g_Protector.m_szPath, szXPath);
	m_szSrcPath = szXPath;

	UpdateData(FALSE);

	return TRUE;	
}

void CDlgCheckPath::OnBnClickedBtnList()
{
	CDlgListFiles dlg;
	dlg.DoModal();
}

void CDlgCheckPath::OnBnClickedBtnSrcPath()
{
	//Select a folder
	CString szPath;
	if (IDOK == MFC_FolderDialog(szPath,
		AfxGetApp()->GetMainWnd()->GetSafeHwnd(),
		_T("Protected Data Path"), m_szSrcPath, false)) 
	{
		m_szSrcPath = szPath;
		UpdateData(FALSE);
	}
}
