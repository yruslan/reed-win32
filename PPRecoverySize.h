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


// CPPRecoverySize dialog
#include "reed.h"
#include "afxcmn.h"

class CPPRecoverySize : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPRecoverySize)

public:
	CPPRecoverySize();
	virtual ~CPPRecoverySize();

// Dialog Data
	enum { IDD = IDD_PP_PROTECT_SIZE };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()

	CFont m_pBoldFont;

public:
	void InvalidateControls();

	CString m_szFolder;
	CString m_szFiles;
	CString m_szSize;
	CString m_szRecFile;
	int m_nRecSizeMB;
	int m_nSliderPercent;
	CSliderCtrl m_cSliderPercent;

	afx_msg void OnEnKillfocusEditRecSize();
	afx_msg void OnEnKillfocusEditPercentSize();
	afx_msg void OnBnClickedBtnRecoverPath();
	afx_msg void OnNMCustomdrawSliderPercent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnList2();
};
