// reedDlg.cpp : implementation file
//

#include "stdafx.h"
#include <process.h>
#include "reed.h"
#include "reedDlg.h"
#include "DlgProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

CDlgProgress *g_DlgProgress = NULL;

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

#define ID_CUSTOM_COMPLETED	10000
/////////////////////////////////////////////////////////////////////////////
// CReedDlg dialog

CReedDlg::CReedDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReedDlg::IDD, pParent)
	, m_bQuadro(FALSE)
{
	//{{AFX_DATA_INIT(CReedDlg)
	m_szPercentSize = _T("10");
	m_szRecSize = _T("100");
	m_szSectorSize = _T("32");
	m_szCntSectors = _T("32000");
	m_szTotalSize = _T("");
	m_szRecoverPath = _T("");
	m_nRadio = 1;
	m_nRadio3 = 0;
	m_bRunning = false;
	m_hThread = 0;
	::InitializeCriticalSection(&m_CS);
	m_szArgFileName = _T("");
	m_szArgDir = _T("");
	m_nOperationType = 0;
	m_nReturnValue = 0;
	m_bCheckNotAll = false;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CReedDlg::~CReedDlg()
{
	if (g_DlgProgress!=NULL)
	{
		delete g_DlgProgress;
		g_DlgProgress = NULL;
	}
	::DeleteCriticalSection(&m_CS);		
}


void CReedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReedDlg)
	DDX_Control(pDX, IDC_LIST_FILES, m_lstFiles);
	DDX_Text(pDX, IDC_EDIT_PERCENT_SIZE, m_szPercentSize);
	//DDV_MaxChars(pDX, m_szPercentSize, 2);
	DDX_Text(pDX, IDC_EDIT_REC_SIZE, m_szRecSize);
	DDX_Radio(pDX, IDC_RADIO1, m_nRadio);
	DDX_Radio(pDX, IDC_RADIO3, m_nRadio3);
	DDV_MaxChars(pDX, m_szRecSize, 4);
	DDX_Text(pDX, IDC_EDIT_SECTOR_SIZE, m_szSectorSize);
	DDX_Text(pDX, IDC_EDIT_RECOVER_PATH, m_szRecoverPath);
	//DDV_MaxChars(pDX, m_szSectorSize, 3);
	DDX_Text(pDX, IDC_STATIC_SECTOR_COUNT, m_szCntSectors);
	DDX_Text(pDX, IDC_STATIC_TOTAL_SIZE, m_szTotalSize);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHK_QUADRO, m_bQuadro);
}

