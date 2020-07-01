#include "HardwareBufferDX11.h"
#include "RenderBackendDX11.h"

CHardwareBufferDX11::~CHardwareBufferDX11()
{
    if (m_pSysmemBuffer != nullptr)
		MEMFREE(m_pSysmemBuffer);
	SAFE_RELEASE(m_pBuffer);
}

bool CHardwareBufferDX11::UpdateData(const byte *pData, dword nSize)
{
    assert(pData != nullptr && nSize > 0);
	if (nSize > m_nLockedLength)
		return false;
	return CopyData(pData, nSize);
}

bool CHardwareBufferDX11::CopyData(const byte *pData, dword nSize)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = g_pDeviceContext11->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(MappedResource.pData, pData, nSize);
        g_pDeviceContext11->Unmap(m_pBuffer, 0);
        if (m_pSysmemBuffer != nullptr)
            memcpy(m_pSysmemBuffer, pData, nSize);
        m_nLockedLength = nSize;
        return true;
    }
    else
        return false;
}

CVertexLayoutDX11::~CVertexLayoutDX11()
{
    SAFE_RELEASE(m_pInputLayout);
}

LPCSTR MappingSemanticName(EVertexSemantic Semantic)
{
    switch (Semantic)
    {
        case EVertexSemantic::EVertexSemantic_Position:
            return "POSITION";
        case EVertexSemantic::EVertexSemantic_Normal:
            return "NORMAL";
        case EVertexSemantic::EVertexSemantic_Color:
            return "COLOR";
        case EVertexSemantic::EVertexSemantic_Tangent:
            return "TANGENT";
        case EVertexSemantic::EVertexSemantic_Binormal:
            return "BINORMAL";
        case EVertexSemantic::EVertexSemantic_Texcoord:
            return "TEXCOORD";
        case EVertexSemantic::EVertexSemantic_PositionT:
            return "POSITIONT";
        case EVertexSemantic::EVertexSemantic_BlendWeight:
            return "BLENDWEIGHT";
        case EVertexSemantic::EVertexSemantic_BlendIndices:
            return "BLENDINDICES";
        default:
            return "";
    }
}

