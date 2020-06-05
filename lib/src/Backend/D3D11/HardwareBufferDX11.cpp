#include "HardwareBufferDX11.h"

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

bool CVertexLayoutDX11::Build(const byte *pShaderByteCode, dword nByteCodeLength, const VertexElement *pArrElem, dword nElemCount)
{
    D3D11_INPUT_ELEMENT_DESC *pElements11 = NewObjectArray<D3D11_INPUT_ELEMENT_DESC>(nElemCount);
    for (dword i = 0; i < nElemCount; ++i)
    {
        const auto &Elem = pArrElem[i];
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
    HRESULT hr = g_pDevice11->CreateInputLayout(pElements11, nElemCount, pShaderByteCode, nByteCodeLength, &m_pInputLayout);
    DeleteObjectArray(pElements11, nElemCount);
    return SUCCEEDED(hr);
}

#define BEGIN_VERTEX_ELEM_DECL(n) { \
    VertexElement Elems[n]; 

#define END_VERTEX_ELEM_DECL(Predefined, n) \
	CVertexLayoutDX11 *pVertexLayout = NEW_TYPE(CVertexLayoutDX11); \
	if (!pVertexLayout->Build(Elems, n)) \
		return false; \
	m_arrPredefinedVertexLayout[(dword)(Predefined)] = pVertexLayout; \
	}

#define ADD_VERTEX_ELEM(Index, Usage, Offset, Type, Semantic, ElemIndex) \
    Elems[Index].m_VertexUsage = Usage; \
    Elems[Index].m_nOffset = Offset; \
    Elems[Index].m_VertexType = Type; \
    Elems[Index].m_Semantic = Semantic; \
    Elems[Index].m_nIndex = ElemIndex;

CHardwareBufferManagerDX11::CHardwareBufferManagerDX11()
{
	m_HardwareBufferPool.Initialize(sizeof(CVertexBufferDX11));
}

CHardwareBufferManagerDX11::~CHardwareBufferManagerDX11()
{
	for (auto &PredefinedLayout : m_arrPredefinedVertexLayout)
	{
		CVertexBufferDX11 *pVertexLayoutDx11 = (CVertexBufferDX11*)PredefinedLayout;
		DELETE_TYPE(pVertexLayoutDx11, CVertexBufferDX11);
	}
	m_arrPredefinedVertexLayout.Clear();

	VertexLayoutMap::_MyIterType Iter = m_VertexLayoutMap.CreateIterator();
	for (; Iter; ++Iter)
	{
		CVertexBufferDX11 *pVertexLayoutDX11 = (CVertexBufferDX11*)Iter.Value();
		DELETE_TYPE(pVertexLayoutDX11, CVertexBufferDX11);
	}
	m_VertexLayoutMap.Clear();
}

bool CHardwareBufferManagerDX11::Initialize()
{
    m_arrPredefinedVertexLayout.Reserve((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);
	m_arrPredefinedVertexLayout.SetNum((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);

	BEGIN_VERTEX_ELEM_DECL(1)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_P, 1)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PT, 2)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 5, EVertexType::EVertexType_Color, EVertexSemantic::EVertexSemantic_Color, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PTC, 3)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PN, 2)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Color, EVertexSemantic::EVertexSemantic_Color, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNC, 3)

	BEGIN_VERTEX_ELEM_DECL(3)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNT, 3)

	BEGIN_VERTEX_ELEM_DECL(4)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT, 4)

	BEGIN_VERTEX_ELEM_DECL(5)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex_Skinning, 0, EVertexType::EVertexType_UByte4, EVertexSemantic::EVertexSemantic_BlendIndices, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex_Skinning, sizeof(byte) * 4, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_BlendWeight, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNT_IDX_WEIGHT, 5)

	BEGIN_VERTEX_ELEM_DECL(6)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex_Skinning, 0, EVertexType::EVertexType_UByte4, EVertexSemantic::EVertexSemantic_BlendIndices, 0)
	ADD_VERTEX_ELEM(5, EVertexUsage::EVertexUsage_PerVertex_Skinning, sizeof(byte) * 4, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_BlendWeight, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT_IDX_WEIGHT, 6)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float4, EVertexSemantic::EVertexSemantic_PositionT, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 4, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PTransformT, 2)

	BEGIN_VERTEX_ELEM_DECL(5)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 3, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Normal, 0)
	ADD_VERTEX_ELEM(2, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 6, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Tangent, 0)
	ADD_VERTEX_ELEM(3, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 9, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	ADD_VERTEX_ELEM(4, EVertexUsage::EVertexUsage_PerVertex, sizeof(float) * 11, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 1)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PNTANT1T2, 5)

	BEGIN_VERTEX_ELEM_DECL(2)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Short2, EVertexSemantic::EVertexSemantic_Position, 0)
	ADD_VERTEX_ELEM(1, EVertexUsage::EVertexUsage_PerVertex, sizeof(short) * 2, EVertexType::EVertexType_Float2, EVertexSemantic::EVertexSemantic_Texcoord, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_PSHORT2T, 2)

	return true;
}