BEGIN_MESSAGE_MAP(CReedDlg, CDialog)
	//{{AFX_MSG_MAP(CReedDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_ADD, &CReedDlg::OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_CALCULATE, &CReedDlg::OnBtnCalculate)
	ON_BN_CLICKED(IDC_BTN_CHECK, &CReedDlg::OnBtnCheck)
	ON_BN_CLICKED(IDC_BTN_LOAD_RECOVERY, &CReedDlg::OnBtnLoadRecovery)
	ON_BN_CLICKED(IDC_BTN_LOCATE, &CReedDlg::OnBtnLocate)
	ON_BN_CLICKED(IDC_BTN_PROTECT_DIRECTORY, &CReedDlg::OnBtnProtectDirectory)
	ON_BN_CLICKED(IDC_BTN_PROTECT_FILES, &CReedDlg::OnBtnProtectFiles)
	ON_BN_CLICKED(IDC_BTN_RECOVER, &CReedDlg::OnBtnRecover)
	ON_BN_CLICKED(IDC_BTN_RECOVER_PATH, &CReedDlg::OnBtnRecoverPath)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CReedDlg::OnBtnRemove)
	ON_BN_CLICKED(IDC_BTN_RECOVER_THIS, &CReedDlg::OnBnClickedBtnRecoverThis)
	ON_BN_CLICKED(IDC_BTN_CHECK_THIS, &CReedDlg::OnBtnCheckThis)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_FILES, &CReedDlg::OnLvnColumnclickFilesTab)
	ON_EN_KILLFOCUS(IDC_EDIT_REC_SIZE, &CReedDlg::OnKillfocusEditRecSize)
	ON_EN_KILLFOCUS(IDC_EDIT_SECTOR_SIZE, &CReedDlg::OnKillfocusEditSectorSize)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO1, &CReedDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CReedDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CReedDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO4, &CReedDlg::OnBnClickedRadio1)
	ON_EN_CHANGE(IDC_EDIT_PERCENT_SIZE, &CReedDlg::OnEnChangeEditPercentSize)
	ON_EN_KILLFOCUS(IDC_EDIT_PERCENT_SIZE, &CReedDlg::OnEnKillfocusEditPercentSize)
	ON_COMMAND(ID_CUSTOM_COMPLETED, &CReedDlg::OnComplete)
	ON_WM_SETCURSOR()
	ON_EN_CHANGE(IDC_EDIT_REC_SIZE, &CReedDlg::OnEnChangeEditRecSize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReedDlg message handlers

void NumbersToBytes(int sectors, int sector_size, CString &msg)
{
	unsigned __int64 bytes = sectors*sector_size;

	if (bytes<10*1024)
	{
		msg.Format(_T("%u bytes"), bytes);
	}
	else
	{
		if (bytes<5*1024*1024)
			msg.Format(_T("%u KB"), (unsigned __int64) (bytes/1024));
		else
			msg.Format(_T("%d MB"), int(bytes/(1024*1024)));
	}
}

BOOL CReedDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (g_DlgProgress==NULL)
	{
		g_DlgProgress = new CDlgProgress;
		g_DlgProgress->Create(IDD_PROGRESS, this);
		g_DlgProgress->ShowWindow(SW_HIDE);
		//EnableWindow(FALSE);
		//EnableWindow(TRUE);
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
	
	InvalidateControls();

	CClientDC dc((CWnd *)&m_lstFiles);
	CRect rc;
	m_lstFiles.GetClientRect(rc);
	rc.DeflateRect( 1, 1, 1, 1 );

	LOGFONT logfont;
	GetDlgItem(IDC_OPERATION)->GetFont()->GetLogFont(&logfont);
	logfont.lfWeight=FW_BOLD;

	m_fntBold.CreateFontIndirect(&logfont);
	GetDlgItem(IDC_STATIC_TOTAL_SIZE)->SetFont(&m_fntBold, TRUE);
	//GetDlgItem(IDC_STATIC_SECTOR_COUNT)->SetFont(&m_fntBold, TRUE);
	
	CSize sz; 
	sz = dc.GetTextExtent(_T(" File Name (no path) - ")); 
	m_lstFiles.InsertColumn(0, _T("File Name"), LVCFMT_CENTER, sz.cx); 

	sz = dc.GetTextExtent(_T(" 1002003 MB ")); 
	m_lstFiles.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, sz.cx); 

	sz = dc.GetTextExtent(_T(" Size Mismatch")); 
	m_lstFiles.InsertColumn(2, _T("Status"), LVCFMT_CENTER, sz.cx); 

	CString szCmdLine = AfxGetApp()->m_lpCmdLine;
	if (szCmdLine!="")
	{
		CString szRecFile = szCmdLine; 
		if (szCmdLine.GetLength()>2)
		{
			if (szRecFile.Left(1)=="\"" && szRecFile.Right(1)=="\"")
			{
				szRecFile = szRecFile.Left(szRecFile.GetLength()-1);
				szRecFile = szRecFile.Right(szRecFile.GetLength()-1);
			}
		}
		
		m_szArgFileName = szRecFile;
		int rc = RunAsync(trdLoadRecovery);

	}

	UpdateList();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CReedDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CReedDlg::OnPaint() 
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
HCURSOR CReedDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CReedDlg::OnLvnColumnclickFilesTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

/*	int m_nCmpCol = pNMLV->iSubItem;	
	int cnt = m_arFiles.GetCount();
	for (int i = cnt-2; i > 0; i--)
	{
		for (int j = 0; j<=i; j++)
		{
			Strata1 = m_pArStrata->GetAt(j);
			Strata2 = m_pArStrata->GetAt(j+1);
			switch (m_nCmpCol)
			{
			case 0:
				if (Strata1.m_nCode > Strata2.m_nCode)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 1:
				if ( _tcscmp( Strata1.m_szName, Strata2.m_szName) > 0)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 2:
				if ( Strata1.m_flDepth > Strata2.m_flDepth )
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 3:
				if (Strata1.m_nType > Strata2.m_nType)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 4:
				if (Strata1.m_bIsBottom > Strata2.m_bIsBottom)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 5:
				if (Strata1.m_fl_X > Strata2.m_fl_X)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			case 6:
				if (Strata1.m_fl_Y > Strata2.m_fl_Y)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			default:
				if (Strata1.m_fl_Z > Strata2.m_fl_Z)
				{
					m_pArStrata->RemoveAt(j);
					m_pArStrata->InsertAt(j+1, Strata1);
				}
				break;
			}
		}			
	}

	UpDateList();*/
    
	*pResult = 0;
}

