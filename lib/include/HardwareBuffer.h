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

class CHardwareBufferManager {
public:
	virtual ~CHardwareBufferManager() {}

	virtual IHardwareBuffer* CreateVertexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) = 0;
	virtual IHardwareBuffer* CreateIndexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) = 0;
	virtual void DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) = 0;
	virtual void DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) = 0;

};