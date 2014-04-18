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

#pragma once

#define _OP_LOADRECOVERY	1
#define _OP_CHECKFILES		2
#define _OP_ADDDIR			3
#define _OP_PROTECTFILE		4
#define _OP_PROTECTDIR		5
#define _OP_RECOVER			6

// CDlgWelcome dialog

class CDlgWelcome : public CDialog
{
	DECLARE_DYNAMIC(CDlgWelcome)

public:
	CDlgWelcome(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgWelcome();

// Dialog Data
	enum { IDD = IDD_DLG_WELCOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
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
	CStringArray m_szArgArFiles;
	CStringArray m_szArgArDirs;
	FILESIZE m_nArgSize;
	int m_nOperationType;
	int m_nReturnValue;
	bool m_bCheckNotAll;

	//Async functions
	static unsigned _stdcall trdLoadRecovery (LPVOID lpParameter);
	static unsigned _stdcall trdAddDir (LPVOID lpParameter);
	static unsigned _stdcall trdFilesAndDirs (LPVOID lpParameter);
	static unsigned _stdcall trdCheckFiles (LPVOID lpParameter);
	static unsigned _stdcall trdProtectFiles (LPVOID lpParameter);
	static unsigned _stdcall trdProtectDir (LPVOID lpParameter);
	static unsigned _stdcall trdRecoverFiles (LPVOID lpParameter);

	typedef unsigned (__stdcall *StaticFunction) (void *);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPaint();

public:
	afx_msg void OnBnClickedBtnProtectFiles();
	afx_msg void OnBnClickedBtnProtectDir();
	afx_msg void OnBnClickedBtnAbout();
	afx_msg void OnComplete();
	afx_msg void OnBnClickedBtnCheck();
};