void CReedDlg::InvalidateControls()
{
	UpdateData(TRUE);
	if (rec.m_bReadOnly)
	{
		GetDlgItem(IDC_CHK_QUADRO)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_REMOVE)->EnableWindow(FALSE);
		//GetDlgItem(IDC_BTN_LOCATE)->EnableWindow(FALSE);

		GetDlgItem(IDC_EDIT_REC_SIZE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SECTOR_SIZE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PERCENT_SIZE)->EnableWindow(FALSE);
		//GetDlgItem(IDC_BTN_CALCULATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PROTECT_FILES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PROTECT_DIRECTORY)->EnableWindow(FALSE);

		GetDlgItem(IDC_BTN_LOAD_RECOVERY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOAD_RECOVERY)->SetWindowText(_T("Unload File"));
		GetDlgItem(IDC_BTN_CHECK)->EnableWindow(TRUE);
		if (rec.isAllChecked())
		{
			GetDlgItem(IDC_BTN_CHECK_THIS)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BTN_CHECK_THIS)->EnableWindow(TRUE);
		}
		if (rec.m_nCntRecoverable>0 && rec.isAllChecked())
		{
			GetDlgItem(IDC_BTN_RECOVER)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_RECOVER_THIS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO3)->EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO4)->EnableWindow(TRUE);
			if (m_nRadio3==1)
				GetDlgItem(IDC_EDIT_RECOVER_PATH)->EnableWindow(TRUE);
			else
				GetDlgItem(IDC_EDIT_RECOVER_PATH)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RECOVER_PATH)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_RECOVER)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_RECOVER_THIS)->EnableWindow(FALSE);
		}

	}
	else
	{
		GetDlgItem(IDC_CHK_QUADRO)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ADD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_REMOVE)->EnableWindow(TRUE);
		//GetDlgItem(IDC_BTN_LOCATE)->EnableWindow(FALSE);

		if (m_nRadio==0)
		{
			GetDlgItem(IDC_EDIT_REC_SIZE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_SECTOR_SIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_PERCENT_SIZE)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_EDIT_REC_SIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_SECTOR_SIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_PERCENT_SIZE)->EnableWindow(TRUE);
		}
		//GetDlgItem(IDC_BTN_CALCULATE)->EnableWindow(TRUE);

		if (rec.m_arFiles.GetSize()==0)
			GetDlgItem(IDC_BTN_PROTECT_FILES)->EnableWindow(FALSE);
			
		else
			GetDlgItem(IDC_BTN_PROTECT_FILES)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_PROTECT_DIRECTORY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOAD_RECOVERY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOAD_RECOVERY)->SetWindowText(_T("Load Recovery Info"));		

		GetDlgItem(IDC_BTN_CHECK_THIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RECOVER_THIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RECOVER_PATH)->EnableWindow(FALSE);
	}
	//Calc Sector Count
	int files = rec.m_arFiles.GetSize();
	double percent = double(_ttoi(m_szPercentSize))*0.01;
	FILESIZE size = rec.m_nTotalSize;

	if (rec.m_bReadOnly)
	{
		double recs = _ttoi(m_szRecSize);
		recs *= 1024.*1024.;
		double tots = double(rec.m_nTotalSize);
		if (recs>0 && tots>0)
		{
			int percent = int(100.*recs/tots);
			if (percent>100) percent=100;
			m_szPercentSize.Format(_T("%d"), percent);
		}
	}

	int rec_size = _ttoi(m_szRecSize);

	if (m_nRadio==1 && rec.m_nTotalSize>0)
	{
		rec_size = int((double(rec.m_nTotalSize)*percent)/double(1024*1024));
	}
	
	if (!rec.m_bReadOnly)
	{
		int sec_size=4;
		if (rec_size<1) rec_size = 1;
		if (rec_size>10000) rec_size = 10000;
		if (rec_size>10) sec_size=4;
		if (rec_size>50) sec_size=8;
		if (rec_size>100) sec_size=16;
		if (rec_size>500) sec_size=32;
		if (rec_size>1000) sec_size=64;
		if (rec_size>2000) sec_size=128;
		if (rec_size>4000) sec_size=256;
		if (m_nRadio==1)
			m_szRecSize.Format(_T("%d"),rec_size);
		m_szSectorSize.Format(_T("%d"),sec_size);
	}

	__int64 recs = _ttoi(m_szRecSize);
	recs *= 1024*1024;
	__int64 ssize = _ttoi(m_szSectorSize);
	ssize *= 1024;

	if (ssize<recs)
	{
		m_szCntSectors.Format(_T("%d"), recs/ssize);
	}
	else
	{
		m_szCntSectors = _T("!wrong!");
	}
	m_szTotalSize.Format(_T("%u MB in %d files"), (unsigned int)(size/(1024*1024)), files);
	UpdateData(FALSE);
}

