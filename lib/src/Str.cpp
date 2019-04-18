#include "Str.h"

String::size_type String::find(const char *pszStr, size_type nOff /* = 0 */) const {
	if (m_nLen < strlen(pszStr))
		return npos;

	size_type l, i, j;
	l = m_nLen - strlen(pszStr);
	for (i = nOff; i <= l; ++i) {
		for (j = 0; pszStr[j]; ++j) {
			if (m_pData[i + j] != pszStr[j])
				break;
		}
		if (!pszStr[j])
			return i;
	}
	return npos;
}

String::size_type String::find_first_of(const char *pszStr, size_type nOff /* = 0 */) const {
	for (size_type i = nOff; i < m_nLen; ++i) {
		if (strchr(pszStr, m_pData[i]))
			return i;
	}
	return npos;
}

void String::Reallocate(size_type nSize) {
	size_type newSize;

	size_type mod = nSize % STR_ALLOC_GRAN;
	if (!mod)
		newSize = nSize;
	else
		newSize = nSize + STR_ALLOC_GRAN - mod;

	char *pTempData = (char *)MEMALLOC(newSize);
	assert(pTempData != nullptr);
	strcpy(pTempData, m_pData);
	pTempData[m_nLen] = 0;
	MEMFREE(m_pData);
	m_pData = pTempData;
	m_nCapacity = newSize;
}

void String::Append(const char *pszStr) {
	size_type nLen = strlen(pszStr);
	size_type newLen = m_nLen + nLen;

	EnsureAllocated(newLen + 1);
	memcpy(m_pData + m_nLen, pszStr, nLen);
	m_pData[newLen] = 0;
	m_nLen = newLen;
}

void String::Append(const String &other) {
	size_type nLen = other.length();
	size_type newLen = m_nLen + nLen;

	EnsureAllocated(newLen + 1);
	memcpy(m_pData + m_nLen, other.c_str(), nLen);
	m_pData[newLen] = 0;
	m_nLen = newLen;
}