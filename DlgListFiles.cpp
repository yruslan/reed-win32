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
#include "DlgListFiles.h"


// CDlgListFiles dialog

IMPLEMENT_DYNAMIC(CDlgListFiles, CDialog)

CDlgListFiles::CDlgListFiles(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgListFiles::IDD, pParent)
{

}

CDlgListFiles::~CDlgListFiles()
{
}

void CDlgListFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgListFiles)
	DDX_Control(pDX, IDC_LIST_FILES, m_lstFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgListFiles, CDialog)
END_MESSAGE_MAP()


// CDlgListFiles message handlers

BOOL CDlgListFiles::OnInitDialog()
{
	CDialog::OnInitDialog();

	CClientDC dc((CWnd *)&m_lstFiles);
	CRect rc;
	m_lstFiles.GetClientRect(rc);
	rc.DeflateRect( 1, 1, 1, 1 );

	//LOGFONT logfont;
	//GetDlgItem(IDC_GROUP_ALL)->GetFont()->GetLogFont(&logfont);
	//logfont.lfWeight=FW_BOLD;

	CSize sz; 
	sz = dc.GetTextExtent(_T(" File Name (no path) - test test test ")); 
	m_lstFiles.InsertColumn(0, _T("File Name"), LVCFMT_CENTER, sz.cx); 

	sz = dc.GetTextExtent(_T(" 1002003 MB ")); 
	m_lstFiles.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, sz.cx); 

	UpdateList();

	return TRUE;
}

void CDlgListFiles::UpdateList()
{
	if ( m_lstFiles.GetItemCount() != 0)
		m_lstFiles.DeleteAllItems();

	int m_nCntFiles = g_Protector.m_arFiles.GetSize();
	for (int i=0; i<m_nCntFiles; i++)
	{
		CString st;
		t_FileInfo fi = g_Protector.m_arFiles.GetAt(i);
		st = fi.szName;
		m_lstFiles.InsertItem(i, st);
		m_lstFiles.SetItemText(i, 0, st);

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
		m_lstFiles.SetItemText(i, 1, st);
	}		
	ListView_SetExtendedListViewStyle( m_lstFiles.m_hWnd, LVS_EX_FULLROWSELECT );
}