void CReedDlg::OnBtnAdd() 
{
	UpdateData(TRUE);

	CFileDialog dlgFiles(TRUE, _T("RECOVFILES"), _T("*.*"), OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, _T("All Files(*.*)|*.*||") );
	CString strFileNames;
	dlgFiles.m_ofn.lpstrFile = strFileNames.GetBuffer(65536);
	dlgFiles.m_ofn.nMaxFile = 65536;
	
	int ret = dlgFiles.DoModal();

	if( ret == IDCANCEL ) return;

	POSITION p=dlgFiles.GetStartPosition();
	CString s;
	CString sPath=_T("");
	
	while (p!=NULL)
	{
		s=dlgFiles.GetNextPathName(p);
		//int i,e=0;
		//for (i=0; i<m_aFilesLocal.GetCount(); i++)
		//	if (m_aFilesLocal.GetAt(i)==s) e=1;
		//if (e == 0)
		//{
		int rc = rec.AddFileInfo(s);
		if (rc==0)
		{
			int pos = s.ReverseFind('\\');
			if (pos>0)
			{
				CString s2 = s.Left(pos+1);
				if (m_szPath != s2 && m_szPath!=_T(""))
				{
					CString msg;
					msg.Format(_T("You are trying to protect files from different paths. This may be confusing and is not recommended!"));
					MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
				}
				m_szPath = s2;
			}
			int cnt = rec.m_arFiles.GetSize()-1;
			t_FileInfo fi = rec.m_arFiles.GetAt(cnt);
			_tcscpy(fi.szName,s);
			rec.CutPath(&fi);
			//m_lstFiles.AddString(fi.szName);
			//m_arFiles.Add(fi.szName);
			//CString siz;
			//siz.Format("%u", (unsigned int)(fi.size/(1024*1024)));
			//m_arSizes.Add(siz);
			//m_arErrors.Add("ok");
		}
	}
	_tcscpy(rec.m_szPath, _T(""));
	//if (rec.m_arFiles.GetSize()>0)
	//{
	//	CString szFile = rec.m_arFiles.GetAt(0);
	//}
	InvalidateControls();
	UpdateList();
}

void CReedDlg::OnBtnCalculate() 
{
	UpdateData(TRUE);

	int percent = _ttoi(m_szPercentSize);
	if (percent>0 && percent<90 && rec.m_nTotalSize>0)
	{
		unsigned long s2 = (unsigned long) (rec.m_nTotalSize / (1024*1024));
		unsigned long rec_size = (unsigned long) ((double(s2)*(double(percent)*0.01)));
		if (rec_size<2) rec_size=2;
		if (rec_size>300) rec_size=300;
		m_szRecSize.Format(_T("%d"), rec_size);
	}
	UpdateData(FALSE);
	InvalidateControls();
}

void CReedDlg::OnBtnLoadRecovery() 
{
	UpdateData(TRUE);

	//g_DlgProgress->DoModal();
	//return;

	if (rec.m_bReadOnly)
	{
		rec.Clear();
		UpdateList();
		InvalidateControls();
		return;
	}
	CFileDialog FileDialog( TRUE, _T("RECOVERY"), _T("*.rcv"), 0, _T("Recovery Files (*.rcv)|*.rcv||"));
    if(FileDialog.DoModal()==IDCANCEL ) return;

	CString szRecFile = FileDialog.GetPathName(); 
	
	//BeginWaitCursor();
	//g_DlgProgress->ShowWindow(SW_SHOW);
	//EnableWindow(FALSE);
	m_szArgFileName = szRecFile;
	int rc = RunAsync(trdLoadRecovery);
	//int rc = rec.LoadRecovery2(szRecFile);
	//EnableWindow(TRUE);
	//g_DlgProgress->ShowWindow(SW_HIDE);
	//EndWaitCursor();
}

void CReedDlg::OnBtnLocate() 
{
}

void CReedDlg::OnBtnProtectDirectory() 
{
	//Select a folder
	UpdateData(TRUE);
	BROWSEINFO info;
	info.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = _T("Select directory to create recovery info file");
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

//	UpdateList();
//	InvalidateControls();

	//UpdateData(FALSE);
}

