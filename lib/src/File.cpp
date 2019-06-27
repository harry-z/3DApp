#include "File.h"

CFile::CFile()
	: m_pFile(nullptr) {}
CFile::~CFile() {
	Close();
}
bool CFile::Open(const char *pszFileName, const char *pszMode) {
	if (pszFileName == nullptr || pszMode == nullptr)
		return false;

	Close();
   	m_pFile = fopen(pszFileName, pszMode);
	return m_pFile != nullptr;
}
void CFile::Close() {
	if (m_pFile != nullptr) {
		fclose(m_pFile);
		m_pFile = nullptr;
	}
}
size_t CFile::Write(const void *pBuffer, size_t nSizeInBytes) {
	assert(pBuffer != nullptr);
	return fwrite(pBuffer, 1, nSizeInBytes, m_pFile);
}
size_t CFile::ReadRaw(INOUT void *pBuffer, size_t nSizeInBytes) {
	assert(pBuffer != nullptr);
	memset(pBuffer, 0, nSizeInBytes);
	return fread(pBuffer, 1, nSizeInBytes, m_pFile);
}
long CFile::Length() {
	assert(m_pFile);
	long pos = ftell(m_pFile);
	fseek(m_pFile, 0, SEEK_END);
	long len = ftell(m_pFile);
	fseek(m_pFile, pos, SEEK_SET);
	return len;
}
bool CFile::Seek(long seek, int mode) {
	assert(m_pFile);
	return fseek(m_pFile, seek, mode) == 0;
}
bool CFile::SeekToBegin() {
	assert(m_pFile);
	return fseek(m_pFile, 0, SEEK_SET) == 0;
}
bool CFile::SeekToEnd() {
	assert(m_pFile);
	return fseek(m_pFile, 0, SEEK_END) == 0;
}
long CFile::GetPosition() {
	assert(m_pFile);
	return ftell(m_pFile);
}
bool CFile::IsEof() {
	assert(m_pFile);
	return feof(m_pFile) != 0;
}
void CFile::Flush() {
	assert(m_pFile);
	fflush(m_pFile);
}

void NewFile(const char *pszFileName) {
#if TARGET_PLATFORM == PLATFORM_WINDOWS
	CreateFile(pszFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
}

void DelFile(const char *pszFileName) {
#if TARGET_PLATFORM == PLATFORM_WINDOWS
	DeleteFile(pszFileName);
#endif
}