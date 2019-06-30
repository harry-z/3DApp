#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

class IHardwareBuffer {
public:
	virtual ~IHardwareBuffer() {}
	
	virtual bool UpdateData(const byte *pData, dword nSize) = 0;

	EHardwareBufferType Type() const { return m_Type; }
	dword Count() const { return m_nCount; }
	dword LockedLength() const { return m_nLockedLength; }

protected:
	EHardwareBufferType m_Type;
	dword m_nCount;
	dword m_nLockedLength;
};
