#include "InputListener.h"

void CInputListener::MouseButtonDown(dword x, dword y, dword nButton) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnMouseButtonDown(x, y, nButton);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::MouseButtonUp(dword x, dword y, dword nButton) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnMouseButtonUp(x, y, nButton);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::MouseMove(dword x, dword y) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnMouseMove(x, y);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::MouseWheel(int nDelta) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnMouseWheel(nDelta);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::KeyDown(EKeyCode eKeyCode) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnKeyDown(eKeyCode);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::KeyUp(EKeyCode eKeyCode) {
	if (!m_lstHandlers.Empty()) {
		Linklist<IInputHandler>::_NodeType *pTemp = m_lstHandlers.m_pRoot;
		while (pTemp != nullptr) {
			pTemp->m_pOwner->OnKeyUp(eKeyCode);
			pTemp = pTemp->m_pNext;
		}
	}
}
void CInputListener::AddInputHandler(IInputHandler *pHandler) {
	assert(pHandler != nullptr);
	m_lstHandlers.PushBack(&pHandler->m_node);
}
void CInputListener::RemoveInputHandler(IInputHandler *pHandler) {
	assert(pHandler != nullptr);
	m_lstHandlers.Remove(&pHandler->m_node);
}