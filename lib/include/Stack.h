#pragma once

#include "PreHeader.h"

class CStack {
public:
	CStack();
	~CStack();

	bool Initialize(dword nSize);
	void* Allocate(dword nBytes);
	inline void Clear() { m_nOffset = 0; }

private:
	inline bool NotEnoughMemory(dword nSize) const {
		return m_nOffset + nSize > m_nSize;
	}

private:
	dword m_nSize;
	dword m_nOffset;
	byte *m_pBuffer;
};