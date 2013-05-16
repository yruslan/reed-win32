/*
Copyright (c) 2009-2013, Ruslan Yushchenko
All rights reserved.

Distributed via 3-clause BSD (aka new BSD) license

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
#include "DlgProgress.h"


// CDlgProgress dialog

IMPLEMENT_DYNAMIC(CDlgProgress, CDialog)

CDlgProgress::CDlgProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgProgress::IDD, pParent)
	, m_szOperation(_T(""))
	, m_szFileName(_T(""))
{

}

CDlgProgress::~CDlgProgress()
{
}

void CDlgProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OPERATION, m_szOperation);
	DDX_Text(pDX, IDC_FILENAME, m_szFileName);
	DDX_Control(pDX, IDC_PROGRESS1, m_cProgress);
}


BEGIN_MESSAGE_MAP(CDlgProgress, CDialog)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CDlgProgress::OnBnClickedBtnCancel)
	ON_COMMAND(IDC_CUSTOM_UPDATE, &CDlgProgress::OnUpdateNow)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CDlgProgress message handlers

void CDlgProgress::OnBnClickedBtnCancel()
{
	m_bNeedTerminate = true;
}

void CDlgProgress::OnUpdateNow()
{
	UpdateData(FALSE);
}

BOOL CDlgProgress::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LOGFONT logfont;
	GetDlgItem(IDC_OPERATION)->GetFont()->GetLogFont(&logfont);
	logfont.lfWeight=FW_BOLD;
	m_fntBoldSmall.CreateFontIndirect(&logfont);
	logfont.lfHeight=(LONG)(double(logfont.lfHeight)*1.3);
	logfont.lfWidth*=(LONG)(double(logfont.lfWidth)*1.3);
	m_fntBold.CreateFontIndirect(&logfont);
	GetDlgItem(IDC_OPERATION)->SetFont(&m_fntBold, TRUE);
	GetDlgItem(IDC_FILENAME)->SetFont(&m_fntBoldSmall, TRUE);
	//m_szOperation="Loading Recovery Info...";
	//m_szFileName="C:\ASUS\Directory.rcv";
	UpdateData(FALSE);

	return TRUE;
}

BOOL CDlgProgress::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_APPSTARTING));	
	return TRUE;
	//return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
