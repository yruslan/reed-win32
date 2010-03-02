// DlgProgress.cpp : implementation file
//

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
