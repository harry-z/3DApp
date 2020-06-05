#pragma once
#include "..\..\RendererStableHeader.h"
#include "HardwareBuffer.h"
#include "GPUResource.h"

class CVertexBufferDX9 final : public IHardwareBuffer, public IGpuResource {
public:
	friend class CHardwareBufferManagerD3D9;
	CVertexBufferDX9(dword nStride, dword nLength) 
		: IHardwareBuffer(nStride, nLength) {
		m_Type = EHardwareBufferType::EHardwareBuffer_Vertex;
	}
	virtual ~CVertexBufferDX9();

	virtual bool UpdateData(const byte *pData, dword nSize) override;
	virtual void OnDeviceLost() override { SAFE_RELEASE(m_pVertexBuffer); }
	virtual void OnDeviceReset(float, float) override;

	bool CopyData(const byte *pData, dword nSize);

	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer = nullptr;
	byte *m_pSysmemBuffer = nullptr;
};

class CIndexBufferDX9 final : public IHardwareBuffer, public IGpuResource {
public:
	friend class CHardwareBufferManagerD3D9;
	CIndexBufferDX9(dword nStride, dword nLength)
		: IHardwareBuffer(nStride, nLength) {
		m_Type = EHardwareBufferType::EHardwareBuffer_Index;
	}
	virtual ~CIndexBufferDX9();

	virtual bool UpdateData(const byte *pData, dword nSize) override;
	virtual void OnDeviceLost() override { SAFE_RELEASE(m_pIndexBuffer); }
	virtual void OnDeviceReset(float, float) override;

	bool CopyData(const byte *pData, dword nSize);

	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer = nullptr;
	byte *m_pSysmemBuffer = nullptr;
};

class CVertexLayoutDX9 final : public IVertexLayout
{
public:
	virtual ~CVertexLayoutDX9();
	bool Build(const VertexElement *pArrElem, dword nElemCount);

	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDecl = nullptr;
};

class CHardwareBufferManagerDX9 final : public CHardwareBufferManager {
public:
	CHardwareBufferManagerDX9();
	virtual ~CHardwareBufferManagerDX9();

	virtual bool Initialize() override;
	
	virtual IHardwareBuffer* CreateVertexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) override;
	virtual IHardwareBuffer* CreateIndexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) override;
	virtual void DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) override;
	virtual void DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) override;

	virtual IVertexLayout* GetOrCreatePredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout, const byte *pShaderByteCode, dword nShaderByteCodeLen) override;
	virtual IVertexLayout* CreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem) override;

	D3DFORMAT GetIndexFormat(dword nStride) const;

private:
	CPool m_HardwareBufferPool;
};