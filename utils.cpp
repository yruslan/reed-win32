#include "StdAfx.h"
#include "utils.h"

void NumbersToBytes(int blocks, int block_size, CString &msg)
{
	unsigned __int64 bytes = ((unsigned __int64)blocks)*((unsigned __int64)block_size);

	if (bytes<1*1024)
	{
		msg.Format(_T("%u bytes"), bytes);
	}
	else
	{
		if (bytes<5*1024*1024)
			msg.Format(_T("%u KB"), (unsigned __int64) (bytes/1024));
		else
		{
			if (bytes<1*1024*1024*1024)
				msg.Format(_T("%d MB"), int(bytes/(1024*1024)));
			else
				msg.Format(_T("%.1f GB"), double(bytes/(1024*1024))/1024.);
		}
	}
}

void HumanReadibleMegabytes(int nMB, CString &msg)
{
	if (nMB<1024)
		msg.Format(_T("%d MB"),  nMB);
	else
		msg.Format(_T("%.1f GB"),  double(nMB)/1024.);
}
