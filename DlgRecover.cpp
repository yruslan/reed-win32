// DlgRecover.cpp : implementation file
//

#include "stdafx.h"
#include "reed.h"
#include "utils.h"
#include "DlgRecover.h"


// CDlgRecover dialog

IMPLEMENT_DYNAMIC(CDlgRecover, CDialog)

CDlgRecover::CDlgRecover(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRecover::IDD, pParent)
	, m_bInPlace(FALSE)
	, m_bSubdirs(TRUE)
{

}

CDlgRecover::~CDlgRecover()
{
}

void CDlgRecover::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILES, m_lstFiles);
	DDX_Text(pDX, IDC_EDIT_RECOVER_PATH2, m_szRecoverPath);
	DDX_Check(pDX, IDC_CHK_INPLACE, m_bInPlace);
	DDX_Check(pDX, IDC_CHK_SUBDIRS, m_bSubdirs);
}


BEGIN_MESSAGE_MAP(CDlgRecover, CDialog)
	ON_BN_CLICKED(IDC_BTN_RECOVER_PATH, &CDlgRecover::OnBnClickedBtnRecoverPath)
	ON_BN_CLICKED(IDC_CHK_INPLACE, &CDlgRecover::OnBnClickedChkInplace)
END_MESSAGE_MAP()


// CDlgRecover message handlers
BOOL CDlgRecover::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString szRcvPath = g_Protector.m_szRecFileName;

	int pos = szRcvPath.ReverseFind('\\');
	CString szXPath=_T(".\\");
	if (pos>0)
		szXPath = szRcvPath.Left(pos+1);

	m_szRecoverPath = szXPath;

	CClientDC dc((CWnd *)&m_lstFiles);
	CRect rc;
	m_lstFiles.GetClientRect(rc);
	rc.DeflateRect( 1, 1, 1, 1 );

	CSize sz; 
	sz = dc.GetTextExtent(_T(" File Name (no path) - test ")); 
	m_lstFiles.InsertColumn(0, _T("File Name"), LVCFMT_CENTER, sz.cx); 

	sz = dc.GetTextExtent(_T(" 1002003 MB ")); 
	m_lstFiles.InsertColumn(1, _T("Damage"), LVCFMT_RIGHT, sz.cx); 

	sz = dc.GetTextExtent(_T(" Size Mismatch")); 
	m_lstFiles.InsertColumn(2, _T("Status"), LVCFMT_CENTER, sz.cx); 

	UpdateList();

	UpdateData(FALSE);

	if (g_Protector.m_nCntRecoverable>0)
	{
		if (g_Protector.m_nCntNotRecoverable==0)
		{
			CString s1;
			NumbersToBytes(g_Protector.m_nCntRecoverable, g_Protector.m_nRecoveryBlockSize, s1);
			CString msg;
			msg.Format(_T("Found %s of incorrect data. All recovarable!"), s1);
			GetDlgItem(IDC_STATIC_MSG)->SetWindowText(msg);
			//MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
		}
		else
		{
			CString s1;
			NumbersToBytes(g_Protector.m_nCntRecoverable, g_Protector.m_nRecoveryBlockSize, s1);
			CString s2;
			NumbersToBytes(g_Protector.m_nCntNotRecoverable, g_Protector.m_nRecoveryBlockSize, s2);
			CString msg;
			msg.Format(_T("Found %s recovarable and %s unrecoverable data!"), s1, s2);
			GetDlgItem(IDC_STATIC_MSG)->SetWindowText(msg);
			//MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
		}
	}
	else
	{
		CString msg;
		msg.Format(_T("Incorrect data too big. Cannot recover. Sorry!"));
		GetDlgItem(IDC_STATIC_MSG)->SetWindowText(msg);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		//MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
	}

	InvalidateControls();
	
	return TRUE;
}

void CDlgRecover::UpdateList()
{
	if ( m_lstFiles.GetItemCount() != 0)
		m_lstFiles.DeleteAllItems();

	int m_nCntFiles = g_Protector.m_arFiles.GetSize();
	int item=0;
	for (int i=0; i<m_nCntFiles; i++)
	{
		CString st;
		t_FileInfo fi = g_Protector.m_arFiles.GetAt(i);
		st = fi.szName;

		if (fi.status > 0)
		{
			m_lstFiles.InsertItem(item, st);
			m_lstFiles.SetItemText(item, 0, st);

			FILESIZE fs = min(fi.errors*g_Protector.m_nRecoveryBlockSize, fi.size);

			if (fs<1024*1)
			{
				st.Format(_T("%u b"), fs);
			}
			else
			{
				if (fs<1024*1024*5)
				{
					st.Format(_T("%u KB"), fs/1024);
				}
				else
				{
					st.Format(_T("%u MB"), fs/(1024*1024));
				}

			}
			m_lstFiles.SetItemText(item, 1, st);

			switch (fi.status)
			{
			case 0:
				st=_T("Ok");
				break;
			case 1:
				st=_T("Recoverable");
				break;
			case 2:
				st=_T("Not Recoverable");
				break;
			case 3:
				st=_T("Missing");
				break;
			case 4:
				st=_T("Error open");
				break;
			case 5:
				st=_T("Size mismatch");
				break;
			case 6:
				st=_T("Not checked");
				break;
			case 7:
				st=_T("CRC Error");
				break;
			default:
				st=_T("?unknown?");
				break;
			}
			m_lstFiles.SetItemText(item, 2, st);
			item++;
		}
	}		
	ListView_SetExtendedListViewStyle( m_lstFiles.m_hWnd, LVS_EX_FULLROWSELECT );
}

void CDlgRecover::OnBnClickedBtnRecoverPath()
{
	//Select a folder
	UpdateData(TRUE);
	BROWSEINFO info;
	info.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = _T("Recover to");
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

	m_szRecoverPath = szFolder;
	UpdateData(FALSE);
}

void CDlgRecover::InvalidateControls()
{
	UpdateData(TRUE);

	if (m_bInPlace)
	{
		GetDlgItem(IDC_CHK_SUBDIRS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RECOVER_PATH2)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOVER_PATH)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_CHK_SUBDIRS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RECOVER_PATH2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_RECOVER_PATH)->EnableWindow(TRUE);
	}
}
void CDlgRecover::OnBnClickedChkInplace()
{
	InvalidateControls();
}
