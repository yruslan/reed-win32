/*
Copyright (c) 2009-2018, Ruslan Yushchenko
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

#include "StdAfx.h"
#include <Shlobj.h>
#include "mfc_helpers.h"
#include <algorithm>

int MFC_FileDialog1(CString &szPathFileOut, int &nFilterIndexOut, BOOL bOpenFileDialog, LPCTSTR lpszDefExt,
					DWORD dwFlags, LPCTSTR lpszFilter,	LPCTSTR lpszDefaultPathFile)
{

	CString szFolder;
	CString szFileNameIn;
	CFileDialog D(bOpenFileDialog, lpszDefExt, NULL, dwFlags, lpszFilter );
	
	if (_tcscmp(lpszDefaultPathFile,_T(""))!=0)
	{
		TCHAR drive[_MAX_DRIVE]; 
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		_tsplitpath(lpszDefaultPathFile, drive, dir, fname, ext);
		_tmakepath(szFolder.GetBuffer(_MAX_PATH), drive, dir, _T(""), _T("")); szFolder.ReleaseBuffer();		
		szFileNameIn.Format(_T("%s%s"), fname, ext);
		D.m_ofn.lpstrInitialDir = szFolder.GetBuffer(MAX_PATH);
		D.m_ofn.lpstrFile = szFileNameIn.GetBuffer(MAX_PATH);
	}

	int ret = D.DoModal();

	if (_tcscmp(lpszDefaultPathFile,_T(""))!=0)
	{
		szFolder.ReleaseBuffer();
		szFileNameIn.ReleaseBuffer();
	}

	if( ret == IDCANCEL )
		return ret;
	else
	{
		szPathFileOut = D.GetPathName();
		nFilterIndexOut = D.m_ofn.nFilterIndex;
		return ret;
	}
}

static CString _szInitFolder;

static int CALLBACK BrowseCallbackProc(HWND hwnd,
    UINT uMsg,LPARAM lParam,LPARAM lpData)
{
    // Look for BFFM_INITIALIZED
    if(uMsg == BFFM_INITIALIZED)
    {
        SendMessage(hwnd, BFFM_SETSELECTION,
            TRUE,(LPARAM) ((LPCTSTR)_szInitFolder));
    }

    return 0;
}

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
                                        // Caller needs to call OleInitialize() before using this API
#endif
#ifndef BIF_USENEWUI
#define BIF_USENEWUI	(BIF_NEWDIALOGSTYLE | BIF_EDITBOX)
#endif

int MFC_FolderDialog(CString &szPathOut,
					 HWND hSafeHWnd,
					 LPCTSTR lpszTitle,
					 LPCTSTR lpszInitialDir,
					 bool bAllowCreateNew)
{
	BROWSEINFO info;
	info.hwndOwner = hSafeHWnd;
	info.pidlRoot = NULL;
	_TCHAR szBuf[_MAX_PATH];
	info.pszDisplayName = szBuf;
	info.lpszTitle = lpszTitle;
	_szInitFolder = lpszInitialDir;
	info.ulFlags = bAllowCreateNew ? BIF_USENEWUI | BIF_VALIDATE : BIF_VALIDATE;
	info.lpfn = BrowseCallbackProc;
	info.lParam = NULL;
	info.iImage = 0;
	LPITEMIDLIST lpH = SHBrowseForFolder(&info);
	if(!lpH) return IDCANCEL;
	if(SHGetPathFromIDList(lpH,szBuf)) 
	{
		szPathOut = szBuf;
	}
	CoTaskMemFree(lpH);
	return IDOK;
}

CString ReplacePathName(const CString &szPathFileName, const CString &szNewPath)
{
	if (szPathFileName.IsEmpty()) return _T("");

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	CString szNewFileName;

	_tsplitpath(szPathFileName, drive, dir, fname, ext);

	szNewFileName = fname;
	szNewFileName += ext;

	if (szNewPath.IsEmpty()) return szNewFileName;

	if (szNewPath[szNewPath.GetLength()-1]=='\\')
		szNewFileName = szNewPath + szNewFileName;
	else
		szNewFileName = szNewPath + _T("\\") + szNewFileName;
	return szNewFileName;
}

CString ReplacePathFile(const CString &szPathFileName, const CString &szNewFileName)
{
	if( szPathFileName.IsEmpty() ) return szNewFileName;

	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	TCHAR drive2[_MAX_DRIVE];
	TCHAR dir2[_MAX_DIR];
	TCHAR fname2[_MAX_FNAME];
	TCHAR ext2[_MAX_EXT];

	_tsplitpath( szPathFileName, drive, dir, fname, ext );
	_tsplitpath( szNewFileName, drive2, dir2, fname2, ext2 );
	_tmakepath( path_buffer, drive, dir, fname2, ext2 );

	CString s(path_buffer);
	return s;
}


CString GetFileByPathName(const CString &szPathFileName)
{
	return ReplacePathName(szPathFileName, _T(""));
}

void GetListOfDirEntries(const CString &szPathFileName, CStringArray &arDirs, bool bDirs/*=false*/)
{
	arDirs.RemoveAll();
	CString szPathCopy(szPathFileName), szMask;

	if (szPathCopy.Right(1) != _T("\\")) szPathCopy += _T("\\");
	szMask = szPathCopy + "*.*";
	
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(szMask, &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			szMask = fd.cFileName;
			if (szMask == _T(".") || szMask == _T("..")) continue;
			bool bDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
			if (!(bDirs ^ bDir))
			{
				szMask = szPathCopy + fd.cFileName;
				if (bDirs) szMask += _T("\\");
				arDirs.Add(szMask);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	if (arDirs.GetSize() > 1)
		std::sort(&arDirs[0], &arDirs[0] + arDirs.GetSize());
}

bool MFC_OpenBrowserURL(const CString &szURL)
{
	// Call ShellExecute to run the file.
	// For an URL, this means opening it in the browser.
	//
	HINSTANCE h = ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
	if ((UINT)h > 32)
		return TRUE;
	return FALSE;
}

int MFC_DeleteToRecycleBin(const CString &szFileOrDirectory)
{
	TCHAR szBuf1[MAX_PATH + 10];
	TCHAR *szBuf2 = NULL;

	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
	ZeroMemory(szBuf1, MAX_PATH + 10);
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE; 
	if (szFileOrDirectory.GetLength()>MAX_PATH)
	{
		szBuf2 = new TCHAR[szFileOrDirectory.GetLength() + 10];
		ZeroMemory(szBuf2, szFileOrDirectory.GetLength() + 10);
		lstrcpy(szBuf2, szFileOrDirectory);
		FileOp.pFrom = szBuf2;
	}
	else
	{
		lstrcpy(szBuf1, szFileOrDirectory); // The string MUST end with TWO null characters
		FileOp.pFrom = szBuf1;
	}
	FileOp.pTo = NULL;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_ALLOWUNDO;
	FileOp.hNameMappings = NULL;
	int nRet = SHFileOperation(&FileOp);

	if (szFileOrDirectory.GetLength()>MAX_PATH)
		delete [] szBuf2;

	return nRet;
}

CString MFC_GetErrorMessage(DWORD err)
{
	CString s; 
	LPTSTR szErr;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		(LPTSTR) &szErr,
		0,
		NULL))
		return s;
	
	//TRACE("ERROR: %s: %s", msg, err);
	static char buffer[1024];
	_snprintf(buffer, sizeof(buffer), "ERROR: %d: %s\n", err, szErr);
	//OutputDebugString(buffer);
	LocalFree(szErr);
	s = buffer;
	return s;
}
