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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "FileSpace.h"
#include "Protector.h"
#ifdef _INSIDE_MFC_APP
#include "reed.h"
#include "DlgProgress.h"

extern CDlgProgress *g_DlgProgress;
#endif

#define RCV_CUR_VERSION		1
#define RCV_MAX_VERSION		100
#define SEG_SIZE	200
#define HEADER_SIZE	24

//extern "C" int __cdecl _fseeki64(FILE *, __int64, int);
//extern "C" __int64 __cdecl _ftelli64(FILE *);

#define FILE_VERSION	4

int _xoradd(char *sum, const char *add, int len)
{
	if (len<=0) return -1;
	for (int i=0; i<len; i++)
	{
		sum[i] ^= add [i];
	}
	return 0;
}

CProtector::CProtector()
{
	m_szRecovery = NULL;
	m_npattern = NULL;
	m_szRestSum = NULL;
	m_nRecoveryBlockSize = BLOCK_SIZE;
	m_nRecoverySize = 10485760;
	m_nCntBlocks = 0;
	m_nCurrentBlock = 0;
	m_nCurrentBlockA = 0;
	m_nCntRecoverable = 0;
	m_nCntNotRecoverable = 0;
	m_bReadOnly = false;
	m_nTotalSize = 0;
	m_szRecFileName = _T("");
	fRest = NULL;
	m_szRecoverPath = _T("");
	m_bCreateSubdirs = true;
	m_bAllSrcFilesExist = false;
	m_nFilesExistCount = 0;
	Init_CRC32_Table();
}

CProtector::~CProtector()
{
	if (m_szRecovery!=NULL)
	{
		delete [] m_szRecovery;
		m_szRecovery = NULL;
	}
	if (m_szRestSum!=NULL)
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
	}
	if (m_npattern!=NULL)
	{
		delete [] m_npattern;
		m_npattern = NULL;
	}
	if (fRest!=NULL)
	{
		fclose(fRest);
		fRest = NULL;
	}
	if (m_szTempFile!=_T(""))
	{
		_tunlink(m_szTempFile);
		m_szTempFile=_T("");
	}
}

void CProtector::Clear()
{
	m_arFiles.RemoveAll();
	if (m_szRecovery!=NULL)
	{
		delete [] m_szRecovery;
		m_szRecovery = NULL;
	}
	_tcscpy(m_szPath, _T(""));
	m_blbCRC.SetSize(0);
	m_nRecoveryBlockSize = BLOCK_SIZE;
	m_nRecoverySize = 10485760;
	m_nCntBlocks = 0;
	m_nCurrentBlock = 0;
	m_nCurrentBlockA = 0;
	m_nCntRecoverable = 0;
	m_nCntNotRecoverable = 0;
	m_bReadOnly = false;
	m_nTotalSize = 0;
	if (fRest!=NULL)
	{
		fclose(fRest);
		fRest = NULL;
	}
	
	if (m_szTempFile!=_T(""))
	{
		_tunlink(m_szTempFile);
		m_szTempFile = _T("");
	}
	
	m_szRecFileName = _T("");
}

void CProtector::Init_CRC32_Table()
{
	UINT ulPolynomial = 0x04c11db7;

	for(int i = 0; i <= 0xFF; i++)
	{
		crc32_table[i]=Reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		crc32_table[i] = Reflect(crc32_table[i], 32);
	}
}

UINT CProtector::Reflect(UINT ref, char ch)
{
	UINT value(0);

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

UINT CProtector::Get_CRC(void * _szData, int nLength, UINT nCRC=0)
{
	const unsigned char *szData = (const unsigned char *) _szData;
	UINT  ulCRC;//(0xffffffff);
//	if (nCRC==0) 
//		ulCRC = 0xffffffff;
//	else
		ulCRC = nCRC;

	int sum = 0;
	while(nLength--)
	{
		sum+=*szData;
		ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *szData++];
	}
	int k = sum;
	return ulCRC;
}

int CProtector::AddFileInfo(LPCTSTR szFileName)
{
	if (_tcslen(szFileName)>MAX_PATH_PROT-1)
		return E_PATH_TOO_LONG;

	FILESIZE file_size;
	struct _stati64 statfile;
	if( _tstati64( szFileName, &statfile ) == 0 )
	{
		file_size = statfile.st_size;
		if( file_size == 0 )
			return E_ZERO_SIZED_FILE;
	}
	
	FILE *f = NULL;
	f = _tfopen(szFileName,_T("rb"));
	if (f == NULL)
		return E_FILE_NOT_OPENS;

	fclose(f);

	struct t_FileInfo fi;
//	fi.st_ctime = statfile.st_ctime;
	fi.st_mtime = statfile.st_mtime;

	//fi.order=-1;
	_tcscpy(fi.szName, szFileName);
	fi.size = file_size;
	fi.status=0;
	m_arFiles.Add(fi);
	m_nTotalSize+=fi.size;
	return 0;
}

//int CProtector::GetListOfFiles(LPCTSTR szDirectory)
//{
//	return 0;
//}

