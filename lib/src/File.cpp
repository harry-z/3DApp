#include "File.h"

CFile::CFile()
	: m_pFile(nullptr) {}
CFile::~CFile() {
	Close();
}
bool CFile::Open(const char *pszFileName, const char *pszMode) {
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
	long pos = ftell(m_pFile);
	fseek(m_pFile, 0, SEEK_END);
	long len = ftell(m_pFile);
	fseek(m_pFile, pos, SEEK_SET);
	return len;
}
bool CFile::Seek(long seek, int mode) {
	return fseek(m_pFile, seek, mode) == 0;
}
bool CFile::SeekToBegin() {
	return fseek(m_pFile, 0, SEEK_SET) == 0;
}
bool CFile::SeekToEnd() {
	return fseek(m_pFile, 0, SEEK_END) == 0;
}
long CFile::GetPosition() {
	return ftell(m_pFile);
}
bool CFile::IsEof() {
	return feof(m_pFile) != 0;
}
void CFile::Flush() {
	fflush(m_pFile);
}