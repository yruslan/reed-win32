#include "StdAfx.h"
#include "FileSpace.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>


CFileSpace::CFileSpace()
{
	file = NULL;
	m_nFilesCount = 0;
	m_nTotalSize = 0;
	m_nCurrentFile = -1;
	m_nCurrentPos = 0;
	m_nSectorSize = 32768;
	m_bReadOnly = true;
	m_nCurrentFilePos = 0;
}

CFileSpace::~CFileSpace()
{
	if (file!=NULL) 
	{
		fclose(file);
		file = NULL;
	}
}

bool CFileSpace::isIgnored()
{
	return m_bCurIgnored;
}

int CFileSpace::AddFile(LPCTSTR szFileName, FILESIZE use_size /*= 0*/, int ignore /*= 0*/)
{
	FILESIZE file_size, file_size2;
	struct _stati64 statfile;
	if( _tstati64( szFileName, &statfile ) == 0 )
	{
		file_size2 = statfile.st_size;
		file_size = file_size2;
//		if( file_size == 0 )
//			return E_FILESPACE_UNEXPECTED;
	}
	else
	{
		file_size2 = 0;
		file_size = file_size2;
	}
//	else
//		return E_FILESPACE_NOTFOUND;

	FILE *f = NULL;
	f = _tfopen(szFileName,_T("rb"));
	if (f != NULL)
		fclose(f);
//		return E_FILESPACE_CANNOT_OPEN;
	//fclose(f);

	m_arFiles.Add(szFileName);
	if (use_size != 0) file_size = use_size;
	m_arRealSizes.Add(file_size2);
	m_arSizes.Add(file_size);

	if (file_size % m_nSectorSize)
	{
		file_size += m_nSectorSize - (file_size % m_nSectorSize);
	}
	m_arIgnore.Add(ignore);

	m_nTotalSize += file_size;
	return 0;
}

int CFileSpace::SetSectorSize(int sector_size)
{
	if (sector_size<512) return -1;
	m_nSectorSize = sector_size;

	FILESIZE size = 0;
	m_nTotalSize = 0;

	int i;
	for (i=0; i<m_nFilesCount; i++)
	{
		if (i<m_arFiles.GetSize() && i<m_arSizes.GetCount())
		{
			size = m_arSizes.GetAt(i);
			if (size % m_nSectorSize > 0)
			{
				size += m_nSectorSize - (size % m_nSectorSize);
			}
			m_nTotalSize += size;
		}		
	}	

	return 0;
}

int CFileSpace::GetSectorSize()
{
	return m_nSectorSize;
}

FILESIZE CFileSpace::GetAllSize()
{
	return m_nTotalSize;
}


void CFileSpace::Close()
{
	if (file!=NULL) 
	{
		fclose(file);
		file = NULL;
	}
	m_nCurrentFile = -1;
	m_nCurrentPos = 0;
}

int CFileSpace::GetCurrentFile(CString &szFileName)
{
	if (m_nCurrentFile>=0 && m_nCurrentFile<m_arFiles.GetSize())
		szFileName = m_arFiles.GetAt(m_nCurrentFile);
	else
		return E_FILESPACE_EOF;
	return m_nCurrentFile;
}

bool CFileSpace::eof()
{
	if (m_nCurrentFile>=0)
		return false;
	else
		return true;
}

FILESIZE CFileSpace::GetPos()
{
	return m_nCurrentPos;
}

FILESIZE CFileSpace::GetSector()
{
	return int(m_nCurrentPos / m_nSectorSize);
}

int CFileSpace::GetFileNumberAndPos(FILESIZE nPos, int &nFile, FILESIZE &nFilePos)
{
	if (nPos>=m_nTotalSize) return -1;

	int i=0;
	FILESIZE cur_pos = 0;
	if (nPos==0)
	{
		nFilePos=0;
		nFile=0;
	}
	while (cur_pos<=nPos)
	{
		FILESIZE size = m_arSizes.GetAt(i);
		if (size % m_nSectorSize > 0)
			size += m_nSectorSize - (size % m_nSectorSize);

		if (cur_pos+size<=nPos)
		{
			i++;
			cur_pos += size;
		}
		else
		{
			nFilePos = nPos - cur_pos;
			nFile = i;
			break;
		}
	}
	return 0;
}

int CFileSpace::GetFileName(int n, CString &szFileName)
{
	if (n<0 || n>m_arFiles.GetSize()-1)
		return -1;
	szFileName = m_arFiles.GetAt(n);
	return 0;
}

