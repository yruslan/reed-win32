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
