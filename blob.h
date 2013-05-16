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
#ifndef _BLOB_H_UNCLUDED_09_06_2007
#define _BLOB_H_UNCLUDED_09_06_2007

class CBlob
{
public:
	CBlob ();
	CBlob(CBlob &ob);
	CBlob (unsigned long int size);
	~CBlob();

	CBlob &CBlob::operator =(const CBlob ob);

	CBlob &operator<<(unsigned int a);
	CBlob &operator<<(int a);
	CBlob &operator<<(short a);
	CBlob &operator<<(long a);
	CBlob &operator<<(float a);
	CBlob &operator<<(double a);
	CBlob &operator<<(char a);
	CBlob &operator<<(char *a);
	CBlob &operator<<(wchar_t *a);
	CBlob &operator<<(CBlob a);

	CBlob &operator>>(int &a);
	CBlob &operator>>(unsigned int &a);
	CBlob &operator>>(short &a);
	CBlob &operator>>(long &a);
	CBlob &operator>>(float &a);
	CBlob &operator>>(double &a);
	CBlob &operator>>(char &a);
	CBlob &operator>>(char *&a);
	CBlob &operator>>(wchar_t *a);

	int Set (void *mem, unsigned long int memsize);
	int Write(void *mem, unsigned long int memsize);
	int Seek(unsigned long int index);
	int Read(void *mem, unsigned long int memsize);
	int SetSize(unsigned long int size);
	void ResetIndex();
	int EnsureSize(unsigned long int size);
	char *GetBuffer();
	unsigned long int GetSize();

private:
	int Reallocate(unsigned long int newsize);

	char *m_buffer;
	unsigned long int m_alloc;
	unsigned long int m_inbuf;
	unsigned long int m_index;

	enum Error
	{
		BLOB_OK=0,
		BLOB_SMALLBUFFER=1,
		BLOB_OUTOFMEMORY=2,
		BLOB_BUFFEREXHAUSTED=3,
		BLOB_OUTOFBUFFER=4
	} m_error;
};

void ConvertTobase64 (char *buf1, char *buf2);

#endif //_BLOB_H_UNCLUDED_09_06_2007