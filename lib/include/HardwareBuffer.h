#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

class IHardwareBuffer {
public:
	virtual bool UpdateData(const byte *pData, dword nSize) = 0;

	inline EHardwareBufferType Type() const { return m_Type; }
	inline dword Stride() const { return m_nStride; }
	inline dword LockedLength() const { return m_nLockedLength; }
	inline dword Count() const { return m_nLockedLength / m_nStride; }

protected:
	IHardwareBuffer(dword nStride, dword nLength)
	: m_nStride(nStride)
	, m_nLockedLength(nLength) {}
	virtual ~IHardwareBuffer() {}

protected:
	EHardwareBufferType m_Type;
	dword m_nStride;
	dword m_nLockedLength;
};

struct VertexElement {
	EVertexUsage m_VertexUsage;
	word m_nOffset;
	EVertexType m_VertexType;
	EVertexSemantic m_Semantic;
	byte m_nIndex;
};

class IVertexLayout {
public:
	virtual ~IVertexLayout() {}
};

class CHardwareBufferManager {
public:
	virtual ~CHardwareBufferManager() {}

	virtual bool Initialize() = 0;

	virtual IHardwareBuffer* CreateVertexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) = 0;
	virtual IHardwareBuffer* CreateIndexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData = nullptr) = 0;
	virtual void DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) = 0;
	virtual void DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) = 0;

	virtual IVertexLayout* GetOrCreatePredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout, const byte *pShaderByteCode, dword nShaderByteCodeLen) = 0;
	virtual IVertexLayout* GetOrCreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem, const byte *pShaderByteCode, dword nShaderByteCodeLen) = 0;
	IVertexLayout* GetVertexLayout(const String &szName);

protected:
	// CArray<IVertexLayout*> m_arrPredefinedVertexLayout;
	IVertexLayout **m_ppPredefinedVertexLayout;
	using VertexLayoutMap = CMap<IdString, IVertexLayout*>;
	VertexLayoutMap m_VertexLayoutMap;
};