int CFileSpace::EnsurePathExist(LPCTSTR szFileName)
{
	struct _stati64 statfile;
	if( _tstati64( szFileName, &statfile ) == 0 )
		return 0;

	CString szPathFileName = szFileName;
	int pos = szPathFileName.ReverseFind('\\');
	
	CString szXPath="";

	if (pos>0) 
		szXPath = szPathFileName.Left(pos);
	else
		return 0;
	
	CStringArray m_arPaths;

	m_arPaths.Add(szXPath);
	pos = szXPath.ReverseFind('\\');

	while(pos>0)
	{
		szXPath = szXPath.Left(pos);
		m_arPaths.Add(szXPath);
		pos = szXPath.ReverseFind('\\');
	}

	int i;
	for (i=m_arPaths.GetSize()-1; i>=0; i--)
	{
		szXPath = m_arPaths.GetAt(i);
		if (szXPath.Right(1)!=':') //If pathname is not like "C:" or "D:"
		{
			if( _tstati64( szXPath, &statfile ) != 0 )
			{
				int rc = _tmkdir(szXPath);
				if (rc!=0)
					return E_FILESPACE_CANNOT_WRITE;
			}
		}
	}	
	return 0;
}

int CFileSpace::SetSector(FILESIZE nSector, bool allow_writes=false)
{
	FILESIZE desired_position = nSector*m_nSectorSize;
	//if (desired_position == m_nCurrentPos && m_nCurrentFile>=0)	return 0;

	int nFile;
	FILESIZE nFilePos;
	int res = GetFileNumberAndPos(desired_position, nFile, nFilePos);
	if (res!=0) return E_FILESPACE_EOF;

	m_bReadOnly = !allow_writes;

	if (nFile!=m_nCurrentFile || file==NULL)
	{
		if (file!=NULL)
		{
			fclose(file);
			file = NULL;
		}
		int ign = m_arIgnore.GetAt(nFile);
		if (ign!=0)
		{
			m_nCurrentFile = nFile;
			m_nCurrentFilePos = nFilePos;
			m_nCurrentPos = desired_position;
			m_bCurIgnored=true;
			return 0;
		}
		else
			m_bCurIgnored=false;
		CString szFileName = m_arFiles.GetAt(nFile);
		FILESIZE sz1 = m_arSizes.GetAt(nFile);
		FILESIZE sz2 = m_arRealSizes.GetAt(nFile);
		if (!m_bReadOnly && sz1!=sz2)
		{
			int rc = EnsurePathExist(szFileName);
			if (rc!=0) return E_FILESPACE_NO_PERMISSION;

			struct _stati64 statfile;
			if( _tstati64( szFileName, &statfile ) != 0 )
			{
				FILE *ff = _tfopen(szFileName,_T("w"));
				if (ff==NULL)
					return E_FILESPACE_CANNOT_OPEN;
				else
					fclose(ff);
			}

			int fh, result;
			if( (fh = _topen( szFileName, _O_RDWR, _S_IREAD | _S_IWRITE ))  != -1 )
			{
				result = _chsize_s( fh, sz1 );
				_close( fh );
				m_arRealSizes.SetAt(nFile, sz1);
			}
			else
				return E_FILESPACE_CANNOT_OPEN;
		}

		if (m_bReadOnly)
			file = _tfopen (szFileName,_T("rb"));
		else
			file = _tfopen (szFileName,_T("rb+"));
		if (file == NULL) 
		{
			//m_nCurrentFile = -1;
			//return E_FILESPACE_CANNOT_OPEN;
		}
	}

	FILESIZE sz = m_arRealSizes.GetAt(nFile);
	if (nFilePos<sz && file!=NULL)
	{
		int rc = _fseeki64(file,  nFilePos, SEEK_SET);
		if (rc!=0)
		{
			if (file!=NULL) fclose(file);
			file=NULL;
			m_nCurrentFile = -1;
			return E_FILESPACE_UNEXPECTED;
		}
	}
	else
	{
		if (file!=NULL) fclose(file);
		file=NULL;
	}

	m_nCurrentFile = nFile;
	m_nCurrentFilePos = nFilePos;
	m_nCurrentPos = desired_position;
	return 0;
}

