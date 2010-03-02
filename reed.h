// reed.h : main header file for the REED application
//

#if !defined(AFX_REED_H__DDD63B33_75F7_4A7A_8729_6A77263B8953__INCLUDED_)
#define AFX_REED_H__DDD63B33_75F7_4A7A_8729_6A77263B8953__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CReedApp:
// See reed.cpp for the implementation of this class
//

class CReedApp : public CWinApp
{
public:
	CReedApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReedApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CReedApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REED_H__DDD63B33_75F7_4A7A_8729_6A77263B8953__INCLUDED_)