int CProtector::CreateSolidRecovery2(FILESIZE size, LPCTSTR szFileName)
{
	if (size<3*m_nRecoveryBlockSize)
		return E_SMALL_SIZE; //Too small size

	if (m_arFiles.GetSize()==0)
		return E_NO_FILES; //No files!

	FILE *f = _tfopen (szFileName, _T("wb"));
	if (f==NULL) return E_FILE_CANNOT_WRITE;

	//Calc Numblocks
	if (size % m_nRecoveryBlockSize == 0)
	{
		m_nCntBlocks = int(size / m_nRecoveryBlockSize);
		m_nRecoverySize = size;
	}
	else
	{
		m_nCntBlocks = int(size / m_nRecoveryBlockSize + 1);
		m_nRecoverySize = size + m_nRecoveryBlockSize - (size % m_nRecoveryBlockSize);
	}
	m_nCurrentBlock = 0;
	m_nCurrentBlockA = 0;

	//Writing Header
	unsigned int uCRC = 0;
	int version = FILE_VERSION;
	int cnt = m_arFiles.GetSize();
	char sig[5]="$RCV";
	fwrite(sig,1,4,f);	
	uCRC = Get_CRC(sig, 4, uCRC);
	fwrite(&version,sizeof(version),1,f);
	uCRC = Get_CRC(&version,sizeof(version), uCRC);
	fwrite(&cnt,sizeof(cnt),1,f);
	uCRC = Get_CRC(&cnt,sizeof(cnt), uCRC);	
	fwrite(&m_nRecoverySize,sizeof(m_nRecoverySize),1,f);
	uCRC = Get_CRC(&m_nRecoverySize,sizeof(m_nRecoverySize), uCRC);
	fwrite(&m_nRecoveryBlockSize,sizeof(m_nRecoveryBlockSize),1,f);
	uCRC = Get_CRC(&m_nRecoveryBlockSize,sizeof(m_nRecoveryBlockSize), uCRC);

	int nSegSize = SEG_SIZE*1024*1024;
	if (nSegSize % m_nRecoveryBlockSize!=0)
	{
		nSegSize += m_nRecoveryBlockSize - (nSegSize % m_nRecoveryBlockSize);
	}
	int nSegments = 1;
	if (nSegSize > m_nRecoverySize)
		nSegSize = int(m_nRecoverySize);
	else
	{
		nSegments = int(m_nRecoverySize / nSegSize);
		if (m_nRecoverySize % nSegSize > 0) nSegments++;
	}

	int segment;

	if (m_szRecovery!=NULL)	delete [] m_szRecovery;
	m_szRecovery = new char [nSegSize];
	if (m_szRecovery==NULL)
		return E_WRONG_OUT_OF_MEMORY;
	char *buf = new char [m_nRecoveryBlockSize];
	if (buf==NULL)
	{
		delete [] m_szRecovery;
		m_szRecovery = NULL;
		return E_WRONG_OUT_OF_MEMORY;
	}
	m_blbCRC.SetSize(0);
	m_blbCRC.ResetIndex();

//	for (int i=0; i<m_arFiles.GetSize(); i++)
//	{
//		t_FileInfo fi = m_arFiles.GetAt(i);
///		AddFileToRecovery(fi);
///		m_arFiles.SetAt(i, fi);
//	}
	CFileSpace fs;
	fs.SetBlockSize(m_nRecoveryBlockSize);
	for (int i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		fs.AddFile(fi.szName, fi.size);
	}

#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt=0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Creating recovery info for");
		//g_DlgProgress->m_szFileName=szFileName;
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
#endif

	FILESIZE data_written=0;
	for (segment = 0; segment<nSegments; segment++)
	{
		memset(m_szRecovery,0, nSegSize*sizeof(char));
		int cur_seg_size = nSegSize;
		if (segment==nSegments-1)
		{
			if (m_nRecoverySize % nSegSize >0)
			{
				cur_seg_size = int(m_nRecoverySize % nSegSize);
			}
		}

		FILESIZE offset=((FILESIZE)segment)*((FILESIZE)nSegSize);
		FILESIZE offset2=offset+cur_seg_size;
		if (offset2>fs.GetAllSize()) offset2 = fs.GetAllSize();

		while (offset<fs.GetAllSize())
		{
			FILESIZE blockA = offset / m_nRecoveryBlockSize;
			fs.SetBlock(blockA, false);
			int to_read = int(offset2-offset);
			int rec_size = to_read;
			int block=0;

			while (to_read>0)
			{
				fs.ReadBlock(buf);
				if (block==0)
					int kk=1;
				_xoradd(m_szRecovery + block*m_nRecoveryBlockSize, buf, m_nRecoveryBlockSize);

				UINT crc = Get_CRC(buf, m_nRecoveryBlockSize);
				m_blbCRC << crc;

				if (to_read>=m_nRecoveryBlockSize)				
					to_read-=m_nRecoveryBlockSize;
				else
					return 500;
				block++;
#ifdef _INSIDE_MFC_APP
				if (pcnt % 1000 == 0 && g_DlgProgress!=NULL)
				{
					int percent = 0;
					if (fs.GetAllSize()>0)
					{
						percent = int(double(100*pcnt*m_nRecoveryBlockSize)/double(fs.GetAllSize()));
						if (percent>100 || percent<0)
							percent = 0;
					}
					CString s;
					fs.GetCurrentFile(s);
					t_FileInfo fi;
					_tcscpy(fi.szName, s);
					CutPath(&fi);
					if (nSegments>1)
						g_DlgProgress->m_szFileName.Format(_T("%s (segment %d/%d)"),fi.szName, segment+1, nSegments);
					else
						g_DlgProgress->m_szFileName.Format(_T("%s"),fi.szName);
					g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
					//g_DlgProgress->UpdateData(FALSE);
					g_DlgProgress->m_cProgress.SetPos(percent);
					//g_DlgProgress->Invalidate();
					//g_DlgProgress->UpdateWindow();

					//g_DlgProgress->RedrawWindow();
				}
				if (g_DlgProgress->m_bNeedPause)
				{
					while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
						Sleep(1000);
				}
				if (g_DlgProgress->m_bNeedTerminate)
				{
					if (m_szRecovery!=NULL)	
					{
						delete [] m_szRecovery;
						m_szRecovery = NULL;
					}
					delete [] buf ;
					fclose(f);
					_tunlink(szFileName);
					return E_PROTECTOR_TERMINATED;
				}
				pcnt++;
#endif
			}
			offset+=m_nRecoverySize;
			offset2=offset+cur_seg_size;
			if (offset2>fs.GetAllSize()) offset2 = fs.GetAllSize();			
		}

		if (data_written+nSegSize<=m_nRecoverySize)
		{
			fwrite(m_szRecovery, 1, nSegSize, f);
			uCRC = Get_CRC(m_szRecovery, nSegSize, uCRC);

			data_written+=nSegSize;
		}
		else
		{
			if (m_nRecoverySize>data_written)
			{
				FILESIZE to_write = m_nRecoverySize - data_written;
				if (to_write>0)
				{
					fwrite(m_szRecovery, 1, (size_t)(to_write), f);
					uCRC = Get_CRC(m_szRecovery, (size_t)(to_write), uCRC);
					data_written=m_nRecoverySize;
				}
			}
		}
	}
	if (m_szRecovery!=NULL)	
	{
		delete [] m_szRecovery;
		m_szRecovery = NULL;
	}
	delete [] buf ;

	//Writing Footer
	unsigned long siz2 = m_blbCRC.GetSize();
	fwrite(&siz2,sizeof(siz2),1,f);
	uCRC = Get_CRC(&siz2,sizeof(siz2), uCRC);
	int ll = _tcslen(m_szPath)+1;
	fwrite(&ll, sizeof(ll),1,f);
	uCRC = Get_CRC(&ll, sizeof(ll), uCRC);
	fwrite(m_szPath, sizeof(TCHAR),ll,f);
	uCRC = Get_CRC(m_szPath, sizeof(TCHAR)*ll, uCRC);

	bool bHasPath = true;
	if (_tcscmp(m_szPath,_T(""))==0)
		bHasPath = false;

	int i;
	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		if (bHasPath)
			CutBasePath(&fi);
		else
			CutPath(&fi);

		fwrite(&fi.size, sizeof(fi.size),1,f);
		uCRC = Get_CRC(&fi.size, sizeof(fi.size), uCRC);

		int len = _tcslen(fi.szName)+1;
		fwrite(&len, sizeof(len),1,f);
		uCRC = Get_CRC(&len, sizeof(len), uCRC);

		fwrite(fi.szName, sizeof(TCHAR),len,f);
		uCRC = Get_CRC(fi.szName, sizeof(TCHAR)*len, uCRC);

//		fwrite(&fi.st_ctime, sizeof(fi.st_ctime),1,f);
//		uCRC = Get_CRC(&fi.st_ctime, sizeof(fi.st_ctime), uCRC);

		fwrite(&fi.st_mtime, sizeof(fi.st_mtime),1,f);
		uCRC = Get_CRC(&fi.st_mtime, sizeof(fi.st_mtime), uCRC);

//		fwrite(&fi.nCrc, sizeof(fi.nCrc),1,f);
//		uCRC = Get_CRC(&fi.nCrc, sizeof(fi.nCrc), uCRC);
	}
	
	fwrite(m_blbCRC.GetBuffer(), 1, siz2, f);
	uCRC = Get_CRC(m_blbCRC.GetBuffer(), siz2, uCRC);

	fwrite(&uCRC, sizeof(uCRC), 1, f);
	fclose(f);

	return 0;
}