int CFileSpace::ReadSector(void *buf)
{
	size_t size = 0;

	while (size < (size_t) m_nSectorSize)
	{
		if (file!=NULL)
		{
			if (!feof(file))
			{
				FILESIZE sz1 = m_arSizes.GetAt(m_nCurrentFile);

				if (sz1 < m_nCurrentFilePos)
				{
					size = 0;
				}
				else
				{
					size = fread(buf, 1, m_nSectorSize, file);
					if (size == 0)
					{
						fclose(file);
						file = NULL;
					}
					else
					{
						if (size < (size_t) m_nSectorSize)
						{
							memset(((char *)buf)+size, 0, m_nSectorSize - size);
							size = m_nSectorSize;
						}
						m_nCurrentFilePos+=m_nSectorSize;
					}
				}
			}
			else
			{
				fclose(file);
				file = NULL;
			}
		}
		else
		{
			if (m_bCurIgnored)
			{
				FILESIZE sz1 = m_arSizes.GetAt(m_nCurrentFile);

				if (sz1 < m_nCurrentFilePos)
				{
					size = 0;
				}
				else
				{
					size = (size_t) m_nSectorSize;
					m_nCurrentFilePos+=m_nSectorSize;
				}
			}
		}
		if (size < (size_t) m_nSectorSize)
		{
			int nFile;
			FILESIZE nFilePos;
			int ret = GetFileNumberAndPos(m_nCurrentPos, nFile, nFilePos);
			if (ret!=0) return E_FILESPACE_EOF;
			if (nFile!=m_nCurrentFile)
			{
				if (file!=NULL)
				{
					fclose(file);
					file=NULL;
				}
				CString szFileName = m_arFiles.GetAt(nFile);

				m_nCurrentFile = nFile;
				m_nCurrentFilePos = 0;

				int ign = m_arIgnore.GetAt(nFile);
				if (ign!=0)
				{
					memset(buf, 0, m_nSectorSize);
					size = m_nSectorSize;
					m_nCurrentFilePos+=m_nSectorSize;
					m_bCurIgnored = true;
					continue;
				}
				else
					m_bCurIgnored = false;

				FILESIZE sz1 = m_arSizes.GetAt(nFile);
				FILESIZE sz2 = m_arRealSizes.GetAt(nFile);
				if (!m_bReadOnly && sz1!=sz2)
				{
					int fh, result;
					if( (fh = _topen( szFileName, _O_RDWR, _S_IREAD | _S_IWRITE ))  != -1 )
					{
						result = _chsize_s( fh, sz1 );
						_close( fh );
						m_arRealSizes.SetAt(nFile, sz1);
					}
					else
						return E_FILESPACE_CANNOT_OPEN;
				}

				if (m_bReadOnly)
					file = _tfopen (szFileName,_T("rb"));
				else
					file = _tfopen (szFileName,_T("rb+"));

				if (file == NULL && !m_bReadOnly) return E_FILESPACE_CANNOT_OPEN;
				if (file == NULL && m_bReadOnly)
				{
					memset(buf, 0, m_nSectorSize);
					size = m_nSectorSize;
					m_nCurrentFilePos+=m_nSectorSize;
				}
			}
			else
			{
				memset(buf, 0, m_nSectorSize);
				size = m_nSectorSize;
				m_nCurrentFilePos+=m_nSectorSize;
			}
		}
	}
	m_nCurrentPos+=m_nSectorSize;
	return 0;
}

int CFileSpace::WriteSector(void *buf)
{
	if (m_bReadOnly) return E_FILESPACE_NO_PERMISSION;

	int to_write = 0;
	FILESIZE fz = m_arRealSizes.GetAt(m_nCurrentFile);

	bool bWritten = false;
	while (!bWritten)
	{
		if (file!=NULL)
		{
			if (m_nCurrentFilePos + m_nSectorSize < fz)
			{
				to_write = m_nSectorSize;
			}
			else
			{
				to_write = (size_t) (fz - m_nCurrentFilePos);
				if (to_write <= 0)
				{
					FILESIZE sector = m_nCurrentPos / m_nSectorSize;
					int rc = SetSector(sector, true);
					if (rc!=0) return rc;
					continue;
				}
			}

			size_t size = fwrite(buf, 1, to_write, file);
			if (size<=0)
			{
				return E_FILESPACE_CANNOT_WRITE;
			}
			else
			{
				m_nCurrentPos+=m_nSectorSize;
				m_nCurrentFilePos+=m_nSectorSize;
				bWritten = true;
			}
		}
		else
		{
			FILESIZE sector = m_nCurrentPos / m_nSectorSize;
			int rc = SetSector(sector, true);
			if (rc!=0) return rc;
			continue;
		}
	}

	return 0;
}

