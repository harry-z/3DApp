#include "HardwareBufferDX9.h"
#include "RenderBackendDX9.h"

CVertexBufferDX9::~CVertexBufferDX9() {
	if (m_pSysmemBuffer != nullptr)
		MEMFREE(m_pSysmemBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
}

bool CVertexBufferDX9::UpdateData(const byte *pData, dword nSize) {
	assert(m_pSysmemBuffer != nullptr);
	if (nSize != m_nLockedLength)
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
	assert(m_pSysmemBuffer != nullptr);
	if (nSize != m_nLockedLength)
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
		return true;
	}
	else
		return false;
}

CHardwareBufferManagerDX9::CHardwareBufferManagerDX9()
	: CHardwareBufferManager() {
	m_HardwareBufferPool.Initialize(sizeof(CVertexBufferDX9));
}

IHardwareBuffer* CHardwareBufferManagerDX9::CreateVertexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) {
	assert(!bDynamic && pData != nullptr);

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
		CVertexBufferDX9 *pVertexBuffer = new (m_HardwareBufferPool.Allocate()) CVertexBufferDX9(nLength, nCount);
		pVertexBuffer->m_pVertexBuffer = pVertexBuffer9;
		pVertexBuffer->m_pSysmemBuffer = pSysmem;
		pVertexBuffer->CopyData(pData, nLength);
		if (bDynamic) {
			((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.PushBack(&pVertexBuffer->m_node);
		}
		return pVertexBuffer;
	}
	return nullptr;
}

IHardwareBuffer* CHardwareBufferManagerDX9::CreateIndexBuffer(bool bDynamic, const byte *pData, dword nLength, dword nCount) {
	assert(!bDynamic && pData != nullptr);

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
	HRESULT hr = g_pDevice9->CreateIndexBuffer(nLength, dwUsage, D3DFMT_INDEX16, ePool, &pIndexBuffer9, 0);
	if (SUCCEEDED(hr)) {
		CIndexBufferDX9 *pIndexBuffer = new (m_HardwareBufferPool.Allocate()) CIndexBufferDX9(nLength, nCount);
		pIndexBuffer->m_pIndexBuffer = pIndexBuffer9;
		pIndexBuffer->m_pSysmemBuffer = pSysmem;
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