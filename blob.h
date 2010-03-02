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