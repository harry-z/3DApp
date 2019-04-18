#include "Memory.h"
#include "FreelistAlloc.h"
#include "Pool.h"

#define BLOCK_SIZE 8192

CPool::~CPool() {
	Uninitialize();
}

bool CPool::Initialize(dword nItemSize)
{
	dword nAlignedSize = ALIGN_SIZE(nItemSize);
	assert(nAlignedSize < BLOCK_SIZE);
	m_nItemSize = nAlignedSize;
	m_nItemPerBlock = BLOCK_SIZE / m_nItemSize; //nItemsPerBlock;
	Block *pBlock = CreateBlock(m_nItemSize, m_nItemPerBlock);
	return pBlock != nullptr;
}

void CPool::Uninitialize() {
	Linklist<Block>::_NodeType *pTemp = m_lstBlocks.m_pRoot;
	while (pTemp) {
		Block *pBlock = pTemp->m_pOwner;
		pTemp = pTemp->m_pNext;
		DestroyBlock(pBlock);
	}
	m_lstBlocks.m_pRoot = nullptr;
}

void* CPool::Allocate()
{
	Linklist<Block>::_NodeType *pNode = m_lstBlocks.m_pRoot;
	Block *pBlock;
	while (pNode != nullptr) {
		pBlock = pNode->m_pOwner;
		if (pBlock->m_nIndex > 0) // 如果Block中包含有FreeObj则返回一个Obj的指针
			return pBlock->m_pp[--pBlock->m_nIndex];
		pNode = pNode->m_pNext;
	}
	// 未找到有FreeObj的Block，重新创建一个
	pBlock = CreateBlock(m_nItemSize, m_nItemPerBlock);
	if (pBlock == nullptr)
		return nullptr;
	return pBlock->m_pp[--pBlock->m_nIndex];
}

void CPool::Free(void *pObj)
{
	assert(pObj != nullptr);
	byte *pbyte = (byte*)pObj;
	dword nBlockSize = m_nItemSize * m_nItemPerBlock;
	Linklist<Block>::_NodeType *pNode = m_lstBlocks.m_pRoot;
	Block *pBlock;
	while (pNode) {
		pBlock = pNode->m_pOwner;
		if (pbyte >= pBlock->m_pObjs && pbyte < (pBlock->m_pObjs + nBlockSize)) {
			pBlock->m_pp[pBlock->m_nIndex++] = pObj;
			return;
		}
		pNode = pNode->m_pNext;
	}
}

void CPool::Clear() {
	Linklist<Block>::_NodeType *pNode = m_lstBlocks.m_pRoot;
	Block *pBlock;
	while (pNode) {
		pBlock = pNode->m_pOwner;
		for (dword i = 0; i < m_nItemPerBlock; ++i)
			pBlock->m_pp[m_nItemPerBlock - i - 1] = pBlock->m_pObjs + m_nItemSize * i;
		pBlock->m_nIndex = m_nItemPerBlock;
		pNode = pNode->m_pNext;
	}
}

CPool::Block* CPool::CreateBlock(dword nItemSize, dword nItemPerBlock) {
	dword nObjSize = nItemSize * nItemPerBlock;
	dword nBlockSize = sizeof(Block) + nObjSize + sizeof(void*) * nItemPerBlock;
	byte *p = (byte*)MEMALLOC(nBlockSize);
	assert(p != nullptr);

	// 内存布局
	// [Objs][每个Obj的指针][Block信息]
	Block *pBlock = (Block *)(p + nObjSize + sizeof(void*) * nItemPerBlock);
	memset(pBlock, 0, sizeof(Block));
	pBlock->m_node.m_pOwner = pBlock;
	pBlock->m_pObjs = p;
	pBlock->m_pp = (void **)(p + nObjSize);

	// 初始化每个Obj的指针，反向保存
	for (dword i = 0; i < nItemPerBlock; ++i)
		pBlock->m_pp[nItemPerBlock - i - 1] = pBlock->m_pObjs + i * nItemSize;
	// 记录Block中有多少FreeObj
	pBlock->m_nIndex = nItemPerBlock;
	m_lstBlocks.PushFront(&pBlock->m_node);
	++m_nBlockCount;
	return pBlock;
}

void CPool::DestroyBlock(Block *pBlock) {
	assert(pBlock != nullptr);
	pBlock->~Block();
	MEMFREE(pBlock->m_pObjs);
	--m_nBlockCount;
}