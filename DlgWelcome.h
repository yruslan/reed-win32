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
	afx_msg void OnBnClickedBtnProtectDir();
	afx_msg void OnBnClickedBtnAbout();
	afx_msg void OnComplete();
};
