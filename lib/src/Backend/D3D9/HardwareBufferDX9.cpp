#include "HardwareBufferDX9.h"
#include "RenderBackendDX9.h"

CVertexBufferDX9::~CVertexBufferDX9() {
	if (m_pSysmemBuffer != nullptr)
		MEMFREE(m_pSysmemBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
}

bool CVertexBufferDX9::UpdateData(const byte *pData, dword nSize) {
	assert(pData != nullptr && nSize > 0);
	if (nSize > m_nLockedLength)
		return false;
	return CopyData(pData, nSize);
}

void CVertexBufferDX9::OnDeviceReset(float, float) {
	HRESULT hr = g_pDevice9->CreateVertexBuffer(m_nLockedLength, 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, 0);
    assert(SUCCEEDED(hr) && "Error when recreating vertex buffer during device reset");

	void *p;
	hr = m_pVertexBuffer->Lock(0, m_nLockedLength, &p, D3DLOCK_DISCARD);
    assert(SUCCEEDED(hr) && "Error when locking vertex buffer during device reset");

	memcpy(p, m_pSysmemBuffer, m_nLockedLength);
	m_pVertexBuffer->Unlock();
}

bool CVertexBufferDX9::CopyData(const byte *pData, dword nSize) {
	void *p;
	HRESULT hr = m_pVertexBuffer->Lock(0, nSize, &p, m_pSysmemBuffer ? D3DLOCK_DISCARD : 0);
	if (SUCCEEDED(hr)) {
		memcpy(p, pData, nSize);
		m_pVertexBuffer->Unlock();
		if (m_pSysmemBuffer != nullptr)
			memcpy(m_pSysmemBuffer, pData, nSize);
		m_nLockedLength = nSize;
		return true;
	}
	else
		return false;
}

CIndexBufferDX9::~CIndexBufferDX9() {
	if (m_pSysmemBuffer != nullptr)
		MEMFREE(m_pSysmemBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}

bool CIndexBufferDX9::UpdateData(const byte *pData, dword nSize) {
	assert(pData != nullptr && nSize > 0);
	if (nSize > m_nLockedLength)
		return false;
	return CopyData(pData, nSize);
}

void CIndexBufferDX9::OnDeviceReset(float, float) {
	HRESULT hr = g_pDevice9->CreateIndexBuffer(m_nLockedLength, 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIndexBuffer, 0);
    assert(SUCCEEDED(hr) && "Error when recreating index buffer during device reset");

	void *p;
	hr = m_pIndexBuffer->Lock(0, m_nLockedLength, &p, D3DLOCK_DISCARD);
    assert(SUCCEEDED(hr) && "Error when locking index buffer during device reset");

	memcpy(p, m_pSysmemBuffer, m_nLockedLength);
	m_pIndexBuffer->Unlock();
}

bool CIndexBufferDX9::CopyData(const byte *pData, dword nSize) {
	void *p;
	HRESULT hr = m_pIndexBuffer->Lock(0, nSize, &p, m_pSysmemBuffer ? D3DLOCK_DISCARD : 0);
	if (SUCCEEDED(hr)) {
		memcpy(p, pData, nSize);
		m_pIndexBuffer->Unlock();
		if (m_pSysmemBuffer != nullptr)
			memcpy(m_pSysmemBuffer, pData, nSize);
		m_nLockedLength = nSize;
		return true;
	}
	else
		return false;
}

D3DDECLTYPE MappingDeclType(EVertexType Type)
{
	switch (Type)
	{
		case EVertexType::EVertexType_Float:
			return D3DDECLTYPE_FLOAT1;
		case EVertexType::EVertexType_Float2:
			return D3DDECLTYPE_FLOAT2;		
		case EVertexType::EVertexType_Float3:
			return D3DDECLTYPE_FLOAT3;
		case EVertexType::EVertexType_Float4:
			return D3DDECLTYPE_FLOAT4;
		case EVertexType::EVertexType_Color:
			return D3DDECLTYPE_D3DCOLOR;
		case EVertexType::EVertexType_UByte4:
			return D3DDECLTYPE_UBYTE4;
		case EVertexType::EVertexType_Short2:
			return D3DDECLTYPE_SHORT2;
		case EVertexType::EVertexType_Short4:
			return D3DDECLTYPE_SHORT4;
		default:
			return D3DDECLTYPE_UNUSED;												
	}
}

D3DDECLUSAGE MappingDeclUsage(EVertexSemantic Semantic)
{
	switch (Semantic)
	{
		case EVertexSemantic::EVertexSemantic_Position:
			return D3DDECLUSAGE_POSITION;
		case EVertexSemantic::EVertexSemantic_BlendWeight:
			return D3DDECLUSAGE_BLENDWEIGHT;
		case EVertexSemantic::EVertexSemantic_BlendIndices:
			return D3DDECLUSAGE_BLENDINDICES;
		case EVertexSemantic::EVertexSemantic_Normal:
			return D3DDECLUSAGE_NORMAL;
		case EVertexSemantic::EVertexSemantic_Texcoord:
			return D3DDECLUSAGE_TEXCOORD;
		case EVertexSemantic::EVertexSemantic_Tangent:
			return D3DDECLUSAGE_TANGENT;
		case EVertexSemantic::EVertexSemantic_Binormal:
			return D3DDECLUSAGE_BINORMAL;
		case EVertexSemantic::EVertexSemantic_PositionT:
			return D3DDECLUSAGE_POSITIONT;
		case EVertexSemantic::EVertexSemantic_Color:
			return D3DDECLUSAGE_COLOR;
		default:
			return (D3DDECLUSAGE)0;
	}
}

CVertexLayoutDX9::~CVertexLayoutDX9()
{
	SAFE_RELEASE(m_pVertexDecl);
}

bool CVertexLayoutDX9::Build(const VertexElement *pArrElem, dword nElemCount)
{
	D3DVERTEXELEMENT9 *pElements9 = (D3DVERTEXELEMENT9 *)MEMALLOC(sizeof(D3DVERTEXELEMENT9) * (nElemCount + 1));
	for (dword i = 0; i < nElemCount; ++i) {
		D3DVERTEXELEMENT9 &element9 = pElements9[i];
		const VertexElement &element = pArrElem[i];
		element9.Stream = (WORD)element.m_VertexUsage;
		element9.Offset = element.m_nOffset;
		element9.Type = MappingDeclType(element.m_VertexType);
		element9.Method = D3DDECLMETHOD_DEFAULT;
		element9.Usage = MappingDeclUsage(element.m_Semantic);
		element9.UsageIndex = element.m_nIndex;
	}
	D3DVERTEXELEMENT9 &element9 = pElements9[nElemCount];
	element9.Stream = 0xFF;
	element9.Offset = 0;
	element9.Type = D3DDECLTYPE_UNUSED;
	element9.Method = D3DDECLMETHOD_DEFAULT;
	element9.Usage = 0;
	element9.UsageIndex = 0;

	LPDIRECT3DVERTEXDECLARATION9 pVertexDecl;
	HRESULT hr = g_pDevice9->CreateVertexDeclaration(pElements9, &pVertexDecl);
	MEMFREE(pElements9);
	if (SUCCEEDED(hr))
	{
		m_pVertexDecl = pVertexDecl;
		return true;
	}
	else
		return false;	
}

#define BEGIN_VERTEX_ELEM_DECL(n) { \
    VertexElement Elems[n]; 

#define END_VERTEX_ELEM_DECL(Predefined, n) \
	CVertexLayoutDX9 *pVertexLayout = NEW_TYPE(CVertexLayoutDX9); \
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

CHardwareBufferManagerDX9::CHardwareBufferManagerDX9()
{
	m_HardwareBufferPool.Initialize(sizeof(CVertexBufferDX9));
}

CHardwareBufferManagerDX9::~CHardwareBufferManagerDX9()
{

}

bool CHardwareBufferManagerDX9::Initialize()
{
	m_arrPredefinedVertexLayout.Reserve((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);
	m_arrPredefinedVertexLayout.SetNum((dword)EPredefinedVertexLayout::EPredefinedLayout_Count);

	BEGIN_VERTEX_ELEM_DECL(1)
	ADD_VERTEX_ELEM(0, EVertexUsage::EVertexUsage_PerVertex, 0, EVertexType::EVertexType_Float3, EVertexSemantic::EVertexSemantic_Position, 0)
	END_VERTEX_ELEM_DECL(EPredefinedVertexLayout::EPredefinedLayout_P, 1)

	return true;
}

IHardwareBuffer* CHardwareBufferManagerDX9::CreateVertexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData) {
	if (!bDynamic && pData == nullptr)
		return nullptr;

	DWORD dwUsage;
	D3DPOOL ePool;
	byte *pSysmem = nullptr;
	if (bDynamic) {
		dwUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
		ePool = D3DPOOL_DEFAULT;
		pSysmem = (byte *)MEMALLOC(nLength);
	}
	else {
		dwUsage = 0;
		ePool = D3DPOOL_MANAGED;
	}

	LPDIRECT3DVERTEXBUFFER9 pVertexBuffer9;
	HRESULT hr = g_pDevice9->CreateVertexBuffer(nLength, dwUsage, 0, ePool, &pVertexBuffer9, 0);
	if (SUCCEEDED(hr)) {
		CVertexBufferDX9 *pVertexBuffer = new (m_HardwareBufferPool.Allocate()) CVertexBufferDX9(nStride, nLength);
		pVertexBuffer->m_pVertexBuffer = pVertexBuffer9;
		pVertexBuffer->m_pSysmemBuffer = pSysmem;
		if (pData != nullptr)
			pVertexBuffer->CopyData(pData, nLength);
		if (bDynamic) {
			((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.PushBack(&pVertexBuffer->m_node);
		}
		return pVertexBuffer;
	}
	return nullptr;
}

IHardwareBuffer* CHardwareBufferManagerDX9::CreateIndexBuffer(bool bDynamic, dword nStride, dword nLength, const byte *pData) {
	if (!bDynamic && pData == nullptr)
		return nullptr;

	D3DFORMAT IndexFmt = GetIndexFormat(nStride);
	if (IndexFmt == D3DFMT_UNKNOWN)
		return nullptr;

	DWORD dwUsage;
	D3DPOOL ePool;
	byte *pSysmem = nullptr;
	if (bDynamic) {
		dwUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
		ePool = D3DPOOL_DEFAULT;
		pSysmem = (byte *)MEMALLOC(nLength);
	}
	else {
		dwUsage = 0;
		ePool = D3DPOOL_MANAGED;
	}

	LPDIRECT3DINDEXBUFFER9 pIndexBuffer9;
	HRESULT hr = g_pDevice9->CreateIndexBuffer(nLength, dwUsage, IndexFmt, ePool, &pIndexBuffer9, 0);
	if (SUCCEEDED(hr)) {
		CIndexBufferDX9 *pIndexBuffer = new (m_HardwareBufferPool.Allocate()) CIndexBufferDX9(nStride, nLength);
		pIndexBuffer->m_pIndexBuffer = pIndexBuffer9;
		pIndexBuffer->m_pSysmemBuffer = pSysmem;
		if (pData != nullptr)
			pIndexBuffer->CopyData(pData, nLength);
		if (bDynamic) {
			((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.PushBack(&pIndexBuffer->m_node);
		}
		return pIndexBuffer;
	}
	return nullptr;
}

void CHardwareBufferManagerDX9::DestroyVertexBuffer(IHardwareBuffer *pVertexBuffer) {
	assert(pVertexBuffer->Type() == EHardwareBufferType::EHardwareBuffer_Vertex);
	CVertexBufferDX9 *pVertexBuffer9 = (CVertexBufferDX9 *)pVertexBuffer;
	if (pVertexBuffer9->m_pSysmemBuffer) {
		((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.Remove(&pVertexBuffer9->m_node);
	}
	pVertexBuffer9->~CVertexBufferDX9();
	m_HardwareBufferPool.Free(pVertexBuffer9);
}

void CHardwareBufferManagerDX9::DestroyIndexBuffer(IHardwareBuffer *pIndexBuffer) {
	assert(pIndexBuffer->Type() == EHardwareBufferType::EHardwareBuffer_Index);
	CIndexBufferDX9 *pIndexBuffer9 = (CIndexBufferDX9 *)pIndexBuffer;
	if (pIndexBuffer9->m_pSysmemBuffer) {
		((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.Remove(&pIndexBuffer9->m_node);
	}
	pIndexBuffer9->~CIndexBufferDX9();
	m_HardwareBufferPool.Free(pIndexBuffer9);
}

IVertexLayout* CHardwareBufferManagerDX9::CreateVertexLayout(const String &szName, const CArray<VertexElement> &arrElem)
{
	return nullptr;
}

D3DFORMAT CHardwareBufferManagerDX9::GetIndexFormat(dword nStride) const
{
	if (nStride == sizeof(word))
		return D3DFMT_INDEX16;
	else if (nStride == sizeof(dword))
		return D3DFMT_INDEX32;
	else
		return D3DFMT_UNKNOWN;
}