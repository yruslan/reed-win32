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
#include "DlgCheckFiles.h"
#include "DlgProgress.h"

extern CDlgProgress *g_DlgProgress;

// CDlgCheckFiles dialog

IMPLEMENT_DYNAMIC(CDlgCheckFiles, CDialog)

CDlgCheckFiles::CDlgCheckFiles(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCheckFiles::IDD, pParent)
	, m_bShowOnlyErrors(FALSE)
{

}

CDlgCheckFiles::~CDlgCheckFiles()
{
}

void CDlgCheckFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILES, m_lstFiles);
	DDX_Check(pDX, IDC_CHECK_SHOW_ERRORS, m_bShowOnlyErrors);
}


BEGIN_MESSAGE_MAP(CDlgCheckFiles, CDialog)
	ON_BN_CLICKED(IDC_BTN_QUCK_CHECK, &CDlgCheckFiles::OnBnClickedBtnQuickCheck)
	ON_BN_CLICKED(IDC_BTN_BACK, &CDlgCheckFiles::OnBnClickedBtnBack)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ERRORS, &CDlgCheckFiles::OnBnClickedCheckShowMissing)
END_MESSAGE_MAP()


// CDlgCheckFiles message handlers
BOOL CDlgCheckFiles::OnInitDialog()
{
	CDialog::OnInitDialog();

	CClientDC dc((CWnd *)&m_lstFiles);
	CRect rc;
	m_lstFiles.GetClientRect(rc);
	rc.DeflateRect( 1, 1, 1, 1 );

	CSize sz; 
	sz = dc.GetTextExtent(_T(" File Name (no path) - test ")); 
	m_lstFiles.InsertColumn(0, _T("File Name"), LVCFMT_CENTER, sz.cx); 

	sz = dc.GetTextExtent(_T(" 1002003 MB ")); 
	m_lstFiles.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, sz.cx); 

	sz = dc.GetTextExtent(_T(" Size Mismatch")); 
	m_lstFiles.InsertColumn(2, _T("Status"), LVCFMT_CENTER, sz.cx); 

	UpdateList();

	UpdateData(FALSE);

	return TRUE;	
}

void CDlgCheckFiles::OnBnClickedBtnQuickCheck()
{
	UINT flag = LVNI_SELECTED;
	int nRow;
	int cnt=0;

	for( nRow = m_lstFiles.GetItemCount()-1; nRow >=0; nRow -- )
	{
		t_FileInfo fi = g_Protector.m_arFiles.GetAt(nRow);

		if ((m_lstFiles.GetItemState(nRow, flag) & flag) != flag ) 
			fi.checked = 0;
		else
		{
			fi.checked = 1;
			cnt++;
		}
			
		g_Protector.m_arFiles.SetAt(nRow, fi);
	}

	if (cnt>0)
	{
		g_DlgProgress->m_bNeedTerminate = false;
		g_DlgProgress->ShowWindow(SW_SHOW);
		g_DlgProgress->GetDlgItem(IDC_OPERATION)->SetWindowText(_T("Quick check"));
		EnableWindow(FALSE);

		int	rc = g_Protector.Check2();
		UpdateList();
		
		EnableWindow(TRUE);
		g_DlgProgress->ShowWindow(SW_HIDE);

		if (rc==0)
		{
			MessageBox(_T("Check complete. No errors found!"),_T("Info"), MB_ICONINFORMATION);
		}
		else
		{
			CString s1;
			NumbersToBytes(g_Protector.m_nCntRecoverable, g_Protector.m_nRecoveryBlockSize, s1);
			CString msg;
			msg.Format(_T("Found CRC Errors! To recover files you'll need to run a FULL check."), s1);
			MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
		}
	}
}

void CDlgCheckFiles::OnBnClickedBtnBack()
{
	EndDialog(100);
}

void CDlgCheckFiles::OnBnClickedCheckShowMissing()
{
	UpdateData(TRUE);
	UpdateList();
}

void CDlgCheckFiles::UpdateList()
{
	if ( m_lstFiles.GetItemCount() != 0)
		m_lstFiles.DeleteAllItems();

	int m_nCntFiles = g_Protector.m_arFiles.GetSize();
	int nListCnt = 0;
	for (int i=0; i<m_nCntFiles; i++)
	{
		CString st;
		t_FileInfo fi = g_Protector.m_arFiles.GetAt(i);
		st = fi.szName;

		if (m_bShowOnlyErrors && fi.status!=3 && fi.status!=4 && fi.status!=7)
			continue;

		m_lstFiles.InsertItem(nListCnt, st);
		m_lstFiles.SetItemText(nListCnt, 0, st);

		if (fi.size<1024*1)
		{
			st.Format(_T("%u b"), fi.size);
		}
		else
		{
			if (fi.size<1024*1024*5)
			{
				st.Format(_T("%u KB"), fi.size/1024);
			}
			else
			{
				st.Format(_T("%u MB"), fi.size/(1024*1024));
			}

		}
		m_lstFiles.SetItemText(nListCnt, 1, st);

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
		m_lstFiles.SetItemText(nListCnt, 2, st);
		nListCnt++;
	}		
	ListView_SetExtendedListViewStyle( m_lstFiles.m_hWnd, LVS_EX_FULLROWSELECT );
}
