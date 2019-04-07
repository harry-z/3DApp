#pragma once

#include "PreHeader.h"

class DLL_EXPORT CFile {
public:
	CFile();
	~CFile();
	bool Open(const char *pszFileName, const char *pszMode);
	inline bool IsOpen() const { return m_pFile != nullptr; }
	void Close();
	size_t Write(const void *pBuffer, size_t nSizeInBytes);
	size_t ReadRaw(INOUT void *pBuffer, size_t nSizeInBytes);
	template <class T> inline size_t ReadTypeRaw(INOUT T *pTypeBuffer, dword nSize = 1) {
		return ReadRaw((void*)pTypeBuffer, sizeof(T)*nSize);
	}
	long Length();
	bool Seek(long seek, int mode);
	bool SeekToBegin();
	bool SeekToEnd();
	long GetPosition();
	bool IsEof();
	void Flush();
	inline FILE* GetHandle() const { return m_pFile; }
private:
	FILE *m_pFile;
};