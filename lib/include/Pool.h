#pragma once

#include "PreHeader.h"
#include "LinkList.h"
// #include "Threading.h"


class CPool {
public:
	CPool() : m_nItemSize(0), m_nItemPerBlock(0), m_nBlockCount(0) {}
	~CPool();

	bool Initialize(dword nItemSize);
	void Uninitialize();

	void* Allocate();
	void Free(void *pObj);
	void Clear();

	inline void* Allocate_mt() { std::lock_guard<std::mutex> slock(m_lock); return Allocate(); }
	inline void Free_mt(void *pObj) { std::lock_guard<std::mutex> slock(m_lock); Free(pObj); }
	inline void Clear_mt() { std::lock_guard<std::mutex> slock(m_lock); Clear(); }

private:
	struct Block {
		LinklistNode<Block> m_node;
		byte *m_pObjs;
		void **m_pp;
		dword m_nIndex;
		Block() : m_pObjs(nullptr), m_pp(nullptr), m_nIndex(0) {
			m_node.m_pOwner = this;
		}
		~Block() {}
	};
	Linklist<Block> m_lstBlocks;

	Block* CreateBlock(dword nItemSize, dword nItemPerBlock);
	void DestroyBlock(Block *pBlock);

private:
	dword m_nItemSize;
	dword m_nItemPerBlock;
	dword m_nBlockCount;
	std::mutex m_lock;
};