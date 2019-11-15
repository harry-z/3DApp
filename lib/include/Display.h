#pragma once


#include "Prereq.h"

class IDisplay {
public:
	virtual ~IDisplay() {}

	virtual bool Initialize() = 0;
	virtual void Move(int x, int y) { m_nClientLeft = x; m_nClientTop = y; }
	virtual void Resize(dword w, dword h) { m_nClientWidth = w; m_nClientHeight = h; }
	inline void GetPosition(int &x, int &y) const { x = m_nLeft; y = m_nTop; }
	inline void GetDimension(dword &w, dword &h) const { w = m_nWidth; h = m_nHeight; }
	inline void GetClientPosition(int &x, int &y) const { x = m_nClientLeft; y = m_nClientTop; }
	inline void GetClientDimension(dword &w, dword &h) const { w = m_nClientWidth; h = m_nClientHeight; }
	virtual bool MessagePump() = 0;

	class IObserver {
	public:
		IObserver() { m_node.m_pOwner = this; }
		virtual ~IObserver() {}
		virtual void OnDisplayResized(dword w, dword h) = 0;
		LinklistNode<IObserver> m_node;
	};
	inline void AddDisplayObserver(IObserver *pObserver) {
		assert(pObserver != nullptr);
		m_lstObserver.PushBack(&pObserver->m_node);
	}
	inline void RemoveDisplayObserver(IObserver *pObserver) {
		assert(pObserver != nullptr);
		m_lstObserver.Remove(&pObserver->m_node);
	}
	virtual void OnDisplayResized(dword w, dword h) {
		m_nWidth = w; m_nHeight = h;
		if (!m_lstObserver.Empty()) {
			Linklist<IObserver>::_NodeType *pTemp = m_lstObserver.m_pRoot;
			while (pTemp != nullptr) {
				pTemp->m_pOwner->OnDisplayResized(w, h);
				pTemp = pTemp->m_pNext;
			}	
		}
	}

private:
	Linklist<IObserver> m_lstObserver;

protected:
	int m_nLeft, m_nTop;
	dword m_nWidth, m_nHeight;
	int m_nClientLeft, m_nClientTop;
	dword m_nClientWidth, m_nClientHeight;
};