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

// reedDlg.h : header file
//

#if !defined(AFX_REEDDLG_H__919BF24F_7B6B_47FA_8C0C_6543A94471DA__INCLUDED_)
#define AFX_REEDDLG_H__919BF24F_7B6B_47FA_8C0C_6543A94471DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Protector.h"

/////////////////////////////////////////////////////////////////////////////
// CReedDlg dialog

#define _OP_LOADRECOVERY	1
#define _OP_CHECKFILES		2
#define _OP_ADDDIR			3
#define _OP_PROTECTFILE		4
#define _OP_PROTECTDIR		5
#define _OP_RECOVER			6

class CReedDlg : public CDialog
{
// Construction
public:
	CReedDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CReedDlg();

// Dialog Data
	//{{AFX_DATA(CReedDlg)
	enum { IDD = IDD_REED_DIALOG };
	CListCtrl	m_lstFiles;
	CString	m_szPercentSize;
	CString	m_szRecSize;
	CString	m_szBlockSize;
	CString	m_szCntBlocks;
	CString	m_szTotalSize;
	CString m_szPath;
	CString m_szRecoverPath;
	BOOL m_bQuadro;

	int m_nRadio;
	int m_nRadio3;
	CFont m_fntBold;

	//Thread tools
	bool m_bRunning;
	HANDLE m_hThread;			//handler
	CRITICAL_SECTION m_CS;		//critical section

	typedef unsigned (__stdcall *StaticFunction) (void *);

	//Async Run
	int RunAsync (StaticFunction fn);

	//Arguments
	CString m_szArgFileName;
	CString m_szArgDir;
	FILESIZE m_nArgSize;
	int m_nOperationType;
	int m_nReturnValue;
	bool m_bCheckNotAll;

	//Async functions
	static unsigned _stdcall trdLoadRecovery (LPVOID lpParameter);
	static unsigned _stdcall trdAddDir (LPVOID lpParameter);
	static unsigned _stdcall trdCheckFiles (LPVOID lpParameter);
	static unsigned _stdcall trdProtectFiles (LPVOID lpParameter);
	static unsigned _stdcall trdProtectDir (LPVOID lpParameter);
	static unsigned _stdcall trdRecoverFiles (LPVOID lpParameter);

	//}}AFX_DATA

public:
	CProtector rec;			//The Recovery Info!

	//CStringArray m_arFiles;
	//CStringArray m_arSizes;
	//CStringArray m_arErrors;

	void InvalidateControls();
	void UpdateList();

	afx_msg void OnLvnColumnclickFilesTab(NMHDR *pNMHDR, LRESULT *pResult);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CReedDlg)
	virtual BOOL OnInitDialog();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnCalculate();
	afx_msg void OnBtnCheck();
	afx_msg void OnBtnCheckThis();
	afx_msg void OnBtnLoadRecovery();
	afx_msg void OnBtnLocate();
	afx_msg void OnBtnProtectDirectory();
	afx_msg void OnBtnProtectFiles();
	afx_msg void OnBtnRecover();
	afx_msg void OnBtnRecoverPath();
	afx_msg void OnBtnRemove();
	afx_msg void OnKillfocusEditRecSize();
	afx_msg void OnKillfocusEditBlockSize();
	afx_msg void OnBnClickedBtnRecoverThis();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnEnChangeEditPercentSize();
	afx_msg void OnEnKillfocusEditPercentSize();
	afx_msg void OnComplete();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEnChangeEditRecSize();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REEDDLG_H__919BF24F_7B6B_47FA_8C0C_6543A94471DA__INCLUDED_)
