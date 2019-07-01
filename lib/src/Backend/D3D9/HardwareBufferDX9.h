#pragma once
#include "..\..\RendererStableHeader.h"
#include "HardwareBuffer.h"
#include "GPUResource.h"

class CVertexBufferDX9 final : public IHardwareBuffer, public IGpuResource {
public:
	friend class CHardwareBufferManagerD3D9;
	CVertexBufferDX9(dword nLength, dword nCount) 
		: m_pVertexBuffer(nullptr), m_pSysmemBuffer(nullptr) {
		m_Type = EHardwareBufferType::EHardwareBuffer_Vertex;
		m_nLockedLength = nLength;
		m_nCount = nCount;
	}
	virtual ~CVertexBufferDX9();

	virtual bool UpdateData(const byte *pData, dword nSize) override;
	virtual void OnDeviceLost() override { SAFE_RELEASE(m_pVertexBuffer); }
	virtual void OnDeviceReset(float, float) override;

	bool CopyData(const byte *pData, dword nSize);

	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	byte *m_pSysmemBuffer;
};

class CIndexBufferDX9 final : public IHardwareBuffer, public IGpuResource {
public:
	friend class CHardwareBufferManagerD3D9;
	CIndexBufferDX9(dword nLength, dword nCount)
		: m_pIndexBuffer(0), m_pSysmemBuffer(0) {
		m_Type = EHardwareBufferType::EHardwareBuffer_Index;
		m_nLockedLength = nLength;
		m_nCount = nCount;
	}
	virtual ~CIndexBufferDX9();

	virtual bool UpdateData(const byte *pData, dword nSize) override;
	virtual void OnDeviceLost() override { SAFE_RELEASE(m_pIndexBuffer); }
	virtual void OnDeviceReset(float, float) override;

	bool CopyData(const byte *pData, dword nSize);

	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	byte *m_pSysmemBuffer;
};

class CHardwareBufferManagerDX9 final : public CHardwareBufferManager {
public:
	CHardwareBufferManagerDX9();
	
	virtual IHardwareBuffer* CreateVertexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) override;
	virtual IHardwareBuffer* CreateIndexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) override;
	virtual void DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) override;
	virtual void DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) override;

private:
	CPool m_HardwareBufferPool;
};