void CReedDlg::OnBtnProtectFiles() 
{
	UpdateData(TRUE);
	CString szProtectFile; 
	int i=0;

/*	int k = sizeof(UINT);
	char *sz="Ruslan Yus";
	char *sz1="hchenko";
	char *sz2="Ruslan Yushc";
	char *sz3="henko";
	UINT crc = rec.Get_CRC(sz, strlen(sz), 0);
	crc = rec.Get_CRC(sz1, strlen(sz1), crc);

	UINT crc2 = rec.Get_CRC(sz2, strlen(sz2), 0);
	crc2 = rec.Get_CRC(sz3, strlen(sz3), crc2);

	CString sss;
	sss.Format("CRC is %u CRC2 is %u", crc, crc2);
	MessageBox(sss,"test");
	*/

	int cnt = rec.m_arFiles.GetSize();
	
	if (cnt==0) 
	{
		MessageBox(_T("No files specified!"),_T("Error"), MB_ICONEXCLAMATION);
		return;
	}
	
	szProtectFile = m_szPath + _T("Recovery.rcv");

	if (cnt==1)
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(0);
		CString szSrcFile = fi.szName;
		szProtectFile = szSrcFile + _T(".rcv");
	}
	
	CFileDialog FileDialog( FALSE, _T("RECOVERY"), szProtectFile, OFN_OVERWRITEPROMPT, _T("Recovery Files|*.*||"));
	if(FileDialog.DoModal()==IDCANCEL ) return;

	szProtectFile = FileDialog.GetPathName();

	
	CString szFile;

	for (i=0; i<cnt; i++)
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(i);
		szFile = fi.szName;

		int rc = 0;//rec.AddFileInfo(szFile);

		if (rc!=0)
		{
			if (rc == -1)
			{
				CString msg;
				msg.Format(_T("File '%s' not found. Cannot continue!"),szFile);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == -4)
			{
				CString msg;
				msg.Format(_T("File '%s' pathname is too long. Cannot continue!"),szFile);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == -2)
			{
				CString msg;
				msg.Format(_T("Cannot open file '%s'. Cannot continue!"),szFile);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			return;
		}
	}
	if (szProtectFile==_T(""))
		return; 

	m_nArgSize = _ttoi(m_szRecSize);
	m_nArgSize*=1024*1024;
	m_szArgFileName = szProtectFile;
	RunAsync(trdProtectFiles);
	
	//BeginWaitCursor();
	//g_DlgProgress->ShowWindow(SW_SHOW);
	//EnableWindow(FALSE);
	//int rc = rec.CreateSolidRecovery2(_ttoi(m_szRecSize)*1024*1024, szProtectFile);
	//EnableWindow(TRUE);
	//g_DlgProgress->ShowWindow(SW_HIDE);
	//EndWaitCursor();

/*	BeginWaitCursor();
	rc = rec.SaveRecovery(szProtectFile);
	EndWaitCursor();
	if (rc!=0)
	{
		CString msg;
		msg.Format("Failed to save recovery info!");
		MessageBox(msg,"Error", MB_ICONEXCLAMATION);
		return; 
	}*/
}

void CReedDlg::OnBtnCheck() 
{
	int ret;
	m_bCheckNotAll = false;
	for (ret=0; ret<rec.m_arFiles.GetSize(); ret++)
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(ret);
		fi.checked = 1;
		rec.m_arFiles.SetAt(ret, fi);
	}

	int rc = RunAsync(trdCheckFiles);

	//BeginWaitCursor();
	//g_DlgProgress->ShowWindow(SW_SHOW);
	//EnableWindow(FALSE);
	//int rc = rec.Check2();
	//EnableWindow(TRUE);
	//g_DlgProgress->ShowWindow(SW_HIDE);
	//EndWaitCursor();

//	UpdateList();
//  InvalidateControls();
//	UpdateList();
//	InvalidateControls();
}

void CReedDlg::OnBtnCheckThis() 
{
	UINT flag = LVNI_SELECTED;
	int nRow;
	int cnt=0;
	m_bCheckNotAll = false;
	for( nRow = m_lstFiles.GetItemCount()-1; nRow >=0; nRow -- )
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(nRow);

		if ((m_lstFiles.GetItemState(nRow, flag) & flag) != flag ) 
		{
			fi.checked = 0;
			m_bCheckNotAll = true;
		}
		else
		{
			fi.checked = 1;
			cnt++;
		}
			
		rec.m_arFiles.SetAt(nRow, fi);
	}

	if (cnt>0)
		int rc = RunAsync(trdCheckFiles);
}

void CReedDlg::OnBtnRecover() 
{
	int ret = 0;

	if (m_nRadio3==0)
	{
		CString msg;
		msg.Format(_T("Are you sure you want to recover files in place?"));
		ret = MessageBox(msg,_T("Quersion"), MB_ICONQUESTION + MB_YESNOCANCEL);
	}
	else
	{
		if (m_szRecoverPath==_T(""))
		{
			MessageBox(_T("Please, specify a path to recover to!"),_T("Quersion"), MB_ICONEXCLAMATION);
			return;
		}
		else
		{
			CString msg;
			msg.Format(_T("Are you sure you want to recover files to %s?"), m_szRecoverPath);
			ret = MessageBox(msg,_T("Quersion"), MB_ICONQUESTION + MB_YESNOCANCEL);
		}
	}	

	if (ret!=IDYES) return;

	if (m_nRadio3==0)
		rec.m_szRecoverPath = _T("");
	else
		rec.m_szRecoverPath = m_szRecoverPath;

	for (ret=0; ret<rec.m_arFiles.GetSize(); ret++)
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(ret);
		fi.to_recover = 1;
		rec.m_arFiles.SetAt(ret, fi);
	}

	int rc = RunAsync(trdRecoverFiles);

	//BeginWaitCursor();
	//g_DlgProgress->ShowWindow(SW_SHOW);
	//EnableWindow(FALSE);
	//int rc = rec.Recover2();
	//EnableWindow(TRUE);
	//g_DlgProgress->ShowWindow(SW_HIDE);
	//EndWaitCursor();

}

