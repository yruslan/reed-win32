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
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "blob.h"

CBlob::CBlob()
{
	m_buffer = NULL;
	m_alloc = 0;
	m_inbuf = 0;
	m_index = 0;
	m_error = BLOB_OK;
}


CBlob::CBlob(CBlob &ob)
{
	m_buffer = NULL;
	m_alloc = ob.m_alloc;
	m_inbuf = ob.m_inbuf;
	m_index = ob.m_index;
	m_error = ob.m_error;
	if (m_alloc>0)
	{
		m_buffer = new char[m_alloc];
		if (m_buffer == NULL) 	
		{	
			m_error = BLOB_OUTOFMEMORY;
			m_alloc = 0;
			m_inbuf = 0;
		}
		else
		{
			if (m_inbuf > 0)
				memcpy(m_buffer, ob.m_buffer, m_inbuf);
		}
	}
}
CBlob::CBlob(unsigned long int size)
{
	m_buffer = NULL;
	m_alloc = 0;
	m_inbuf = 0;
	m_index = 0;
	m_error = BLOB_OK;

	Reallocate(size);
}

CBlob::~CBlob()
{
	if (m_buffer != NULL)
	{
		delete [] m_buffer;
		m_buffer=NULL;
	}
	m_alloc = 0;
	m_inbuf = 0;
	m_index = 0;
}

CBlob &CBlob::operator =(const CBlob ob)
{
	if (m_buffer != NULL)
	{
		delete [] m_buffer;
		m_buffer=NULL;
	}

	if (ob.m_alloc>0)
	{
		m_buffer = new char[ob.m_alloc];
		if (m_buffer == NULL) 	
		{	
			m_error = BLOB_OUTOFMEMORY;
			m_alloc = 0;
			m_inbuf = 0;
		}
		else
		{
			if (ob.m_inbuf > 0)
				memcpy(m_buffer, ob.m_buffer, ob.m_inbuf);
		}
	}
	m_alloc = ob.m_alloc;
	m_inbuf = ob.m_inbuf;
	m_index = ob.m_index;
	m_error = ob.m_error;
	return *this;
}

CBlob &CBlob::operator<<(unsigned int a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(int a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(short a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(long a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(float a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(double a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(char a)
{
	Write ((char *) &a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator<<(char *a)
{
	if (strlen(a)>0)
	{
		unsigned int len = strlen(a);
		Write ((char *) &len, sizeof(len));
		Write ((char *) a, (strlen(a)+1)*sizeof(char));
	}
	return *this;
}

CBlob &CBlob::operator<<(wchar_t *a)
{
	if (wcslen(a)>0)
	{
		unsigned int len = wcslen(a);
		Write ((char *) &len, sizeof(len));
		Write ((char *) a, (wcslen(a)+1)*sizeof(wchar_t));
	}
	return *this;
}

CBlob &CBlob::operator<<(CBlob a)
{
	if (a.m_inbuf>0)
	{
		Write ((char *) a.GetBuffer(), a.m_inbuf);
	}
	return *this;
}

CBlob &CBlob::operator>>(int &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(unsigned int &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(short &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(long &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(float &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(double &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(char &a)
{
	Read ((char *)&a, sizeof(a));
	return *this;
}

CBlob &CBlob::operator>>(char *&a)
{
	unsigned int len;
	Read ((char *) &len, sizeof(len));
	
	if ((len+1)*sizeof(char)+m_index<m_inbuf && len>0)
	{
		a = new char[len+1];
		Read (a, (len+1)*sizeof(char));
	}
	
	return *this;
}

CBlob &CBlob::operator>>(wchar_t *a)
{
	unsigned int len;
	Read ((char *) &len, sizeof(len));
	
	if ((len+1)*sizeof(wchar_t)+m_index<m_inbuf && len>0)
	{
		a = new wchar_t[len+1];
		Read (a, (len+1)*sizeof(wchar_t));
	}
	
	return *this;
}

int CBlob::Reallocate(unsigned long int newsize)
{
	if (newsize==m_alloc) return true;
	
	if (newsize==0)
	{
		delete [] m_buffer;
		m_buffer = NULL;
		m_alloc = newsize;
		m_inbuf = 0;
		return BLOB_OK;
	}
	
	char *newbuffer = new char[newsize];
	if (newbuffer == NULL)
	{
		m_error=BLOB_OUTOFMEMORY;
		return false;
	}
	if (m_alloc!=0)
	{
		if (m_inbuf>newsize) m_inbuf = newsize;
		memcpy(newbuffer, m_buffer, m_inbuf);
		delete [] m_buffer;
	}
	m_buffer = newbuffer;
	m_alloc = newsize;
	
	return BLOB_OK;
}

void CBlob::ResetIndex()
{
	m_index=0;
	return;
}

int CBlob::SetSize(unsigned long int size)
{
	int r = Reallocate(size);
	m_inbuf = size;
	return r;
}

int CBlob::EnsureSize(unsigned long int size)
{
	if (m_alloc>=size) return BLOB_OK;
	return Reallocate(size);
}

int CBlob::Write (void *mem, unsigned long int memsize)
{
	if (m_error != 0) return m_error;
	if ((m_inbuf+memsize) > m_alloc) 
		{
			Reallocate ((m_inbuf + memsize + 1)*2);
			if (m_error!=0) return 0;
		}
	memcpy(m_buffer+m_inbuf, mem, memsize);
	m_inbuf+=memsize;

	return memsize;
}

int CBlob::Seek (unsigned long index)
{
	if (index > m_inbuf)
		return BLOB_OUTOFBUFFER;
	m_index=index;
	return BLOB_OK;
}

int CBlob::Read (void *mem, unsigned long int memsize)
{
	if (m_error != 0) return m_error;
	if ((m_index+memsize-1) > m_inbuf)
	{
		int r = m_inbuf - m_index;
		memcpy(mem, m_buffer+m_index, m_inbuf - m_index);
		m_index = m_inbuf;
		m_error = BLOB_BUFFEREXHAUSTED;
		return r;
	}

	memcpy(mem, m_buffer+m_index, memsize);
	m_index += memsize;

	return memsize;
}

int CBlob::Set (void *mem, unsigned long int memsize)
{
	int er = Reallocate(memsize);
	
	if (er!=BLOB_OK)
		return er;

	memcpy(m_buffer, mem, memsize);
	m_inbuf = memsize;
	m_index = 0;
	
	return BLOB_OK;
}

char *CBlob::GetBuffer()
{
	return m_buffer;
}

unsigned long int CBlob::GetSize()
{
	return m_inbuf;
}

void ConvertTobase64 (char *buf1, char *buf2)
{
	char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned int len=strlen(buf1);
	int i=0;
	
	// strcpy(buf2,""); PGT 2008-09-25
	buf2[0] = NULL;
	while (i<int(len))
	{
		len+=3;
	}
}

