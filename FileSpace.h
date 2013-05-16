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

#ifndef _FILESPACE_H_UNCLUDED_21_11_2009
#define _FILESPACE_H_UNCLUDED_21_11_2009

typedef unsigned __int64 FILESIZE;

#define E_FILESPACE_NOTFOUND		-1
#define E_FILESPACE_CANNOT_OPEN		-2
#define E_FILESPACE_CANNOT_WRITE	-3
#define E_FILESPACE_UNEXPECTED		-4
#define E_FILESPACE_NOT_STARTED		-5
#define E_FILESPACE_EOF				-6
#define E_FILESPACE_NO_PERMISSION	-7

class CFileSpace 
{
public:
	CFileSpace();
	~CFileSpace();

	int SetBlockSize(int block_size);
	int GetBlockSize();

	int EnsurePathExist(LPCTSTR szFileName);

	int AddFile(LPCTSTR szFileName, FILESIZE use_size =0, int ignore =0);
	bool isIgnored();
	int GetCurrentFile(CString &szFileName);
	FILESIZE GetAllSize();

	bool eof();
	FILESIZE GetPos();
	FILESIZE GetBlock();
	int SetBlock(FILESIZE nBlock, bool allow_writes/*=false*/);

	int ReadBlock(void *buf);
	int WriteBlock(void *buf);

	void Close();

	int GetFileNumberAndPos(FILESIZE nPos, int &nFile, FILESIZE &nFilePos);
	int GetFileName(int n, CString &szFileName);

private:

	bool m_bCurIgnored;

	FILE *file;

	CStringArray m_arFiles;
	CArray<FILESIZE> m_arSizes;
	CArray<FILESIZE> m_arRealSizes;
	CArray<int>	m_arIgnore;

	int m_nBlockSize;
	int m_nFilesCount;
	FILESIZE m_nTotalSize;

	int m_nCurrentFile;
	FILESIZE m_nCurrentPos;
	FILESIZE m_nCurrentFilePos;
	bool m_bReadOnly;
};


#endif //_FILESPACE_H_UNCLUDED_21_11_2009