void CReedDlg::OnBtnRemove() 
{
	//int ind = m_lstFiles.GetHotItem();

	UINT flag = LVNI_SELECTED;
	int nRow;
	bool bFound=false;
	for( nRow = m_lstFiles.GetItemCount()-1; nRow >=0; nRow -- )
	{
		if ((m_lstFiles.GetItemState(nRow, flag) & flag) != flag ) 
			continue;
		bFound=true;
		m_lstFiles.DeleteItem(nRow);
		t_FileInfo fi = rec.m_arFiles.GetAt(nRow);
		rec.m_nTotalSize-=fi.size;
		rec.m_arFiles.RemoveAt(nRow);
	}
	if (!bFound)
	{
		for( nRow = m_lstFiles.GetItemCount()-1; nRow >=0; nRow -- )
		{
			m_lstFiles.DeleteItem(nRow);
			t_FileInfo fi = rec.m_arFiles.GetAt(nRow);
			rec.m_nTotalSize-=fi.size;
			rec.m_arFiles.RemoveAt(nRow);
		}
		rec.m_nTotalSize=0;
	}
	if (m_lstFiles.GetItemCount()==0)
		m_szPath=_T("");
	InvalidateControls();


/*	if (ind>=0)
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(ind);
		rec.m_nTotalSize-=fi.size;
		m_lstFiles.DeleteItem(ind);
		rec.m_arFiles.RemoveAt(ind);
	}
*/

//	int ind = m_lstFiles.GetCurSel();
//	if (ind!=-1)
//		m_lstFiles.DeleteString(ind);
}

void CReedDlg::OnKillfocusEditRecSize() 
{
	InvalidateControls();
}

void CReedDlg::OnKillfocusEditSectorSize() 
{
	InvalidateControls();
}

void CReedDlg::UpdateList() 
{
	if ( m_lstFiles.GetItemCount() != 0)
		m_lstFiles.DeleteAllItems();

	int m_nCntFiles = rec.m_arFiles.GetSize();
	for (int i=0; i<m_nCntFiles; i++)
	{
		CString st;
		t_FileInfo fi = rec.m_arFiles.GetAt(i);
		st = fi.szName;
		m_lstFiles.InsertItem(i, st);
		m_lstFiles.SetItemText(i, 0, st);

		if (fi.size<1024*32)
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
		m_lstFiles.SetItemText(i, 2, st);
	}		
	ListView_SetExtendedListViewStyle( m_lstFiles.m_hWnd, LVS_EX_FULLROWSELECT );
}

void CReedDlg::OnBnClickedBtnRecoverThis()
{
	UINT flag = LVNI_SELECTED;
	int nRow;
	int cnt=0;

	if (m_nRadio3==1)
	{
		if (m_szRecoverPath==_T(""))
		{
			MessageBox(_T("Please, specify a path to recover to!"),_T("Warning"), MB_ICONEXCLAMATION);
			return;
		}
	}	

	if (m_nRadio3==0)
		rec.m_szRecoverPath = _T("");
	else
		rec.m_szRecoverPath = m_szRecoverPath;

//	if (!rec.isAllChecked())
//	{
//		CString msg;
//		msg.Format(_T("All data must be checked first!"));
//		MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
//	}

	for( nRow = m_lstFiles.GetItemCount()-1; nRow >=0; nRow -- )
	{
		t_FileInfo fi = rec.m_arFiles.GetAt(nRow);

		if ((m_lstFiles.GetItemState(nRow, flag) & flag) != flag ) 
			fi.to_recover = 0;
		else
		{
			fi.to_recover = 1;
			cnt++;
		}
		rec.m_arFiles.SetAt(nRow, fi);
	}

	if (cnt==0) return;

	int rc = RunAsync(trdRecoverFiles);
}

void CReedDlg::OnBnClickedRadio1()
{
	InvalidateControls();
}

void CReedDlg::OnBnClickedRadio2()
{
	InvalidateControls();
}

void CReedDlg::OnEnChangeEditPercentSize()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CReedDlg::OnEnKillfocusEditPercentSize()
{
	InvalidateControls();
}


