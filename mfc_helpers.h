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

#ifndef _MFC_HELPERS_H_INCLUDED_2018_05_20
#define _MFC_HELPERS_H_INCLUDED_2018_05_20

// Select 1 file using common dialogt. You can specify the dialog title, filter and the initial directory.
int MFC_FileDialog1(CString &szPathFileOut, int &nFilterIndexOut,
					BOOL bOpenFileDialog = TRUE,
					LPCTSTR lpszDefExt = _T("Seismic"),
					DWORD dwFlags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					LPCTSTR lpszFilter = _T("All Files(*.*)|*.*||"),
					LPCTSTR lpszDefaultPathFile = _T(""));

// Select a folder using common folder selection dialog specifying the initial path. 
int MFC_FolderDialog(CString &szPathOut,
					 HWND hSafeHWnd,
					 LPCTSTR lpszTitle = _T("Select a folder"),
					 LPCTSTR lpszInitialDir = _T(""),
					 bool bAllowCreateNew = true);

// Replace path to a file leaving the original file name.
CString ReplacePathName(const CString &szPathFileName, const CString &szNewPath);

// Replace file name leaving the original path.
CString ReplacePathFile(const CString &szPathFileName, const CString &szNewFileName);

// Return file name by a full path name.
CString GetFileByPathName(const CString &szPathFileName);

// Get the list of files/directories by recursively traversing a path name.
void GetListOfDirEntries(const CString &szPathFileName, CStringArray &arDirs, bool bDirsOnly=false);

// Open an URL in the default browser.
bool MFC_OpenBrowserURL(const CString &szURL);

// Delete a file or a directory to Recycle Bin. Returns 0 on success and GetLastError() on failure.
int MFC_DeleteToRecycleBin(const CString &szFileOrDirectory);

// Return Windows system error text by an error code.
CString MFC_GetErrorMessage(DWORD err);

#endif //_MFC_HELPERS_H_INCLUDED_2018_05_20
