#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

class IHardwareBuffer {
public:
	virtual bool UpdateData(const byte *pData, dword nSize) = 0;

	EHardwareBufferType Type() const { return m_Type; }
	dword Stride() const { return m_nStride; }
	dword LockedLength() const { return m_nLockedLength; }

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

	IVertexLayout* GetPredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout);
	virtual IVertexLayout* CreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem) = 0;
	IVertexLayout* GetVertexLayout(const String &szName);

protected:
	CArray<IVertexLayout*> m_arrPredefinedVertexLayout;
	using VertexLayoutMap = CMap<IdString, IVertexLayout*>;
	VertexLayoutMap m_VertexLayoutMap;
};