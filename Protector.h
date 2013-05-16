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

#ifndef _PROTECTOR_H_UNCLUDED_16_11_2009
#define _PROTECTOR_H_UNCLUDED_16_11_2009

#include "blob.h"

typedef unsigned __int64 FILESIZE;

#define BLOCK_SIZE	32768
#define MAX_PATH_PROT	512

#define _INSIDE_MFC_APP


#define E_FILE_NOT_FOUND		-1
#define E_FILE_NOT_OPENS		-2
#define E_FILE_CANNOT_WRITE		-3
#define E_FILE_CANNOT_READ		-4
#define E_NO_FILES				-5
#define E_SMALL_SIZE			-6
#define E_CRC_ERROR				-7
#define E_VERSION_OLD			-8
#define E_FORMAT_ERROR			-9
#define E_WRONG_OPERATION		-10
#define E_WRONG_OUT_OF_MEMORY	-11
#define E_EMPTY_RECOVERY_FILE	-12
#define E_UNRECOVERABLE			-13
#define E_ZERO_SIZED_FILE		-14
#define E_PATH_TOO_LONG			-15
#define E_NOT_ENOUGH_SPACE		-16
#define E_COPY_FAILED			-17
#define E_TMP_FILENAME			-18
#define E_PROTECTOR_TERMINATED	-100
#define E_XUNEXPECTED			-101


struct t_FileInfo
{
	TCHAR szName[MAX_PATH_PROT];
	FILESIZE	size;
	UINT nCrc;
	int status;
	int to_recover;
	int checked;
	int errors;
    __time64_t st_mtime;
};

struct t_pattern
{
	int size;
	int file;
	FILESIZE block;
};

UINT Get_CRC(LPCSTR szData, int nLength, UINT nCRC/*=0*/);

class CProtector 
{
public:
	CProtector();
	virtual ~CProtector();

	//int GetListOfFiles(LPCTSTR szDirectory);

	int CreateSolidRecovery2(FILESIZE rec_size, LPCTSTR szFileName);
	int SaveRecovery(LPCTSTR szFileName);
	int LoadRecovery2(LPCTSTR szFileName);
	int Check2();
	int Recover2(int spec_file=-1);
	int CheckAndRecoverSmall();
	void CutPath(t_FileInfo *fi);
	void CutBasePath(t_FileInfo *fi);
	
	void Clear();
	int AddFileInfo(LPCTSTR szFileName);
	int AddDir(LPCTSTR szPath);
	bool isAllChecked();
	int GetTmpFileName(CString &szFileName);
	bool isOrigPathCorrect();

	UINT CProtector::Get_CRC(void *_szData, int nLength, UINT nCRC/*=0*/);

	CArray<t_FileInfo,t_FileInfo&> m_arFiles;

	int m_nRecoveryBlockSize;
	FILESIZE m_nRecoverySize;	
	int m_nCntRecoverable;
	int m_nCntNotRecoverable;
	FILESIZE m_nTotalSize; 
	TCHAR m_szPath[MAX_PATH_PROT];

	bool m_bReadOnly;
	bool m_bCreateSubdirs;
	CString m_szRecoverPath;
	CString m_szRecFileName;

private:
	void Init_CRC32_Table();
	UINT Reflect(UINT ref, char ch);
	
	UINT crc32_table[256];

	int AddFileToRecovery(t_FileInfo fi);
	int CheckFile(t_FileInfo &fi);

	CString m_szTempFile;
	FILE *fRest;

	int m_nCntBlocks;
	int m_nCurrentBlock;
	FILESIZE m_nCurrentBlockA;
	t_pattern *m_npattern;
	char *m_szRecovery;
	char *m_szRestSum;
		
	CBlob m_blbCRC;
};


#endif //_PROTECTOR_H_UNCLUDED_16_11_2009