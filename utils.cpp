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

void HumanReadableMegabytes(int nMB, CString &msg)
{
	if (nMB<1024)
		msg.Format(_T("%d MB"),  nMB);
	else
		msg.Format(_T("%.1f GB"),  double(nMB)/1024.);
}