void CProtector::CutPath(t_FileInfo *fi)
{
	if (fi==NULL)
		return;

	int rt = 0;
	int i;
	for (i=_tcslen(fi->szName)-1; i>0; i--)
	{
		if (fi->szName[i]=='\\')
		{
			rt=i;
			break;
		}
	}
	if (rt>0)
	{
		for (i=rt+1; i<=(int)_tcslen(fi->szName); i++)
		{
			fi->szName[i-rt-1] = fi->szName[i];
		}
	}
	
}

void CProtector::CutBasePath(t_FileInfo *fi)
{
	if (fi==NULL)
		return;

	int rt = _tcslen(m_szPath);
	if (rt>0)
	{
		int i;
		for (i=rt; i<=(int)_tcslen(fi->szName); i++)
		{
			fi->szName[i-rt] = fi->szName[i];
		}
	}
	
}

int CProtector::LoadRecovery2(LPCTSTR szFileName)
{

	FILESIZE file_size;
	struct _stati64 statfile;
	if( _tstati64( szFileName, &statfile ) == 0 )
	{
		file_size = statfile.st_size;
		if( file_size == 0 )
			return E_EMPTY_RECOVERY_FILE;
	}

	FILE *f = _tfopen (szFileName, _T("rb"));
	if (f==NULL) return E_FILE_NOT_OPENS;

	CString szFN2 = szFileName;
	int pos = szFN2.ReverseFind('\\');
	CString szXPath=".\\";
	if (pos>0)
	{
		szXPath = szFN2.Left(pos+1);
		SetCurrentDirectory(szXPath);
	}

	char *buf1 = new char [1000];
	char *buf2 = new char [1000];
	int b2e = 0;

	UINT nCRC_a = 0;
	UINT nCRC_f = 0;
#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt=0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Checking integrity of the recovery file");
		g_DlgProgress->m_szFileName=szFileName;
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
#endif
	while (!feof(f))
	{
		int size = fread(buf1, sizeof(char), 1000, f);
		if (size<1000)
		{
			if (size<sizeof(UINT))
			{
				if (b2e==0)	return E_FORMAT_ERROR;
				nCRC_a = Get_CRC(buf2, 1000-(sizeof(UINT)-size), nCRC_a);
				memcpy(&nCRC_f, buf2+1000-(sizeof(UINT)-size), sizeof(UINT)-size);
				memcpy(((char *)&nCRC_f)+(sizeof(UINT)-size), buf1, size);
				break;
			}
			else
			{
				if (b2e)
					nCRC_a = Get_CRC(buf2, 1000, nCRC_a);
				nCRC_a = Get_CRC(buf1, size-sizeof(UINT), nCRC_a);
				memcpy(&nCRC_f, buf1+size-sizeof(UINT), sizeof(UINT));
				break;
			}
		}
		if (b2e)
			nCRC_a = Get_CRC(buf2, 1000, nCRC_a);
		memcpy(buf2,buf1, size);
		b2e=1;
#ifdef _INSIDE_MFC_APP
		if (pcnt % 1000 == 0 && g_DlgProgress!=NULL)
		{
			int percent = 0;
			if (file_size>0)
			{
				percent = int(double(100*pcnt*1000)/double(file_size));
				if (percent>100 || percent<0)
					percent = 0;
			}
			g_DlgProgress->m_cProgress.SetPos(percent);
			//g_DlgProgress->Invalidate();
			//g_DlgProgress->UpdateWindow();
			//g_DlgProgress->RedrawWindow();
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			if (buf1!=NULL)
				delete [] buf1;
			if (buf2!=NULL)
				delete [] buf2;
			fclose(f);
			Clear();
			return E_PROTECTOR_TERMINATED;
		}

		pcnt++;
#endif
	}

	delete [] buf2;
	delete [] buf1;

	fclose(f);

	if (nCRC_a!=nCRC_f)	return E_CRC_ERROR;

	//Clearing
	Clear();

	//Loading

	f = _tfopen (szFileName, _T("rb"));
	if (f==NULL) return E_FILE_NOT_OPENS;

	int version = FILE_VERSION;
	int cnt = 0;
	char sig[5]="$RCV";
	char sig2[5];
	unsigned long siz2 = 0;
	
	fread(sig2,1,4,f);	
	fread(&version,sizeof(version),1,f);
	if (version > FILE_VERSION) 
		return E_VERSION_OLD;
	fread(&cnt,sizeof(cnt),1,f);
	if (cnt<=0) return E_FORMAT_ERROR;
	fread(&m_nRecoverySize,sizeof(m_nRecoverySize),1,f);
	if (m_nRecoverySize<=0) return E_FORMAT_ERROR;
	fread(&m_nRecoveryBlockSize,sizeof(m_nRecoveryBlockSize),1,f);
	if (m_nRecoveryBlockSize<=0) return E_FORMAT_ERROR;

	FILESIZE xx = _ftelli64(f);

	if (_fseeki64(f, m_nRecoverySize, SEEK_CUR)!=0)
		return E_FILE_CANNOT_READ;
	
	fread(&siz2,sizeof(siz2),1,f);
	if (siz2<=0) return E_FORMAT_ERROR;

	int ll;
	fread(&ll, sizeof(ll),1,f);
	fread(m_szPath, sizeof(TCHAR),ll,f);

	m_nCntBlocks = int(m_nRecoverySize/m_nRecoveryBlockSize);

	//Create buffers
//	if (m_szRecovery!=NULL)
//		delete [] m_szRecovery;
//	m_szRecovery = new char [m_nRecoverySize];
//	memset(m_szRecovery,0, m_nRecoverySize*sizeof(char));

	m_blbCRC.SetSize(siz2+1);
	m_blbCRC.ResetIndex();

#ifdef _INSIDE_MFC_APP
	pcnt = 0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Loading the list of files");
		g_DlgProgress->m_szFileName=szFileName;
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
	}
#endif

	int i;
	for (i=0; i<cnt; i++)
	{
		t_FileInfo fi;
		int len;
		CString s2,s3;
		

		fread(&fi.size, sizeof(fi.size),1,f);
		fread(&len, sizeof(len),1,f);
		if (len<=0) return E_FORMAT_ERROR;

		fread(fi.szName, sizeof(TCHAR),len,f);
		fread(&fi.st_mtime, sizeof(fi.st_mtime),1,f);

		m_nTotalSize+=fi.size;
		fi.status=6;

		/*FILE *ft = _tfopen(fi.szName,_T("rb"));
		int k = errno;
		if (ft==NULL)
		{
			fi.status=3;
		}
		else
		{
			fclose(ft);
			FILESIZE file_size;
			struct _stati64 statfile;
			if( _tstati64( fi.szName, &statfile ) == 0 )
			{
				file_size = statfile.st_size;
				if( file_size!=fi.size)
					fi.status=5;
			}
			else
				fi.status=4;
		}*/
		m_arFiles.Add(fi);
#ifdef _INSIDE_MFC_APP
		if (pcnt % 100 == 0 && g_DlgProgress!=NULL)
		{
			int percent = 0;
			if (file_size>0)
			{
				percent = int(double(100*pcnt)/double(cnt));
				if (percent>100 || percent<0)
					percent = 0;
			}
			g_DlgProgress->m_cProgress.SetPos(percent);
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			fclose(f);
			Clear();
			return E_PROTECTOR_TERMINATED;
		}
		pcnt++;
#endif
	}
	
	fread(m_blbCRC.GetBuffer(), 1, siz2, f);
	fclose(f);

	m_szRecFileName = szFileName;
	m_bReadOnly = true;

	CheckFilesExists();

	return 0;
}

