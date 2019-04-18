#include "Memory.h"
#include "Stack.h"

CStack::CStack()
	: m_nSize(0),
	m_nOffset(0),
	m_pBuffer(nullptr) {
	
}

CStack::~CStack() {
	if (m_pBuffer) {
		MEMFREE(m_pBuffer);
	}
}

bool CStack::Initialize(dword nSize) {
	m_pBuffer = (byte *)MEMALLOC(nSize);
	assert(m_pBuffer != nullptr);
	m_nSize = nSize;
	return m_pBuffer != nullptr;
}

void* CStack::Allocate(dword nBytes) {
	assert(m_pBuffer != nullptr);
	dword nAlignedSize = ALIGN_SIZE(nBytes);
	if (NotEnoughMemory(nAlignedSize))
		return nullptr;
	byte *pTemp = m_pBuffer + m_nOffset;
	m_nOffset += nAlignedSize;
	return pTemp;
}