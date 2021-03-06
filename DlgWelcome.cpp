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
#include <process.h>
#include "DlgWelcome.h"
#include "DlgProgress.h"
#include "PPRecoverySize.h"
#include "DlgCheckPath.h"
#include "DlgCheckFiles.h"
#include "DlgRecover.h"

CDlgProgress *g_DlgProgress = NULL;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CDlgWelcome dialog

IMPLEMENT_DYNAMIC(CDlgWelcome, CDialog)

CDlgWelcome::CDlgWelcome(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWelcome::IDD, pParent),
	m_bStartedFromCommandLine(false)
{
	m_bRunning = false;
	::InitializeCriticalSection(&m_CS);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDlgWelcome::~CDlgWelcome()
{
	if (g_DlgProgress!=NULL)
	{
		delete g_DlgProgress;
		g_DlgProgress = NULL;
	}
	::DeleteCriticalSection(&m_CS);
}

void CDlgWelcome::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgWelcome, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CDlgWelcome::OnBnClickedBtnAbout)
	ON_BN_CLICKED(IDC_BTN_PROTECT_FILES, &CDlgWelcome::OnBnClickedBtnProtectFiles)
	ON_BN_CLICKED(IDC_BTN_PROTECT_DIR, &CDlgWelcome::OnBnClickedBtnProtectDir)
	ON_COMMAND(ID_CUSTOM_COMPLETED, &CDlgWelcome::OnComplete)
	ON_BN_CLICKED(IDC_BTN_CHECK, &CDlgWelcome::OnBnClickedBtnCheck)
END_MESSAGE_MAP()

// CDlgWelcome message handlers