int CProtector::CheckFilesExists()
{
	m_bAllSrcFilesExist = true;
	m_nFilesExistCount = 0;
#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt = 0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Checking file sizes");
		g_DlgProgress->m_szFileName = _T("...");
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
	}
#endif

	CString szPath = m_szPath;
	if (szPath.GetLength()>1 && szPath.Right(1)!=_T("\\"))
		szPath += _T("\\");
	CString szFileName;

	int i;
	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi;
		CString s2,s3;		

		fi = m_arFiles[i];
		fi.status=6;

		szFileName = szPath + fi.szName;

		if (_taccess(szFileName, 04))
		{
			fi.status=3;
			m_bAllSrcFilesExist = false;
		}
		else
		{
			m_nFilesExistCount++;
			FILESIZE file_size;
			struct _stati64 statfile;
			if( _tstati64( szFileName, &statfile ) == 0 )
			{
				file_size = statfile.st_size;
				if( file_size!=fi.size)
					fi.status=5;
			}
			else
				fi.status=4;
		}
		m_arFiles.SetAt(i, fi);
#ifdef _INSIDE_MFC_APP
		if (pcnt % 100 == 0 && g_DlgProgress!=NULL)
		{
			int percent = 0;
			percent = int(double(100*pcnt)/double(m_arFiles.GetSize()));
			if (percent>100 || percent<0)
				percent = 0;
			g_DlgProgress->m_cProgress.SetPos(percent);
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			Clear();
			return E_PROTECTOR_TERMINATED;
		}
		pcnt++;
#endif
	}
	return 0;
}

/*
int CProtector::Check()
{
	if (m_npattern!=NULL)
		delete [] m_npattern;

	if (m_szRestSum!=NULL)
		delete [] m_szRestSum;
	m_szRestSum = new char [m_nRecoverySize];
	if (m_szRestSum==NULL)
		return -2;
	memset(m_szRestSum, 0, m_nRecoverySize);

	m_nCurrentBlock = 0;
	m_nCurrentBlockA = 0;
	m_blbCRC.ResetIndex();

	m_npattern = new t_pattern[m_nCntBlocks];
	if (m_npattern==NULL)
		return -2;
	memset(m_npattern, 0, sizeof(t_pattern)*m_nCntBlocks);

	int i;
	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		CheckFile(fi);
		m_arFiles.SetAt(i,fi);
	}

	bool bOk=true;
	m_nCntRecoverable=0;
	m_nCntNotRecoverable=0;

	for (i=0; i<m_nCntBlocks; i++)
	{
		if (m_npattern[i].size!=0)
		{
			bOk = false;
			if (m_npattern[i].size>0)
				m_nCntRecoverable++;
			else
				m_nCntNotRecoverable++;
		}
	}

	if (bOk)
		return 0;
	else
		return -1;
}
*/
int CProtector::GetTmpFileName(CString &szFileName)
{
	szFileName.Format(_T("%s.tmp"), m_szRecFileName);
	FILE * f;
	if ((f=_tfopen(szFileName,_T("w")) )==NULL) 
	{
		CString szTmpPath;
		GetTempPath(MAX_PATH_PROT, szTmpPath.GetBuffer(MAX_PATH_PROT+1));
		szTmpPath.ReleaseBuffer();
		if (szTmpPath==_T(""))
			return -1;
		else
		{
			if (szTmpPath.Right(1)!='\\') szTmpPath+="\\";
			szFileName.Format(_T("%s%s.tmp"), szTmpPath, m_szRecFileName);
			if ((f=_tfopen(szFileName,_T("w")) )==NULL) 
				return -1;
			else
				fclose(f);
		}
	}
	else
	{
		fclose(f);
		_tunlink(szFileName);
	}

	return 0;
}

int CProtector::Check2()
{
	if (m_arFiles.GetSize()==0)
		return E_NO_FILES;

	if (m_szRecFileName=="")
		return E_WRONG_OPERATION;

	if (m_npattern!=NULL)
	{
		delete [] m_npattern;
		m_npattern = NULL;
	}
	if (m_szRestSum!=NULL)
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
	}

	int nSegSize = SEG_SIZE*1024*1024;
	if (nSegSize % m_nRecoveryBlockSize!=0)
	{
		nSegSize += m_nRecoveryBlockSize - (nSegSize % m_nRecoveryBlockSize);
	}
	int nSegments = 1;
	if (nSegSize > m_nRecoverySize)
		nSegSize = int(m_nRecoverySize);
	else
	{
		nSegments = int(m_nRecoverySize / nSegSize);
		if (m_nRecoverySize % nSegSize > 0) nSegments++;
	}
	int nBlocksPerSeg = nSegSize / m_nRecoveryBlockSize;

	if (m_szTempFile==_T(""))
	{
		int rc = GetTmpFileName(m_szTempFile);
		if (rc!=0)	return E_TMP_FILENAME; 
	}
	CString szPath = m_szPath;
	if (szPath.GetLength()>1 && szPath.Right(1)!=_T("\\"))
		szPath+=_T("\\");
	
	m_szRestSum = new char [nSegSize];
	if (m_szRestSum==NULL)
		return E_WRONG_OUT_OF_MEMORY;

	m_blbCRC.ResetIndex();

	m_npattern = new t_pattern[m_nCntBlocks];

	if (m_npattern==NULL)
		return E_WRONG_OUT_OF_MEMORY;
	memset(m_npattern, 0, sizeof(t_pattern)*m_nCntBlocks);

	char *buf = new char [m_nRecoveryBlockSize];
	if (buf==NULL)
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
		return E_WRONG_OUT_OF_MEMORY;
	}

	CFileSpace fs;
	fs.SetBlockSize(m_nRecoveryBlockSize);
	bool bNotAll = false;
#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt=0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Prepearing to check");
		g_DlgProgress->m_szFileName=_T("...");
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
#endif
	CString szFileName;
	for (int i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		szFileName = szPath + fi.szName;

		int nOK = _taccess (szFileName, 04);
		if (nOK != 0) 
		{
			fi.status=3;
		}
		else
		{
			FILESIZE file_size;
			struct _stati64 statfile;
			if( _tstati64( szFileName, &statfile ) == 0 )
				file_size = statfile.st_size;
			else
				return 100;

			if (file_size!=fi.size)
				fi.status=5;
//			else
//				fi.status=6;
		}

		fi.errors=0;
		m_arFiles.SetAt(i, fi);

		if (fi.checked==1)
			fs.AddFile(szFileName, fi.size);
		else
		{
			fs.AddFile(szFileName, fi.size, 1);
			bNotAll = true;
		}

#ifdef _INSIDE_MFC_APP
		if (pcnt % 100 == 0 && g_DlgProgress!=NULL)
		{
			int percent = 0;
			if (m_arFiles.GetSize()>0)
			{
				percent = int(double(100*pcnt)/double(m_arFiles.GetSize()));
				if (percent>100 || percent<0)
					percent = 0;
			}
			//g_DlgProgress->UpdateData(FALSE);
			g_DlgProgress->m_cProgress.SetPos(percent);
			//g_DlgProgress->RedrawWindow();
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			if (fRest!=NULL)
			{
				fclose(fRest);
				fRest = NULL;
				_tunlink(m_szTempFile);
			}
			if (buf!=NULL)
				delete [] buf;
			
			if (m_szRestSum!=NULL)
				delete [] m_szRestSum;
			m_szRestSum = NULL;
			return E_PROTECTOR_TERMINATED;
		}
		pcnt++;
#endif
	}

	if (!bNotAll)
	{
		if (fRest!=NULL)
		{
			fclose(fRest);
			fRest = NULL;
		}

		if ((fRest=_tfopen(m_szTempFile,_T("w")) )==NULL) return E_FILE_CANNOT_WRITE;
		fclose(fRest);
		fRest=NULL;
		if ((fRest=_tfopen(m_szTempFile,_T("rb+")) )==NULL) return E_FILE_CANNOT_WRITE;
	}

	//FILESIZE data_written=0;