void CReedDlg::OnBtnRecoverPath() 
{
	UpdateData(TRUE);
	BROWSEINFO info;
	info.hwndOwner = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = _T("Select directory to copy recover files to");
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

//

int CReedDlg::RunAsync (StaticFunction fn)
{
	::EnterCriticalSection(&m_CS);
	bool bRunning = m_bRunning;
	g_DlgProgress->m_bNeedTerminate = false;
	::LeaveCriticalSection(&m_CS);

	if (bRunning) return -1;

	g_DlgProgress->ShowWindow(SW_SHOW);
	EnableWindow(FALSE);
	//BeginWaitCursor();	
	//AfxGetApp()->DoWaitCursor(1);
	//::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

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
		//EndWaitCursor();
		//AfxGetApp()->DoWaitCursor(-1);
		return -1;
	}

	//PostMessage(WM_SETCURSOR, (WPARAM) this->m_hWnd);

	return 0;
}

unsigned _stdcall CReedDlg::trdLoadRecovery (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_LOADRECOVERY;

	int rc = d->rec.LoadRecovery2(d->m_szArgFileName);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CReedDlg::trdAddDir (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_ADDDIR;

	int rc = d->rec.AddDir(d->m_szArgDir);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CReedDlg::trdCheckFiles (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_CHECKFILES;

	int rc = d->rec.Check2();

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

unsigned _stdcall CReedDlg::trdProtectFiles (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_PROTECTFILE;

	int rc = d->rec.CreateSolidRecovery2(d->m_nArgSize, d->m_szArgFileName);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;

}

unsigned _stdcall CReedDlg::trdProtectDir (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_PROTECTDIR;

	int rc = d->rec.CreateSolidRecovery2(d->m_nArgSize, d->m_szArgFileName);

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}


unsigned _stdcall CReedDlg::trdRecoverFiles (LPVOID lpParameter)
{
	if (lpParameter == NULL) return -1;
	CReedDlg *d = (CReedDlg * ) lpParameter;
	d->m_nOperationType = _OP_RECOVER;

	int rc = d->rec.Recover2();

	::EnterCriticalSection(&d->m_CS);
	d->m_nReturnValue = rc;
	::LeaveCriticalSection(&d->m_CS);

	d->PostMessage(WM_COMMAND, ID_CUSTOM_COMPLETED);

	return 0;
}

void CReedDlg::OnComplete()
{
	EnableWindow(TRUE);
	g_DlgProgress->ShowWindow(SW_HIDE);
	//EndWaitCursor();
	//AfxGetApp()->DoWaitCursor(-1);
	//::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));
	//CWaitCursor wait;

	int rc = m_nReturnValue;

	if (rc == E_PROTECTOR_TERMINATED)
	{
		InvalidateControls();
		UpdateList();
		MessageBox(_T("Process was interrupted!"),_T("Info"), MB_ICONEXCLAMATION);
		return;
	}
	if (rc == E_UNEXPECTED)
	{
		CString msg;
		msg.Format(_T("Unexpected Error!"));
		MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
	}

	if (m_nOperationType==_OP_LOADRECOVERY)
	{
		if (rc!=0)
		{
			if (rc == E_CRC_ERROR)
			{
				CString msg;
				msg.Format(_T("CRC Error! Bad Recovery File!"));
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
			return; 
		}
		if (rec.m_nTotalSize==0)
			return;

		m_szRecSize.Format(_T("%d"), rec.m_nRecoverySize/(1024*1024));
		m_szSectorSize.Format(_T("%d"), rec.m_nRecoverySectorSize/1024);
		m_szPercentSize.Format(_T("%d"), rec.m_nRecoverySize/rec.m_nTotalSize);
		UpdateData(FALSE);
	}

	if (m_nOperationType==_OP_ADDDIR)
	{
		if (m_nRadio==1)
		{
			UpdateData(TRUE);
			if (rec.m_nTotalSize>0)
			{
				double percent = double(_ttoi(m_szPercentSize))*0.01;
				int rec_size = int((double(rec.m_nTotalSize)*percent)/double(1024*1024));
				int sec_size=4;
				if (rec_size<1) rec_size = 1;
				if (rec_size>10000) rec_size = 10000;
				if (rec_size>10) sec_size=4;
				if (rec_size>50) sec_size=8;
				if (rec_size>100) sec_size=16;
				if (rec_size>500) sec_size=32;
				if (rec_size>1000) sec_size=64;
				if (rec_size>2000) sec_size=128;
				m_szRecSize.Format(_T("%d"),rec_size);
				m_szSectorSize.Format(_T("%d"),sec_size);
			}
			UpdateData(FALSE);
		}

		if (rc!=0)
		{
			MessageBox(_T("Failed to scan directory!"),_T("Error"), MB_ICONEXCLAMATION);
			return;
		}

		CString msg;
		msg.Format(_T("%u MB in %d files. Proceed?"), (unsigned int)(rec.m_nTotalSize/(1024*1024)), rec.m_arFiles.GetSize());
		int ret = MessageBox(msg,_T("Quersion"), MB_ICONQUESTION + MB_YESNOCANCEL);

		if (ret!=IDYES)
		{
			rec.Clear();
			UpdateList();
			InvalidateControls();
			return;
		}

		CString szProtectFile = rec.m_szPath;
		szProtectFile += _T("Directory.rcv");

		m_szArgFileName = szProtectFile;
		m_nArgSize = _ttoi(m_szRecSize)*1024*1024;
		int rc = RunAsync(trdProtectDir);

		//InvalidateControls();
		//UpdateList();
		return;

		//BeginWaitCursor();
		//g_DlgProgress->ShowWindow(SW_SHOW);
		//EnableWindow(FALSE);
		//rc = rec.CreateSolidRecovery2(_ttoi(m_szRecSize)*1024*1024, szProtectFile);
		//EnableWindow(TRUE);
		//g_DlgProgress->ShowWindow(SW_HIDE);
		//EndWaitCursor();
	}

	if (m_nOperationType==_OP_PROTECTDIR)
	{
		if (rc!=0)
		{
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
			rec.Clear();
			InvalidateControls();
			UpdateList();
			return; 
		}

		CString msg;
		msg.Format(_T("Recovery file saved in %s!"), m_szArgFileName);
		MessageBox(msg,_T("Information"), MB_ICONINFORMATION);

		rec.Clear();
	}

	if (m_nOperationType==_OP_PROTECTFILE)
	{
		if (rc!=0)
		{
			if (rc == E_SMALL_SIZE)
			{
				CString msg;
				msg.Format(_T("Too small size. Cannot continue!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_NO_FILES)
			{
				CString msg;
				msg.Format(_T("No Files Selected. Cannot continue!"));
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			if (rc == E_FILE_CANNOT_WRITE)
			{
				CString msg;
				msg.Format(_T("Cannot write to file %s!"), m_szArgFileName);
				MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
			}
			return; 
		}
		CString msg;
		msg.Format(_T("Recovery file saved in %s!"), m_szArgFileName);
		MessageBox(msg,_T("Information"), MB_ICONINFORMATION);
	}

	if (m_nOperationType==_OP_CHECKFILES)
	{
		InvalidateControls();
		UpdateList();

		if (rc==0)
		{
			MessageBox(_T("Check complete. No errors found!"),_T("Info"), MB_ICONINFORMATION);
		}
		else
		{
			if (m_bCheckNotAll)
			{
				CString s1;
				NumbersToBytes(rec.m_nCntRecoverable, rec.m_nRecoverySectorSize, s1);
				CString msg;
				msg.Format(_T("Found CRC Errors! To recover files you'll need to run a FULL check."), s1);
				MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
			}
			else
			{
				if (rec.m_nCntRecoverable>0)
				{
					if (rec.m_nCntNotRecoverable==0)
					{
						CString s1;
						NumbersToBytes(rec.m_nCntRecoverable, rec.m_nRecoverySectorSize, s1);
						CString msg;
						msg.Format(_T("Found %s of incorrect data. All are recovarable!"), s1);
						MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
					}
					else
					{
						CString s1;
						NumbersToBytes(rec.m_nCntRecoverable, rec.m_nRecoverySectorSize, s1);
						CString s2;
						NumbersToBytes(rec.m_nCntNotRecoverable, rec.m_nRecoverySectorSize, s2);
						CString msg;
						msg.Format(_T("Found %s recovarable and %s unrecoverable data!"), s1, s2);
						MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
					}
				}
				else
				{
					CString msg;
					msg.Format(_T("Incorrect data too big. Cannot recover. Sorry!"));
					MessageBox(msg,_T("Warning"), MB_ICONEXCLAMATION);
				}
			}
		}
		return;
	}
	if (m_nOperationType==_OP_RECOVER)
	{
		InvalidateControls();
		UpdateList();

		if (rc==0)
		{
			CString msg;
			msg.Format(_T("File(s) successfully recoveres!"));
			MessageBox(msg,_T("Information"), MB_ICONINFORMATION);
		}
		else
		{
			CString msg;
			msg.Format(_T("Recover failed!"));
			MessageBox(msg,_T("Error"), MB_ICONEXCLAMATION);
		}
		return;
	}

	InvalidateControls();
	UpdateList();
}

BOOL CReedDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bRunning)
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));	
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}


void CReedDlg::OnEnChangeEditRecSize()
{
	InvalidateControls();
}
