#pragma once

template <class T>
struct LinklistNode {
	typedef T _Myt;
	T *m_pOwner;
	LinklistNode *m_pPrev;
	LinklistNode *m_pNext;
	LinklistNode() : m_pOwner(nullptr), m_pPrev(nullptr), m_pNext(nullptr) {}
};

template <class T>
struct Linklist {
	typedef LinklistNode<T> _NodeType;
	_NodeType *m_pRoot;
	_NodeType *m_pTail;

	Linklist() : m_pRoot(nullptr), m_pTail(nullptr) {}
	
	void PushFront(_NodeType *pNode) {
		if (m_pRoot == nullptr)
			m_pRoot = m_pTail = pNode; 
		else { 
			pNode->m_pNext = m_pRoot;
			m_pRoot->m_pPrev = pNode;
			m_pRoot = pNode;
		}
	}
	void PushBack(_NodeType *pNode) {
		if (m_pRoot == nullptr)
			m_pRoot = m_pTail = pNode; 
		else { 
			m_pTail->m_pNext = pNode;
			pNode->m_pPrev = m_pTail;
			m_pTail = pNode;
		}
	}
	void Remove(_NodeType *pNode) {
		if (pNode->m_pNext != nullptr)
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		if (pNode->m_pPrev != nullptr)
			pNode->m_pPrev->m_pNext = pNode->m_pNext;
		if (m_pTail == pNode) 
			m_pTail = pNode->m_pPrev;
		if (m_pRoot == pNode) 
			m_pRoot = pNode->m_pNext;
		pNode->m_pNext = pNode->m_pPrev = nullptr;
	}
	_NodeType* PopFront() {
		if (m_pRoot == nullptr)
			return nullptr;
		_NodeType *pTemp = m_pRoot;
		m_pRoot = pTemp->m_pNext;
		if (m_pRoot != nullptr)
			m_pRoot->m_pPrev = nullptr;
		else 
			m_pTail = nullptr;
		pTemp->m_pPrev = pTemp->m_pNext = nullptr;
		return pTemp;
	}
	_NodeType* PopBack() {
		if (m_pRoot == nullptr)
			return nullptr;
		_NodeType *pTemp = m_pTail;
		m_pTail = pTemp->m_pPrev;
		if (m_pTail != nullptr)
			m_pTail->m_pNext = nullptr;
		else 
			m_pRoot = nullptr;
		pTemp->m_pPrev = pTemp->m_pNext = nullptr;
		return pTemp;
	}
	_NodeType* Front() const {
		return m_pRoot != nullptr ? m_pRoot : nullptr;
	}
	_NodeType* Back() const {
		return m_pTail != nullptr ? m_pTail : nullptr;
	}
	void Clear() {
		m_pRoot = m_pTail = nullptr;
	}
	bool Empty() const {
		return m_pRoot == nullptr;
	}
};