#ifdef _INSIDE_MFC_APP
	pcnt=0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Checking integrity of");
		g_DlgProgress->m_szFileName=_T("");
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
#endif
	int segment;
	for (segment = 0; segment<nSegments; segment++)
	{
		memset(m_szRestSum,0, nSegSize*sizeof(char));
		int cur_seg_size = nSegSize;
		if (segment==nSegments-1)
		{
			if (m_nRecoverySize % nSegSize >0)
			{
				cur_seg_size = int(m_nRecoverySize % nSegSize);
			}
		}

		FILESIZE offset=((FILESIZE)segment)*((FILESIZE)nSegSize);
		FILESIZE offset2=offset+cur_seg_size;
		if (offset2>fs.GetAllSize()) offset2 = fs.GetAllSize();

		while (offset<fs.GetAllSize())
		{
			FILESIZE blockA = offset / m_nRecoveryBlockSize;
			fs.SetBlock(blockA, false);
			int to_read = int(offset2-offset);
			int rec_size = to_read;
			int block=0;

			while (to_read>0)
			{
				fs.ReadBlock(buf);
				UINT crc = 0;
				if (!fs.isIgnored()) crc = Get_CRC(buf, m_nRecoveryBlockSize);
				UINT crc2;
				m_blbCRC >> crc2;

				if (!fs.isIgnored())
				{
					if (crc==crc2)
					{
						_xoradd(m_szRestSum + block*m_nRecoveryBlockSize, buf, m_nRecoveryBlockSize);
					}
					else
					{
						CString szFN;
						int f = fs.GetCurrentFile(szFN);
						int sc = nBlocksPerSeg*segment+block;
						if (m_npattern[sc].size==0)
						{
							m_npattern[sc].block = blockA + block;
							m_npattern[sc].size = m_nRecoveryBlockSize;
							m_npattern[sc].file = f;
							t_FileInfo fi = m_arFiles.GetAt(f);
							fi.errors++;

							if (fi.status!=2)
								fi.status=1;

							if (bNotAll)
								fi.status=7;

							m_arFiles.SetAt(f, fi);
						}
						else
						{
							t_FileInfo fi = m_arFiles.GetAt(f);
							fi.status=2;
							fi.errors++;
							m_arFiles.SetAt(f, fi);

							if (m_npattern[sc].size>0)
							{
								fi = m_arFiles.GetAt(m_npattern[sc].file);
								fi.status=2;
								m_arFiles.SetAt(m_npattern[sc].file, fi);
								m_npattern[sc].file=0;
							}

							if (bNotAll)
								fi.status=7;

							m_npattern[sc].file++;											 
							int sc = nBlocksPerSeg*segment+block;
							m_npattern[sc].size=-1;
						}
					}
				}

				if (to_read>=m_nRecoveryBlockSize)				
					to_read-=m_nRecoveryBlockSize;
				else
					return 500;
				block++;

#ifdef _INSIDE_MFC_APP
				if (pcnt % 1000 == 0 && g_DlgProgress!=NULL)
				{
					int percent = 0;
					if (fs.GetAllSize()>0)
					{
						percent = int(double(100*pcnt*m_nRecoveryBlockSize)/double(fs.GetAllSize()));
						if (percent>100 || percent<0)
							percent = 0;
					}
					CString s;
					fs.GetCurrentFile(s);
					t_FileInfo fi;
					_tcscpy(fi.szName, s);
					CutPath(&fi);
					//g_DlgProgress->m_szFileName=fi.szName;
					if (nSegments>1)
						g_DlgProgress->m_szFileName.Format(_T("%s (segment %d/%d)"),fi.szName, segment+1, nSegments);
					else
						g_DlgProgress->m_szFileName.Format(_T("%s"),fi.szName);
					g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
					//g_DlgProgress->UpdateData(FALSE);
					g_DlgProgress->m_cProgress.SetPos(percent);
					//g_DlgProgress->Invalidate();
					//g_DlgProgress->UpdateWindow();
				}
				if (g_DlgProgress->m_bNeedPause)
				{
					while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
						Sleep(1000);
				}
				if (g_DlgProgress->m_bNeedTerminate)
				{
					if (fRest!=NULL)
					{
						fclose(fRest);
						fRest = NULL;
						_tunlink(m_szTempFile);
					}
					if (buf!=NULL)
						delete [] buf;
					
					if (m_szRestSum!=NULL)
						delete [] m_szRestSum;
					m_szRestSum = NULL;
					return E_PROTECTOR_TERMINATED;
				}
				pcnt++;
#endif

			}
			offset+=m_nRecoverySize;
			offset2=offset+cur_seg_size;
			if (offset2>fs.GetAllSize()) offset2 = fs.GetAllSize();			
		}

		if (!bNotAll)
		{
			size_t nActualSize = fwrite(m_szRestSum, 1, cur_seg_size, fRest);
			if (nActualSize!=cur_seg_size)
			{
				if (fRest!=NULL)
				{
					fclose(fRest);
					fRest = NULL;
					_tunlink(m_szTempFile);
				}
				if (buf!=NULL)
					delete [] buf;

				if (m_szRestSum!=NULL)
					delete [] m_szRestSum;
				m_szRestSum = NULL;
				return E_NOT_ENOUGH_SPACE;
			}
		}
		
		/*if (data_written+nSegSize<=m_nRecoverySize)
		{
			fwrite(m_szRestSum, 1, nSegSize, fRest);
			data_written+=nSegSize;
		}
		else
		{
			if (m_nRecoverySize>data_written)
			{
				FILESIZE to_write = m_nRecoverySize - data_written;
				if (to_write>0)
				{
					fwrite(m_szRestSum, 1, to_write, fRest);
					data_written=m_nRecoverySize;
				}
			}
		}*/
	}
	if (m_szRestSum!=NULL)	
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
	}
	delete [] buf ;
	bool bOk=true;
	m_nCntRecoverable=0;
	m_nCntNotRecoverable=0;

	int i;
	for (i=0; i<m_nCntBlocks; i++)
	{
		if (m_npattern[i].size!=0)
		{
			bOk = false;
			if (m_npattern[i].size>0)
				m_nCntRecoverable++;
			else
			{
				m_nCntNotRecoverable+=m_npattern[i].file;
			}
		}
	}

	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		if (fi.checked==1)
		{
			fi.checked=2;
			if (fi.status==6)
				fi.status=0;
		}
		m_arFiles.SetAt(i, fi);
	}

	if (m_nCntRecoverable==0 || bNotAll)
	{
		if (fRest!=NULL)
		{
			fclose(fRest);
			fRest=NULL;
			_tunlink(m_szTempFile);
		}
	}

	if (bOk)
		return 0;
	else
		return -1;
}