DXGI_FORMAT MappingTypeFormat(EVertexType Type)
{
    switch (Type)
    {
        case EVertexType::EVertexType_Float:
            return DXGI_FORMAT_R32_FLOAT;
        case EVertexType::EVertexType_Float2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case EVertexType::EVertexType_Float3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case EVertexType::EVertexType_Float4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case EVertexType::EVertexType_Color:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case EVertexType::EVertexType_Short2:
            return DXGI_FORMAT_R16G16_SINT;
        case EVertexType::EVertexType_Short4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case EVertexType::EVertexType_UByte4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

bool CVertexLayoutDX11::Build(const byte *pShaderByteCode, dword nByteCodeLength)
{
    D3D11_INPUT_ELEMENT_DESC *pElements11 = NewObjectArray<D3D11_INPUT_ELEMENT_DESC>(m_nElemCount);
    for (dword i = 0; i < m_nElemCount; ++i)
    {
        const auto &Elem = m_pArrElem[i];
        auto &D3D11Elem = pElements11[i];
        MappingSemanticName(Elem.m_Semantic);
        D3D11Elem.SemanticName = MappingSemanticName(Elem.m_Semantic);
        D3D11Elem.SemanticIndex = Elem.m_nIndex;
        D3D11Elem.Format = MappingTypeFormat(Elem.m_VertexType);
        D3D11Elem.InputSlot = (UINT)Elem.m_VertexUsage;
        D3D11Elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        D3D11Elem.AlignedByteOffset = Elem.m_nOffset;
        D3D11Elem.InstanceDataStepRate = 0;
    }
    HRESULT hr = g_pDevice11->CreateInputLayout(pElements11, m_nElemCount, pShaderByteCode, nByteCodeLength, &m_pInputLayout);
    DeleteObjectArray(pElements11, m_nElemCount);
    return SUCCEEDED(hr);
}

#define BEGIN_VERTEX_ELEM_DECL(n) { \
	CVertexLayoutDX11 *pVertexLayout = NEW_TYPE(CVertexLayoutDX11); \
	pVertexLayout->m_pArrElem = NewObjectArray<VertexElement>(n); \
	pVertexLayout->m_nElemCount = n;

#define END_VERTEX_ELEM_DECL(Predefined) \
	m_ppPredefinedVertexLayout[(dword)(Predefined)] = pVertexLayout; \
	}

#define ADD_VERTEX_ELEM(Index, Usage, Offset, Type, Semantic, ElemIndex) \
	auto &Elem##Index = pVertexLayout->m_pArrElem[Index]; \
    Elem##Index.m_VertexUsage = Usage; \
    Elem##Index.m_nOffset = Offset; \
    Elem##Index.m_VertexType = Type; \
    Elem##Index.m_Semantic = Semantic; \
    Elem##Index.m_nIndex = ElemIndex;

CHardwareBufferManagerDX11::CHardwareBufferManagerDX11()
{
	m_HardwareBufferPool.Initialize(sizeof(CVertexBufferDX11));
}

CHardwareBufferManagerDX11::~CHardwareBufferManagerDX11()
{
	for (dword i = 0; i < (dword)EPredefinedVertexLayout::EPredefinedLayout_Count; ++i)
	{
		CVertexLayoutDX11 *pVertexLayoutDx11 = static_cast<CVertexLayoutDX11*>(m_ppPredefinedVertexLayout[i]);
		DELETE_TYPE(pVertexLayoutDx11, CVertexLayoutDX11);
	}
	DeleteObjectArray(m_ppPredefinedVertexLayout, (dword)EPredefinedVertexLayout::EPredefinedLayout_Count);

	VertexLayoutMap::_MyIterType Iter = m_VertexLayoutMap.CreateIterator();
	for (; Iter; ++Iter)
	{
		CVertexLayoutDX11 *pVertexLayoutDX11 = (CVertexLayoutDX11*)Iter.Value();
		DELETE_TYPE(pVertexLayoutDX11, CVertexLayoutDX11);
	}
	m_VertexLayoutMap.Clear();
}

bool CHardwareBufferManagerDX11::Initialize()
{
    // m_arrPredefinedVertexLayout.Reserve((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);
	// m_arrPredefinedVertexLayout.SetNum((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);
	m_ppPredefinedVertexLayout = NewObjectArray<IVertexLayout*>((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);

	BEGIN_VERTEX_ELEM_DECL(1)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_P)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PT)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 5, EVertexType::EVertexType_Color, EVertexSemantic::EVertexSemantic_Color, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PTC)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PN)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Color, EVertexSemantic::EVertexSemantic_Color, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNC)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNT)

	BEGIN_VERTEX_ELEM_DECL(4)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT)

	BEGIN_VERTEX_ELEM_DECL(5)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex_Skinning, 0, EVertexType::EVertexType_UByte4, EVertexSemantic::EVertexSemantic_BlendIndices, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex_Skinning, sizeof(byte) * 4, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_BlendWeight, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNT_IDX_WEIGHT)

	BEGIN_VERTEX_ELEM_DECL(6)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex_Skinning, 0, EVertexType::EVertexType_UByte4, EVertexSemantic::EVertexSemantic_BlendIndices, 0)
	ADD_VERTEX_ELEM(5, EVertexUsage::EVertexUsage_PerVertex_Skinning, sizeof(byte) * 4, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_BlendWeight, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT_IDX_WEIGHT)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_PositionT, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 4, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PTransformT)

	BEGIN_VERTEX_ELEM_DECL(5)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 11, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 1)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT1T2)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Short2, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(short) * 2, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PSHORT2T)

	return true;
}

IHardwareBuffer* CHardwareBufferManagerDX11::CreateVertexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData)
{
	return nullptr;
}

