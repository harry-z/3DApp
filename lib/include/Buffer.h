#pragma once

#include "Str.h"

inline void AddBytes(INOUT byte *&pBuffer, const byte *bytes, dword n) {
	assert(pBuffer != nullptr && bytes != nullptr);
	for (dword i = 0; i < n; ++i)
		*pBuffer++ = bytes[i];
}

inline void AddWords(INOUT byte *&pBuffer, const word *words, dword n) {
	assert(pBuffer != nullptr && words != nullptr);
	word *pWords = (word*)pBuffer;
	for (dword i = 0; i < n; ++i)
		*pWords++ = words[i];
	pBuffer = (byte*)pWords;
}

inline void AddDwords(INOUT byte *&pBuffer, const dword *dwords, dword n) {
	assert(pBuffer != nullptr && dwords != nullptr);
	dword *pDwords = (dword*)pBuffer;
	for (dword i = 0; i < n; ++i)
		*pDwords++ = dwords[i];
	pBuffer = (byte*)pDwords;
}

inline void AddFloats(INOUT byte *&pBuffer, const float *floats, dword n) {
	assert(pBuffer != nullptr && floats != nullptr);
	float *pFloats = (float*)pBuffer;
	for (dword i = 0; i < n; ++i)
		*pFloats++ = floats[i];
	pBuffer = (byte*)pFloats;
}

inline void AddString(INOUT byte *&pBuffer, const String &str) {
	assert(pBuffer != nullptr);
	String::size_type len = str.length();
	for (String::size_type i = 0; i < len; ++i)
		*pBuffer++ = str[i];
	*pBuffer++ = 0;
}

inline void GetBytes(INOUT byte *&pBuffer, INOUT byte *bytes, dword n) {
	assert(pBuffer != nullptr && bytes != nullptr);
	for (dword i = 0; i < n; ++i)
		bytes[i] = *pBuffer++;
}

inline void GetWords(INOUT byte *&pBuffer, INOUT word *words, dword n) {
	assert(pBuffer != nullptr && words != nullptr);
	word *pWords = (word*)pBuffer;
	for (dword i = 0; i < n; ++i)
		words[i] = *pWords++;
	pBuffer = (byte*)pWords;
}

inline void GetDwords(INOUT byte *&pBuffer, INOUT dword *dwords, dword n) {
	assert(pBuffer != nullptr && dwords != nullptr);
	dword *pDwords = (dword*)pBuffer;
	for (dword i = 0; i < n; ++i)
		dwords[i] = *pDwords++;
	pBuffer = (byte*)pDwords;
}

inline void GetFloats(INOUT byte *&pBuffer, INOUT float *floats, dword n) {
	assert(pBuffer != nullptr && floats != nullptr);
	float *pFloats = (float*)pBuffer;
	for (dword i = 0; i < n; ++i)
		floats[i] = *pFloats++;
	pBuffer = (byte*)pFloats;
}

inline String GetString(INOUT byte *&pBuffer) {
	assert(pBuffer != nullptr);
	String str((char*)pBuffer);
	pBuffer += (str.length() + 1 + 1);
	return str;
}