/*

int CProtector::Recover()
{
	int file;

	if (m_nCntRecoverable==0)
		return -1;

	m_nCurrentBlock=0;
	m_nCurrentBlockA=0;
	
	char *buf = new char [m_nRecoveryBlockSize];
	char *buf_rec = new char [m_nRecoveryBlockSize];

	for (file=0; file<m_arFiles.GetSize(); file++)
	{
		t_FileInfo fi = m_arFiles.GetAt(file);
		FILESIZE to_read = fi.size;

		FILESIZE file_size;
		struct _stati64 statfile;
		if( _stati64( fi.szName, &statfile ) == 0 )
			file_size = statfile.st_size;
		else
			return 100;
		
		FILE *f = fopen (fi.szName, "rb+");
		if (f==NULL) return -2;

		int nStatus = 0;

		while (to_read>0)
		{
			if (m_npattern[m_nCurrentBlock].size<0)
				nStatus=2;

			if (m_npattern[m_nCurrentBlock].size>0 && m_npattern[m_nCurrentBlock].block == m_nCurrentBlockA)
			{
				//This block needs recover
				memset(buf_rec,0,m_nRecoveryBlockSize);
				_xoradd(buf_rec, m_szRecovery+m_nCurrentBlock*m_nRecoveryBlockSize, m_nRecoveryBlockSize);
				_xoradd(buf_rec, m_szRestSum+m_nCurrentBlock*m_nRecoveryBlockSize, m_nRecoveryBlockSize);
				
				fwrite(buf_rec, sizeof(char), m_npattern[m_nCurrentBlock].size, f);	
				if (to_read>=m_npattern[m_nCurrentBlock].size)
					to_read-=m_npattern[m_nCurrentBlock].size;
				else
					to_read=0;
			}
			else
				if (!feof(f))
				{
					int size = fread(buf, sizeof(char), m_nRecoveryBlockSize, f);
					if (size==0)
						continue;
					if (to_read>=m_nRecoveryBlockSize)
						to_read-=m_nRecoveryBlockSize;
					else
						to_read=0;
				}
				else
				{
					if (to_read>=m_nRecoveryBlockSize)
						to_read-=m_nRecoveryBlockSize;
					else
						to_read=0;
				}

			m_nCurrentBlock++;
			m_nCurrentBlockA++;
			if (m_nCurrentBlock>=m_nCntBlocks)
				m_nCurrentBlock=0;

		}
		fclose(f);
		if (file_size>fi.size)
		{
			int fh, result;
			if( (fh = _open( fi.szName, _O_RDWR, _S_IREAD | _S_IWRITE ))  != -1 )
			{
				result = _chsize_s( fh, fi.size );
				_close( fh );
			}
			else
				nStatus=5;				
		}
		fi.status = nStatus;
		m_arFiles.SetAt(file, fi);
	}

	delete [] buf_rec;
	delete [] buf;

	return 0;
}*/

int CProtector::Recover2(int spec_file /*= -1*/)
{
	if (fRest==NULL)
		return CheckAndRecoverSmall();

	if (m_nCntRecoverable==0)
		return E_UNRECOVERABLE;

	FILE *fRec = _tfopen (m_szRecFileName, _T("rb"));
	if (fRec==NULL) return E_FILE_NOT_OPENS;

	char *buf_rec = new char [m_nRecoveryBlockSize];
	char *buf_sum = new char [m_nRecoveryBlockSize];
	char *buf_file = new char [m_nRecoveryBlockSize];

	int i; 
	CFileSpace fs;
	fs.SetBlockSize(m_nRecoveryBlockSize);
#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt=0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Prepearing to recover");
		g_DlgProgress->m_szFileName=_T("...");
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
	bool bUpdateNow=false;
#endif
	bool bUsePath=false;
	

	CString szPath = m_szRecoverPath;
	if (szPath!=_T(""))
	{
		if (szPath.Right(1)!='\\')
			szPath+="\\";
		bUsePath=true;
	}

	CString szSrcPath = m_szPath;
	if (szSrcPath.GetLength()>1 && szSrcPath.Right(1)!=_T("\\"))
		szSrcPath+=_T("\\");

	CString szSrcFileName;
	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);

		szSrcFileName = szSrcPath + fi.szName;

		if (bUsePath && fi.to_recover==1)
		{
			CString szTrgFileName = szPath + fi.szName;
			//Checking File
			FILESIZE file_size=0;
			struct _stati64 statfile;
			if( _tstati64( szSrcFileName, &statfile ) == 0 )
			{
				file_size = statfile.st_size;
			}
						
			if (file_size>0)
			{
				fs.EnsurePathExist(szTrgFileName);
				//Copying file...
				int rc = CopyFile(szSrcFileName, szTrgFileName, FALSE);
				if (rc==0)
				{
					if (buf_rec!=NULL) delete [] buf_rec;
					if (buf_sum!=NULL) delete [] buf_sum;
					if (buf_file!=NULL) delete [] buf_file;
					return E_COPY_FAILED;
				}
				bUpdateNow=true;
			}

			fs.AddFile(szTrgFileName, fi.size);
		}
		else
			fs.AddFile(szSrcFileName, fi.size);

#ifdef _INSIDE_MFC_APP
		if ((pcnt % 100 == 0 || bUpdateNow) && g_DlgProgress!=NULL)
		{
			bUpdateNow = false;
			int percent = 0;
			if (m_arFiles.GetSize()>0)
			{
				percent = int(double(100*pcnt)/double(m_arFiles.GetSize()));
				if (percent>100 || percent<0)
					percent = 0;
			}
			//g_DlgProgress->UpdateData(FALSE);
			g_DlgProgress->m_cProgress.SetPos(percent);
			//g_DlgProgress->RedrawWindow();
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			if (buf_rec!=NULL)
				delete [] buf_rec;
			if (buf_sum!=NULL)
				delete [] buf_sum;
			if (buf_file!=NULL)
				delete [] buf_file;
			return E_PROTECTOR_TERMINATED;
		}
		pcnt++;
#endif
	}

#ifdef _INSIDE_MFC_APP
	pcnt=0;
	CString szLastFile=_T("...");
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Recovering");
		g_DlgProgress->m_szFileName=_T("...");
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
	}