BOOL CDlgWelcome::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (g_DlgProgress==NULL)
	{
		g_DlgProgress = new CDlgProgress;
		g_DlgProgress->Create(IDD_PROGRESS, this);
		g_DlgProgress->ShowWindow(SW_HIDE);
	}

	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString szCmdLine = AfxGetApp()->m_lpCmdLine;

	if (szCmdLine!="")
	{
		CStringArray arRecFiles;
		CStringArray arProtectFiles;
		CStringArray arProtectDirs;

		CStringArray arInParams;

		CString szSeps = _T(" ");
		if (szCmdLine.Find(_T("\""))>=0)
			szSeps = _T("\"");

		m_bStartedFromCommandLine = true;

		// Debug print command line
		//CString msg;
		//msg.Format(_T("'%s'"), szCmdLine);
		//MessageBox(msg, _T("CMD"), 0);

		int curPos = 0;
		CString szToken = szCmdLine.Tokenize(szSeps, curPos);
		while (szToken != "")
		{
			// If the token is a recovery info file name
			if (szToken.GetLength()>4 && !szToken.Right(4).CompareNoCase(_T(".rcv")))
			{
				arRecFiles.Add(szToken);
			}
			else
			{
				// If the token is an ordinary file or directory name
				WIN32_FIND_DATA fd;
				HANDLE  hFind = FindFirstFile(szToken, &fd);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
						arProtectDirs.Add(szToken);
					else
					{
						// Skip zero-sized files
						if (fd.nFileSizeHigh>0 || fd.nFileSizeLow>0)
							arProtectFiles.Add(szToken);
					}
					FindClose(hFind);
				}
			}
			szToken = szCmdLine.Tokenize(szSeps, curPos);
		}

		// If have at least one directory
		if (arProtectDirs.GetSize()>0)
		{
			if (arProtectDirs.GetSize()>1 || arProtectFiles.GetSize()>0)
			{
				m_szArgArFiles.Copy(arProtectFiles);
				m_szArgArDirs.Copy(arProtectDirs);
				int rc = RunAsync(trdFilesAndDirs);
			}
			else
			{
				// Start directory protection dialog
				m_szArgDir = arProtectDirs[0];
				int rc = RunAsync(trdAddDir);
			}
		}
		// If have at least one file to protect
		else if (arProtectFiles.GetSize()>0)
		{
			for (int i=0; i<arProtectFiles.GetSize(); i++)
			{
				int ret = g_Protector.AddFileInfo(arProtectFiles[i]);
				if (ret!=0)
				{
					CString msg;
					msg.Format(_T("Unable to read file '%s'!"), arProtectFiles[i]);
					MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
					g_Protector.Clear();
					AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
				}				
			}
			if (g_Protector.m_arFiles.GetSize()>0)
			{
				// Start protection dialog
				m_nReturnValue = 0;
				m_nOperationType = _OP_ADDDIR;
				OnComplete();
			}
		}
		else if (arRecFiles.GetSize()>0)
		{
			if (arRecFiles.GetSize()>1)
				MessageBox(_T("Please, select only one recovery info file a time!"),_T("Error"), MB_ICONEXCLAMATION);
			else
			{
				// Start check and recovery dialog
				m_szArgFileName = arRecFiles[0];
				int rc = RunAsync(trdLoadRecovery);
			}
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgWelcome::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CDlgWelcome::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDlgWelcome::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDlgWelcome::OnBnClickedBtnAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CDlgWelcome::OnBnClickedBtnProtectDir()
{
	g_Protector.Clear();

	//Select a folder
	UpdateData(TRUE);
	BROWSEINFO info;
	info.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = _T("Select directory to protect");
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

	m_szArgDir = szFolder;
	int rc = RunAsync(trdAddDir);
}

void CDlgWelcome::OnBnClickedBtnProtectFiles()
{
	g_Protector.Clear();

	//Select files to protect
	CFileDialog dlg(TRUE, _T("All"), _T("*.*"), OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, _T("All Files(*.*)|*.*||") );

	CString szBuf;

	dlg.m_ofn.lpstrFile = szBuf.GetBuffer(10*1024*1024);
	dlg.m_ofn.nMaxFile = 10*1024*1024;

	int ret = dlg.DoModal();

	if (ret!=IDOK) return;

	//Add files
	POSITION p=dlg.GetStartPosition();
	CString s;
	bool bHaveRcv = false;
	while (p!=NULL)
	{
		s=dlg.GetNextPathName(p);
		// Allow to protect recovery files, why not?
		//if (s.Right(4)==_T(".rcv"))
		//{
		//	bHaveRcv = true;
		//	continue;
		//}
		int ret = g_Protector.AddFileInfo(s);
		if (ret!=0)
		{
			CString msg;
			msg.Format(_T("Unable to read file '%s'!"), s);
			MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			return;
		}
	}
	szBuf.ReleaseBuffer(1);

	if (g_Protector.m_arFiles.GetSize()==0)
	{
		if (bHaveRcv)
			MessageBox(_T("You are asking to protect a recovery info file. Please, select different file."),_T("Error"), MB_ICONEXCLAMATION);
		else
			MessageBox(_T("No files selected!"),_T("Error"), MB_ICONEXCLAMATION);
		return;
	}

	m_nReturnValue = 0;
	m_nOperationType = _OP_ADDDIR;
	OnComplete();
}

void CDlgWelcome::OnBnClickedBtnCheck()
{
	g_Protector.Clear();

	CFileDialog FileDialog( TRUE, _T("RECOVERY"), _T("*.rcv"), 0, _T("Recovery Files (*.rcv)|*.rcv||"));
	if(FileDialog.DoModal()==IDCANCEL ) return;

	CString szRecFile = FileDialog.GetPathName(); 
	
	m_szArgFileName = szRecFile;
	int rc = RunAsync(trdLoadRecovery);
}

int CDlgWelcome::RunAsync (StaticFunction fn)
{
	::EnterCriticalSection(&m_CS);
	bool bRunning = m_bRunning;
	g_DlgProgress->m_bNeedTerminate = false;
	::LeaveCriticalSection(&m_CS);

	if (bRunning) return -1;

	g_DlgProgress->ShowWindow(SW_SHOW);
	EnableWindow(FALSE);

	m_hThread = (HANDLE) _beginthreadex( 
		NULL, //void *security
		0,    //unsigned stack_size
		fn, 
		this, //parameter
		false, //CREATE_SUSPEND
		NULL 
		);

	if (m_hThread==NULL) // || m_hThread==1
	{
		m_hThread=0;
		bRunning = false;
		EnableWindow(TRUE);
		g_DlgProgress->ShowWindow(SW_HIDE);
		return -1;
	}

	return 0;
}

unsigned _stdcall CDlgWelcome::trdAddDir (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_ADDDIR;

	int rc = g_Protector.AddDir(d->m_szArgDir);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CDlgWelcome::trdFilesAndDirs (LPVOID lpParameter)
{
	//m_szArgArFilesAndDirs
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_ADDDIR;

	int rc = 0;

	// Get Minimum Path
	CString szPath = _T("");
	if (d->m_szArgArDirs.GetSize()>0)
		szPath = d->m_szArgArDirs[0];
	if (d->m_szArgArFiles.GetSize()>0)
		szPath = d->m_szArgArFiles[0];

	for (int i=0; i<d->m_szArgArFiles.GetSize(); i++)
	{
		int j=0;
		while (j<szPath.GetLength() && j<d->m_szArgArFiles[i].GetLength())
		{
			if (szPath[j]!=d->m_szArgArFiles[i][j])
				break;
			j++;
		}
		szPath = szPath.Left(j);
	}
	for (int i=0; i<d->m_szArgArDirs.GetSize(); i++)
	{
		int j=0;
		while (j<szPath.GetLength() && j<d->m_szArgArDirs[i].GetLength())
		{
			if (szPath[j]!=d->m_szArgArDirs[i][j])
				break;
			j++;
		}
		szPath = szPath.Left(j);
	}
	_tcscpy(g_Protector.m_szPath, szPath);

	// Add Files
	for (int i=0; i<d->m_szArgArFiles.GetSize(); i++)
	{
		if (!rc) rc |= g_Protector.AddFileInfo(d->m_szArgArFiles[i]);
	}

	// Add Dirs	
	for (int i=0; i<d->m_szArgArDirs.GetSize(); i++)
	{
		if (!rc) g_Protector.AddDir(d->m_szArgArDirs[i]);
	}

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);
	return 0;
}

unsigned _stdcall CDlgWelcome::trdProtectDir (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_PROTECTDIR;

	int rc = g_Protector.CreateSolidRecovery2(d->m_nArgSize, d->m_szArgFileName);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CDlgWelcome::trdLoadRecovery (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_LOADRECOVERY;

	int rc = g_Protector.LoadRecovery2(d->m_szArgFileName);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CDlgWelcome::trdCheckExists (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_CHECKEXISTS;

	int rc = g_Protector.CheckFilesExists();

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CDlgWelcome::trdCheckFiles (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_CHECKFILES;

	int rc = g_Protector.Check2();

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CDlgWelcome::trdRecoverFiles (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CDlgWelcome *d = (CDlgWelcome * ) lpParameter;
	d->m_nOperationType = _OP_RECOVER;

	int rc = g_Protector.Recover2();

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

void CDlgWelcome::OnComplete()
{
	EnableWindow(TRUE);
	g_DlgProgress->ShowWindow(SW_HIDE);

	int rc = m_nReturnValue;
	static int nWizardStep = 1;

	if (m_nOperationType == _OP_ADDDIR)
	{
		if (rc!=0)
		{
			MessageBox(_T("Failed to scan directory!"),_T("Error"), MB_ICONEXCLAMATION);
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return;
		}
		if (g_Protector.m_nTotalSize < 10000)
		{
			MessageBox(_T("Total size of selected file(s) is too small for this protection to be practical (<10KB)."), _T("Error"), MB_ICONEXCLAMATION);
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return;
		}

		CPropertySheet pS;

		pS.m_psh.dwFlags &= ~PSH_HASHELP;
		CPPRecoverySize p1;

		// For small files
		if (g_Protector.m_nTotalSize < 5000000)
			p1.m_bSizeInMB = false;
		else
			p1.m_bSizeInMB = true;

		p1.m_psp.dwFlags &= ~PSP_HASHELP;

		pS.AddPage(&p1);
		pS.SetWizardMode();

		if(pS.DoModal() == IDCANCEL)
		{
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return;
		}

		int rec_size=1;

		rec_size = p1.m_nRecSizeMB;			

		g_Protector.m_bCreateRecForRec = p1.m_bRecForRec ? true : false; // BOOL -> bool

		if (p1.m_bSizeInMB)
		{
			m_nArgSize = ((FILESIZE)rec_size)*1024*1024;
		}
		else
		{
			g_Protector.m_nRecoveryBlockSize = 512;
			m_nArgSize = ((FILESIZE)rec_size)*1024;
		}
		m_nArgSize = g_Protector.SetRecoverySize(m_nArgSize);

		m_szArgFileName = p1.m_szRecFile;
		int rc = RunAsync(trdProtectDir);
	}

	if (m_nOperationType==_OP_PROTECTDIR)
	{
		if (rc!=0)
		{
			g_Protector.Clear();
			if (rc == -1)
			{
				CString msg;
				msg.Format(_T("Too small size. Cannot continue!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == -2)
			{
				CString msg;
				msg.Format(_T("No Files Selected. Cannot continue!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_FILE_CANNOT_WRITE)
			{
				CString msg;
				msg.Format(_T("Unable to write to the recovery file '%s'!"), m_szArgFileName);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return;
		}

		// Create recover info file for recovery info file
		if (g_Protector.m_bCreateRecForRec)
		{
			g_Protector.Clear();
			g_Protector.m_bCreateRecForRec = false;
			int ret = g_Protector.AddFileInfo(m_szArgFileName);
			if (ret!=0)
			{
				CString msg;
				msg.Format(_T("Unable to read file '%s'!"), m_szArgFileName);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
				g_Protector.Clear();
				if (m_bStartedFromCommandLine)
					AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			}
			FILESIZE rec_size = (FILESIZE) (double(g_Protector.m_nTotalSize) * 0.1);
			if (rec_size>1024*1024)
			{
				rec_size = g_Protector.SetRecoverySize(rec_size);
				m_nArgSize = rec_size;
				m_szArgFileName += ".rcv";
				int rc = RunAsync(trdProtectDir);
				return;
			}
		}

		CString msg;
		msg.Format(_T("Recovery file saved to '%s'!"), m_szArgFileName);
		MessageBox(msg,_T("Information"), MB_ICONINFORMATION);
		g_Protector.Clear();
		if (m_bStartedFromCommandLine)
			AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
	}

	if (m_nOperationType==_OP_LOADRECOVERY)
	{
		if (rc!=0)
		{
			if (rc == E_CRC_ERROR)
			{
				CString msg;
				msg.Format(_T("Sorry, the recovery file itself is corrupted. Recovery info cannot be used to check and recover files."));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_FILE_NOT_OPENS)
			{
				CString msg;
				msg.Format(_T("Error opening recovery file %s!"), m_szArgFileName);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_VERSION_OLD)
			{
				CString msg;
				msg.Format(_T("Version of recovery is too old!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_FORMAT_ERROR)
			{
				CString msg;
				msg.Format(_T("Unknown format error!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return; 
		}
		if (g_Protector.m_nTotalSize==0)
		{
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
			return;
		}

		if (g_Protector.m_arFiles.GetSize()==1)
		{
			if (!_taccess(g_Protector.m_arFiles[0].szName, 04))
			{
				// Start Checking files
				t_FileInfo fi = g_Protector.m_arFiles.GetAt(0);
				fi.checked = 1;
				g_Protector.m_arFiles.SetAt(0, fi);
				RunAsync(trdCheckFiles);
				return;
			}
		}
		nWizardStep = 1;
		m_nOperationType=_OP_CHECKEXISTS;
	}
	if (m_nOperationType==_OP_CHECKEXISTS)
	{
		//Wizard! Specify the original path and files to check
		while (true)
		{
			if (nWizardStep==2 && g_Protector.m_nFilesExistCount==0)
			{
				CString msg;
				msg.Format(_T("Selected folder (%s) does not contain any of the source files.\n")
					_T("Please, specify the source folder again."), g_Protector.m_szPath);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
				nWizardStep--;
			}
			if (nWizardStep==1)
			{
				CDlgCheckPath dlg1;
				int ret = dlg1.DoModal();

				if (ret!=IDOK)
				{
					if (m_bStartedFromCommandLine)
						AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
					return;
				}
				if (dlg1.m_szSrcPath!=_T(""))
				{
					nWizardStep++;
					_tcscpy(g_Protector.m_szPath, dlg1.m_szSrcPath);
					RunAsync(trdCheckExists);
					return;
				}
				nWizardStep++;
			}
			if (nWizardStep==2)
			{
				CDlgCheckFiles dlg2;
				int ret = dlg2.DoModal();

				if (ret==100)
				{
					nWizardStep--;
					continue;
				}
				if (ret!=IDOK)
					return;
				//Run check
				//...
				for (ret=0; ret<g_Protector.m_arFiles.GetSize(); ret++)
				{
					t_FileInfo fi = g_Protector.m_arFiles.GetAt(ret);
					fi.checked = 1;
					g_Protector.m_arFiles.SetAt(ret, fi);
				}
				RunAsync(trdCheckFiles);
				return;
			}
			break;
		}
	}
	if (m_nOperationType==_OP_RECOVER)
	{
		if (rc==0)
		{
			CString msg;
			msg.Format(_T("Files are successfully recovered!"));
			MessageBox(msg,_T("Information"), MB_ICONINFORMATION);
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
		}
		else
		{
			CString msg;
			if (rc==E_COPY_FAILED)
			{
				msg.Format(_T("Unable to write to output directory!\n")
					       _T("Please, choose an empty directory with write access to save recovered files ")
						   _T("or set \"Recover In-Place\" checkbox.\n"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
				
				// Back to recover dialog
				m_nOperationType=_OP_CHECKFILES;
			}
			else
			{
				msg.Format(_T("Recover failed!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
				if (m_bStartedFromCommandLine)
					AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
				return;
			}			
		}		
	}
	if (m_nOperationType==_OP_CHECKFILES)
	{
		if (rc==0)
		{
			MessageBox(_T("Check complete. No errors found!"),_T("Info"), MB_ICONINFORMATION);
			if (m_bStartedFromCommandLine)
				AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
		}
		else
		{
			if (rc==E_PROTECTOR_TERMINATED)
				return;

			if (rc == E_NOT_ENOUGH_SPACE)
			{
				CString szMsg;
				szMsg.Format(_T("Not enough space on disk drive (while writing %s)!"),
					(LPCTSTR)g_Protector.GetLastTempFileName());
				MessageBox(szMsg,_T("Info"), MB_ICONEXCLAMATION);
				return;
			}

			//Select files to recover
			//and recover path
			CDlgRecover dlg;
			int ret = dlg.DoModal();

			if (ret==IDOK)
			{
				if (dlg.m_bInPlace)
					g_Protector.m_szRecoverPath = _T("");
				else
					g_Protector.m_szRecoverPath = dlg.m_szRecoverPath;

				if (dlg.m_bInPlace)
					g_Protector.m_bCreateSubdirs = true;
				else
					g_Protector.m_bCreateSubdirs = (dlg.m_bSubdirs!=FALSE);

				for (ret=0; ret<g_Protector.m_arFiles.GetSize(); ret++)
				{
					t_FileInfo fi = g_Protector.m_arFiles.GetAt(ret);
					if (fi.status>0)
					{
						fi.to_recover = 1;
						g_Protector.m_arFiles.SetAt(ret, fi);
					}
				}

				//Recover
				//...
				int rc = RunAsync(trdRecoverFiles);
			}
		}
		return;
	}
}
