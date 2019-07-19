#pragma once

#include "Memory.h"

const dword STR_ALLOC_BASE = 32;
const dword STR_ALLOC_GRAN = 32;

class DLL_EXPORT String {
public:
	typedef dword size_type;
	static const size_type npos = 0xFFFFFFFF;

	String() { Init(); }
	String(const char *pszStr);
	String(const String &other);
	~String() { MEMFREE(m_pData); }

	String& operator= (const char *pszStr);
	String& operator= (const String &other);

	bool operator== (const char *pszStr) const {
		assert(pszStr != nullptr);
		return strcmp(m_pData, pszStr) == 0;
	}
	bool operator== (const String &other) const {
		return strcmp(m_pData, other.c_str()) == 0;
	}
	friend bool operator== (const char *pszStr, const String &str) {
		assert(pszStr != nullptr);
		return strcmp(pszStr, str.c_str()) == 0;
	}

	String& operator+= (const char *pszStr) {
		Append(pszStr);
		return *this;
	}
	String& operator+= (const String &other) {
		Append(other);
		return *this;
	}

	String operator+ (const char *pszStr) const {
		String str(*this);
		str += pszStr;
		return str;
	}
	String operator+ (const String &other) const {
		String str(*this);
		str += other;
		return str;
	}
	friend String operator+ (const char *pszStr, const String &str) {
		String s(str);
		s += pszStr;
		return s;
	}

	char& operator[] (size_type index) { 
		assert(index < m_nLen); 
		return m_pData[index]; 
	}
	char operator[] (size_type index) const { 
		assert(index < m_nLen); 
		return m_pData[index]; 
	}

	

	// STL compatible functions
	inline const char* c_str() const { return m_pData; }
	inline size_type length() const { return m_nLen; }
	inline bool empty() const { return m_nLen == 0; }
	void clear();

	inline char& at(size_type index) {
		return operator[] (index);
	}
	inline char at(size_type index) const {
		return operator[] (index);
	}

	size_type find(char elem, size_type nOff = 0) const;
	size_type rfind(char elem, size_type nOff = 0) const;

	size_type find(const char *pszStr, size_type nOff = 0) const;
	size_type find_first_of(const char *pszStr, size_type nOff = 0) const;

	String substr(size_type nOff = 0, size_type nCount = npos) const;

	void push_back(char elem);

	String Left(size_type nLength) const;
	String Right(size_type nLength) const;
	void LeftSelf(size_type nLength);
	void RightSelf(size_type nLength);

	void Reserve(size_type nLength) { EnsureAllocated(nLength); }

private:
	void Init();
	void EnsureAllocated(size_type nSize);
	void Reallocate(size_type nSize);
	void Append(const char *pszStr);
	void Append(const String &other);

private:
	char *m_pData;
	size_type m_nLen;
	size_type m_nCapacity;
};

inline String::String(const char *pszStr) {
	assert(pszStr != nullptr);

	Init();

	size_type nLen = strlen(pszStr);
	EnsureAllocated(nLen + 1);
	strcpy(m_pData, pszStr);
	m_pData[nLen] = 0;
	m_nLen = nLen;
}

inline String::String(const String &other) {
	Init();

	size_type nLen = other.length();
	EnsureAllocated(nLen + 1);
	strcpy(m_pData, other.c_str());
	m_pData[nLen] = 0;
	m_nLen = nLen;
}

inline String& String::operator=(const char *pszStr) {
	assert(pszStr != nullptr);
	size_type nLen = strlen(pszStr);
	EnsureAllocated(nLen + 1);
	strcpy(m_pData, pszStr);
	m_pData[nLen] = 0;
	m_nLen = nLen;
	return *this;
}

inline String& String::operator=(const String &other) {
	size_type nLen = other.length();
	EnsureAllocated(nLen + 1);
	strcpy(m_pData, other.c_str());
	m_pData[nLen] = 0;
	m_nLen = nLen;
	return *this;
}

inline void String::clear() {
	MEMFREE(m_pData);
	Init();
}

inline String::size_type String::find(char elem, size_type nOff /* = 0 */) const {
	for (size_type n = nOff; n < m_nLen; ++n)
		if (m_pData[n] == elem)
			return n;
	return npos;
}

inline String::size_type String::rfind(char elem, size_type nOff /* = 0 */) const {
	for (int n = m_nLen - 1 - nOff; n >= 0; --n)
		if (m_pData[n] == elem)
			return n;
	return npos;
}

inline String String::substr(size_type nOff /* = 0 */, size_type nCount /* = npos */) const {
	String str;
	size_type nSize = nCount == npos ? (m_nLen - nOff) : nCount;
	str.EnsureAllocated(nSize + 1);
	memcpy(str.m_pData, m_pData + nOff, nSize);
	str.m_pData[nSize] = 0;
	str.m_nLen = nSize;
	return std::move(str);
}

inline void String::push_back(char elem) {
	// 1 for new char, 1 for nul terminate
	EnsureAllocated(m_nLen + 1 + 1);
	m_pData[m_nLen] = elem;
	m_pData[m_nLen + 1] = 0;
	++m_nLen;
}

inline String String::Left(size_type nLength) const {
	return substr(0, nLength);
}

inline String String::Right(size_type nLength) const {
	return substr(length() - nLength, nLength);
}

inline void String::LeftSelf(size_type nLength) {
	m_pData[nLength] = '\0';
	m_nLen = nLength;
}

inline void String::RightSelf(size_type nLength) {
	size_type n = length() - nLength;
	for (size_type i = 0; i < nLength; ++i)
		m_pData[i] = m_pData[n + i];
	m_pData[nLength] = '\0';
	m_nLen = nLength;
}

inline void String::Init() {
	m_nLen = 0;
	m_nCapacity = STR_ALLOC_BASE;
	m_pData = (char *)MEMALLOC(STR_ALLOC_BASE);
	assert(m_pData != nullptr);
	m_pData[0] = 0;
}

inline void String::EnsureAllocated(size_type nSize) {
	if (nSize > m_nCapacity)
		Reallocate(nSize);
}