#endif
	for (i=0; i<m_nCntBlocks; i++)
	{
		if (m_npattern[i].size>0)
		{
			FILESIZE blockA = m_npattern[i].block;

			int nFile=0;
			FILESIZE nPos=0;
			int fok = fs.GetFileNumberAndPos(blockA*m_nRecoveryBlockSize, nFile, nPos);
			if (fok!=0)
				continue;

			t_FileInfo fi = m_arFiles.GetAt(nFile);
			if (fi.to_recover==0 || fi.checked!=2)
				continue;

			//Get recover data
			FILESIZE ipos = (FILESIZE) HEADER_SIZE + 
				((FILESIZE)i)*((FILESIZE)m_nRecoveryBlockSize);
			int rc = _fseeki64 (fRec, ipos, SEEK_SET);
			if (rc!=0)
				continue;
			int size = fread(buf_rec, 1, m_nRecoveryBlockSize, fRec);
			if (size<m_nRecoveryBlockSize)
				continue;

			//Get sum data
			ipos = ((FILESIZE)i)*((FILESIZE)m_nRecoveryBlockSize);
			rc = _fseeki64 (fRest, ipos, SEEK_SET);
			if (rc!=0)
				continue;
			size = fread(buf_sum, 1, m_nRecoveryBlockSize, fRest);
			if (size<m_nRecoveryBlockSize)
				continue;

			//Get file data
			memset(buf_file,0,m_nRecoveryBlockSize);

			//XOR!
			_xoradd(buf_file, buf_rec, m_nRecoveryBlockSize);
			_xoradd(buf_file, buf_sum, m_nRecoveryBlockSize);

			//Set file data
			rc = fs.SetBlock(m_npattern[i].block, true);
			if (rc==0)
			{
				rc = fs.WriteBlock(buf_file);
				if (rc==0)
				{
					m_npattern[i].size=0; 
					m_npattern[i].block=0;
					fi.to_recover=2;
					fi.errors--;
					if (fi.errors == 0)
						fi.status = 0;
					m_arFiles.SetAt(nFile, fi);
					CString s;
					int nFile2=fs.GetCurrentFile(s);
					if (nFile2!=nFile)
					{
						t_FileInfo fi = m_arFiles.GetAt(nFile2);
						fi.to_recover=2;
						m_arFiles.SetAt(nFile2, fi);
					}
				}				
			}
		}
#ifdef _INSIDE_MFC_APP
		if (pcnt % 100 == 0 && g_DlgProgress!=NULL)
		{
			int percent = 0;
			if (fs.GetAllSize()>0)
			{
				percent = int(double(100*pcnt)/double(m_nCntBlocks));
				if (percent>100 || percent<0)
					percent = 0;
			}
			g_DlgProgress->m_szFileName=szLastFile;
			g_DlgProgress->m_cProgress.SetPos(percent);
			g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		}
		if (g_DlgProgress->m_bNeedPause)
		{
			while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
				Sleep(1000);
		}
		if (g_DlgProgress->m_bNeedTerminate)
		{
			if (buf_rec!=NULL)
				delete [] buf_rec;
			if (buf_sum!=NULL)
				delete [] buf_sum;
			if (buf_file!=NULL)
				delete [] buf_file;
			return E_PROTECTOR_TERMINATED;
		}
		pcnt++;
#endif
	}

	delete [] buf_file;
	delete [] buf_sum;
	delete [] buf_rec;

	bool bOk=true;
	m_nCntRecoverable=0;
	m_nCntNotRecoverable=0;

	for (i=0; i<m_nCntBlocks; i++)
	{
		if (m_npattern[i].size!=0)
		{
			bOk = false;
			if (m_npattern[i].size>0)
				m_nCntRecoverable++;
			else
				m_nCntNotRecoverable++;
		}
	}
	fs.Close();

	for (i=0; i<m_arFiles.GetSize(); i++)
	{
		t_FileInfo fi = m_arFiles.GetAt(i);
		if (fi.to_recover==2)
		{
			if (bUsePath)
			{
				CString szFileName;
				fs.GetFileName(i, szFileName);
				__utimbuf64 times;
				times.actime = 0;
				times.modtime = fi.st_mtime;
				_tutime64(szFileName,&times);
			}
			else
			{
				__utimbuf64 times;
				times.actime = 0;
				times.modtime = fi.st_mtime;
				_tutime64(fi.szName,&times);
			}
		}
	}

	if (m_nCntRecoverable==0)
	{
		if (fRest!=NULL)
		{
			fclose(fRest);
			fRest=NULL;
			_tunlink(m_szTempFile);
		}
	}

	if (bOk)
		return 0;

	return 0;
}

int CProtector::CheckAndRecoverSmall()
{
/*	if (m_arFiles.GetSize()==0)
		return E_NO_FILES;

	if (m_szRecFileName=="")
		return E_WRONG_OPERATION;

	if (m_npattern!=NULL)
	{
		delete [] m_npattern;
		m_npattern = NULL;
	}
	if (m_szRestSum!=NULL)
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
	}

	int nSegSize = SEG_SIZE*1024*1024;
	if (nSegSize % m_nRecoveryBlockSize!=0)
	{
		nSegSize += m_nRecoveryBlockSize - (nSegSize % m_nRecoveryBlockSize);
	}
	int nSegments = 1;
	if (nSegSize > m_nRecoverySize)
		nSegSize = int(m_nRecoverySize);
	else
	{
		nSegments = int(m_nRecoverySize / nSegSize);
		if (m_nRecoverySize % nSegSize > 0) nSegments++;
	}
	int nBlocksPerSeg = nSegSize / m_nRecoveryBlockSize;

	CString m_szTempFile;
	int rc = GetTmpFileName(m_szTempFile);
	if (rc!=0)	return E_TMP_FILENAME; 
	
	m_szRestSum = new char [nSegSize];
	if (m_szRestSum==NULL)
		return E_WRONG_OUT_OF_MEMORY;

	m_blbCRC.ResetIndex();

	m_npattern = new t_pattern[m_nCntBlocks];

	if (m_npattern==NULL)
		return E_WRONG_OUT_OF_MEMORY;
	memset(m_npattern, 0, sizeof(t_pattern)*m_nCntBlocks);

	char *buf = new char [m_nRecoveryBlockSize];
	if (buf==NULL)
	{
		delete [] m_szRestSum;
		m_szRestSum = NULL;
		return E_WRONG_OUT_OF_MEMORY;
	}*/

	return 0;
}

int CProtector::AddFileToRecovery(t_FileInfo fi)
{
	if (fi.size==0)
		return E_ZERO_SIZED_FILE;

	FILE *f = _tfopen (fi.szName, _T("rb"));
	if (f==NULL) return E_FILE_NOT_OPENS;

	char *buf = new char [m_nRecoveryBlockSize];

	UINT FileCRC=0;

	while (!feof(f))
	{
		int size = fread(buf, sizeof(char), m_nRecoveryBlockSize, f);
		if (size==0)
			break;
		if (size<m_nRecoveryBlockSize)
			memset(buf+size, 0, m_nRecoveryBlockSize-size);

		if (size>0)
			FileCRC = Get_CRC(buf, size, FileCRC);
		
		_xoradd(m_szRecovery + m_nCurrentBlock*m_nRecoveryBlockSize, buf, m_nRecoveryBlockSize);
		m_nCurrentBlock++;
		m_nCurrentBlockA++;
		if (m_nCurrentBlock>=m_nCntBlocks)
			m_nCurrentBlock=0;

		UINT crc = Get_CRC(buf, m_nRecoveryBlockSize);
//		if (crc==0)
//			int k = 1;
		m_blbCRC << crc;
	}
	
	fi.nCrc=FileCRC;

	delete [] buf;

	fclose(f);
	return 0;
}

