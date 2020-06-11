#pragma once
#include "..\..\RendererStableHeader.h"
#include "HardwareBuffer.h"

class CHardwareBufferDX11 : public IHardwareBuffer {
public:
    CHardwareBufferDX11(dword nStride, dword nLength) 
		: IHardwareBuffer(nStride, nLength) {}
	virtual ~CHardwareBufferDX11();

	virtual bool UpdateData(const byte *pData, dword nSize) override;
	bool CopyData(const byte *pData, dword nSize);

    ID3D11Buffer *m_pBuffer = nullptr;
	byte *m_pSysmemBuffer = nullptr;
};

class CVertexBufferDX11 final : public CHardwareBufferDX11 {
public:
    CVertexBufferDX11(dword nStride, dword nLength)
        : CHardwareBufferDX11(nStride, nLength)
    {
        m_Type = EHardwareBufferType::EHardwareBuffer_Vertex;
    }
};

class CIndexBufferDX11 final : public CHardwareBufferDX11 {
public:
	CIndexBufferDX11(dword nStride, dword nLength)
		: CHardwareBufferDX11(nStride, nLength) {
		m_Type = EHardwareBufferType::EHardwareBuffer_Index;
	}
};

class CVertexLayoutDX11 final : public IVertexLayout
{
public:
	virtual ~CVertexLayoutDX11();
	bool Build(const byte *pShaderByteCode, dword nByteCodeLength);

	ID3D11InputLayout *m_pInputLayout = nullptr;

	VertexElement *m_pArrElem;
	dword m_nElemCount;
};

class CHardwareBufferManagerDX11 final : public CHardwareBufferManager {
public:
	CHardwareBufferManagerDX11();
	virtual ~CHardwareBufferManagerDX11();

	virtual bool Initialize() override;
	
	virtual IHardwareBuffer* CreateVertexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) override;
	virtual IHardwareBuffer* CreateIndexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) override;
	virtual void DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) override;
	virtual void DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) override;

	virtual IVertexLayout* GetOrCreatePredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout, const byte *pShaderByteCode, dword nShaderByteCodeLen) override;
	virtual IVertexLayout* GetOrCreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem, const byte *pShaderByteCode, dword nShaderByteCodeLen) override;

	DXGI_FORMAT GetIndexFormat(dword nStride) const;

private:
	CPool m_HardwareBufferPool;
};