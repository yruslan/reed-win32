// DlgCheckPath.cpp : implementation file
//

#include "stdafx.h"
#include "reed.h"
#include "utils.h"
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

	int nOrigSize = (int)(g_Protector.m_nTotalSize/(1024*1024));
	HumanReadibleMegabytes(nOrigSize, m_szOrigSize);
	//m_szOrigSize.Format(_T("%d MB"), nOrigSize);
	if (g_Protector.m_szPath!=_T(""))
	{
		m_szOrigDir = g_Protector.m_szPath;
		m_szSrcPath = m_szOrigDir;
	}
	else
		m_szOrigDir = _T("N/A");

	m_szOrigCount.Format(_T("%d"), g_Protector.m_arFiles.GetSize());

	if (g_Protector.isOrigPathCorrect())
	{
		GetDlgItem(IDC_EDIT_SRC_PATH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_SRC_PATH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText(_T("The original path is correct. No need to specify a path"));
	}
	else
	{
		CString szTmpPath = g_Protector.m_szPath;
		CString szRcvPath = g_Protector.m_szRecFileName;

		int pos = szRcvPath.ReverseFind('\\');
		CString szXPath=_T(".\\");
		if (pos>0)
			szXPath = szRcvPath.Left(pos+1);
		//{
		//	SetCurrentDirectory(szXPath);
		//}
		_tcscpy(g_Protector.m_szPath, szXPath);
		if (g_Protector.isOrigPathCorrect())
			m_szSrcPath = szXPath;
		else
			m_szSrcPath = _T("");
		_tcscpy(g_Protector.m_szPath, szTmpPath);
	}

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
	UpdateData(TRUE);
	BROWSEINFO info;
	info.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = _T("Protected Data Path");
	info.ulFlags = 0;
	info.lpfn = NULL;
	info.lParam = NULL;
	info.iImage = 0;
	LPITEMIDLIST lpH = SHBrowseForFolder(&info);
	if(!lpH) return;
	CString szFolder;
	if(SHGetPathFromIDList(lpH,szBuf)) 
	{
		szFolder = szBuf;
	}
	IMalloc *pMalloc = NULL;
	if(SUCCEEDED(SHGetMalloc(&pMalloc))&&pMalloc)
	{
		pMalloc->Free(lpH); pMalloc->Release();
	}

	if (szFolder==_T("")) return;

	m_szSrcPath = szFolder;
	UpdateData(FALSE);
}