int CProtector::CheckFile(t_FileInfo &fi)
{
	if (fi.size==0)
		return E_ZERO_SIZED_FILE;

	FILESIZE to_read = fi.size;

	FILE *f = _tfopen (fi.szName, _T("rb"));
	if (f==NULL) 
	{
		fi.status=3;
		f = _tfopen (fi.szName, _T("w"));
		if (f==NULL) 
			return E_FILE_CANNOT_WRITE;
		fclose(f);
		f = _tfopen (fi.szName, _T("rb"));
		if (f==NULL) 
			return E_FILE_NOT_OPENS;
	}
	FILESIZE file_size;
	struct _stati64 statfile;
	if( _tstati64( fi.szName, &statfile ) == 0 )
		file_size = statfile.st_size;
	else
		return 100;


	if (file_size!=fi.size)
		fi.status=5;
	else
		fi.status=0;


	char *buf = new char [m_nRecoveryBlockSize];

	UINT FileCRC=0;

	while (to_read>0)
	{
		int size = 0;
		if (feof(f))
		{
			if (to_read>=m_nRecoveryBlockSize)
			{
				to_read-=m_nRecoveryBlockSize;
				size = m_nRecoveryBlockSize;
				memset(buf, 0, size);
			}
			else
			{
				size = int(to_read);
				to_read = 0;
				memset(buf, 0, size);
			}
		}
		else
		{
			size = fread(buf, sizeof(char), m_nRecoveryBlockSize, f);
			if (size<m_nRecoveryBlockSize)
			{
				if (to_read>size)
				{
					if (to_read>m_nRecoveryBlockSize)
					{
						size=m_nRecoveryBlockSize;
						to_read-=m_nRecoveryBlockSize;
					}
					else
					{
						size=int(to_read);
						to_read=0;
					}
				}
				else
					to_read=0;
			}
			else
			{
				if (to_read>size)
					to_read-=size;
				else
					to_read=0;
			}
			
		}
		if (size==0)
			break;

		if (size<m_nRecoveryBlockSize)
			memset(buf+size, 0, m_nRecoveryBlockSize-size);

		if (size>0)
			FileCRC = Get_CRC(buf, size, FileCRC);
		UINT crcF = Get_CRC(buf, m_nRecoveryBlockSize);
		
		//_xoradd(m_szRecovery + m_nCurrentBlock*m_nRecoveryBlockSize, buf, m_nRecoveryBlockSize);

		UINT crc;
		m_blbCRC >> crc;
		if (crc!=crcF)
		{
			if (m_npattern[m_nCurrentBlock].size==0)
			{
				m_npattern[m_nCurrentBlock].block = m_nCurrentBlockA;
				m_npattern[m_nCurrentBlock].size = size;
				if (fi.status!=2)
					fi.status=1;
			}
			else
			{
				fi.status=2;
				m_npattern[m_nCurrentBlock].size=-1;
			}
		}
		else
		{
			_xoradd(m_szRestSum + m_nCurrentBlock*m_nRecoveryBlockSize, buf, m_nRecoveryBlockSize);
		}
		m_nCurrentBlock++;
		m_nCurrentBlockA++;
		if (m_nCurrentBlock>=m_nCntBlocks)
			m_nCurrentBlock=0;
	}
	
	//fi.nCrc=FileCRC;

	delete [] buf;

	fclose(f);
	return 0;
}

int CProtector::AddDir(LPCTSTR szPath)
{
	CString szFolder = szPath;

	if (szFolder=="") return E_NO_FILES;
	if (szFolder.Right(1)!='\\')
		szFolder+="\\";

	// Can add additional files and dirs
	//m_arFiles.RemoveAll();
	//m_nTotalSize = 0;

	CStringArray szDirList;
	szDirList.Add(szFolder);
	if (m_arFiles.GetSize()==0 && _tcscmp(m_szPath,_T(""))==0)
		_tcscpy(m_szPath, szFolder);

#ifdef _INSIDE_MFC_APP
	FILESIZE pcnt = 0;
	if (g_DlgProgress!=NULL)
	{
		g_DlgProgress->m_szOperation=_T("Loading list of files");
		g_DlgProgress->m_szFileName=szPath;
		g_DlgProgress->m_cProgress.SetRange(0,100);
		g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
		//g_DlgProgress->UpdateData(FALSE);
		//g_DlgProgress->RedrawWindow();
	}
#endif

	//Recursive directory
	while (szDirList.GetSize()>0)
	{
		int cd = szDirList.GetSize()-1;
		CString szCurrentFolder = szDirList.GetAt(cd);
		szDirList.RemoveAt(cd);

		if (szCurrentFolder.Right(1)!='\\')
			szCurrentFolder+=_T("\\");

		CString szMask;
		szMask.Format(_T("%s*.*"), szCurrentFolder);

		WIN32_FIND_DATA fd;
		HANDLE  hFind = FindFirstFile(szMask, &fd);
		if( hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				szMask = fd.cFileName;				
				if (szMask!="." && szMask!=".." && szMask!="Directory.rcv")
				{
					bool isRecovery = false;
					if (szMask.GetLength()>4)
					{
						if (szMask.Right(4)==".rcv")
							isRecovery=true;
					}
					if (!isRecovery)
					{
						szMask = szCurrentFolder + fd.cFileName;
						
						if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
						{
							szDirList.Add(szMask);
						}
						else
						{						
							AddFileInfo(szMask);
						}
					}
				}
			}	while(FindNextFile(hFind, &fd));
			FindClose(hFind);
#ifdef _INSIDE_MFC_APP
			if (g_DlgProgress!=NULL)
			{
				int percent = 0;
				FILESIZE cnt = szDirList.GetSize()+pcnt;
				if (cnt>0)
				{
					percent = int(double(100*pcnt)/double(cnt+1));
					if (percent>100 || percent<0)
						percent = 0;
				}
				g_DlgProgress->m_szFileName=szCurrentFolder;
				g_DlgProgress->m_cProgress.SetPos(percent);
				g_DlgProgress->PostMessageW(WM_COMMAND, IDC_CUSTOM_UPDATE);
			}
			if (g_DlgProgress->m_bNeedPause)
			{
				while (g_DlgProgress->m_bNeedPause && !g_DlgProgress->m_bNeedTerminate)
					Sleep(1000);
			}
			if (g_DlgProgress->m_bNeedTerminate)
			{
				Clear();
				return E_PROTECTOR_TERMINATED;
			}
			pcnt++;
#endif
		}
	}

	return 0;
}

bool CProtector::isAllChecked()
{
	if (fRest==NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CProtector::isOrigPathCorrect()
{
	if (m_arFiles.GetSize()==0)
		return false;

	// If only several files protected, at least one should exist for original path to be correct
	if (m_arFiles.GetSize()<10 && m_nFilesExistCount>0)
		return true;

	// If many files protected, some must exist (>20%) for original path to be correct
	if (m_arFiles.GetSize()/5 < m_nFilesExistCount)
		return true;
	return false;
	
	/*
	int cnt = m_arFiles.GetSize()<=10 ? m_arFiles.GetSize() : 10;
	int i;
	CString szPathName;
	for (i=0; i<cnt; i++)
	{
		szPathName.Format(_T("%s%s"), m_szPath, m_arFiles[i].szName);		
		if (_taccess(szPathName, 0)!=0)
			return false;
	}*/
	return true;
}