IHardwareBuffer* CHardwareBufferManagerDX11::CreateIndexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData)
{
	return nullptr;
}

void CHardwareBufferManagerDX11::DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer)
{

}

void CHardwareBufferManagerDX11::DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer)
{
	
}

IVertexLayout* CHardwareBufferManagerDX11::GetOrCreatePredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout, const byte *pShaderByteCode, dword nShaderByteCodeLen)
{
	dword nIndex = (dword)PredefinedLayout;
	IVertexLayout *pVertexLayout = (nIndex < (dword)EPredefinedVertexLayout::EPredefinedLayout_Count) ? m_ppPredefinedVertexLayout[nIndex] : nullptr;
	if (pVertexLayout != nullptr)	
	{
		CVertexLayoutDX11 *pVertexLayoutDX11 = static_cast<CVertexLayoutDX11*>(pVertexLayout);
		if (pVertexLayoutDX11->m_pInputLayout == nullptr)
		{
			assert(pVertexLayoutDX11->Build(pShaderByteCode, nShaderByteCodeLen));
		}
		return pVertexLayout;
	}
	else
		return nullptr;
}

IVertexLayout* CHardwareBufferManagerDX11::GetOrCreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem, const byte *pShaderByteCode, dword nShaderByteCodeLen)
{
	IdString idStr(szName);
	VertexLayoutMap::_MyIterType Iter = m_VertexLayoutMap.Find(idStr);
	if (Iter)
		return Iter.Value();
	CVertexLayoutDX11 *pVertexLayoutDX11 = NEW_TYPE(CVertexLayoutDX11);
	pVertexLayoutDX11->m_pArrElem = NewObjectArray<VertexElement>(arrElem.Num());
	memcpy(pVertexLayoutDX11->m_pArrElem, arrElem.Data(), sizeof(VertexElement) * arrElem.Num());
	if (pVertexLayoutDX11->Build(pShaderByteCode, nShaderByteCodeLen))
	{
		m_VertexLayoutMap.Insert(idStr, pVertexLayoutDX11);
		return pVertexLayoutDX11;
	}
	else
		return nullptr;
}

void CRenderBackendDX11::SetVertexLayout(IVertexLayout *pLayout)
{
	CVertexLayoutDX11 *pVertexLayoutDX11 = (CVertexLayoutDX11 *)pLayout;
	m_pD3DContext11->IASetInputLayout(pVertexLayoutDX11->m_pInputLayout);
}

void CRenderBackendDX11::SetVertexBuffers(const CArray<IHardwareBuffer*> &arrVertexBuffer)
{
	dword nNumBuffers = arrVertexBuffer.Num();
	ID3D11Buffer **pVBs = (ID3D11Buffer **)g_pFrameAllocator->Allocate(sizeof(ID3D11Buffer *) * nNumBuffers);
	dword *pStrides = (dword *)g_pFrameAllocator->Allocate(sizeof(dword) * nNumBuffers);
	dword *pOffsets = (dword *)g_pFrameAllocator->Allocate(sizeof(dword) * nNumBuffers);
	for (dword i = 0; i < nNumBuffers; ++i)
	{
		CVertexBufferDX11 *pVBDX11 = (CVertexBufferDX11 *)arrVertexBuffer[i];
		pVBs[i] = pVBDX11->m_pBuffer;
		pStrides[i] = pVBDX11->Stride();
		pOffsets[i] = 0;
	}
	m_pD3DContext11->IASetVertexBuffers(0, nNumBuffers, pVBs, pStrides, pOffsets);
}

void CRenderBackendDX11::SetIndexBuffer(IHardwareBuffer *pIndexBuffer)
{
	CIndexBufferDX11 *pIBDX11 = (CIndexBufferDX11 *)pIndexBuffer;
	m_pD3DContext11->IASetIndexBuffer(pIBDX11->m_pBuffer, pIBDX11->Stride() == sizeof(dword) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
}