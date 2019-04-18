#pragma once

#include "InputHandler.h"

class IDisplay;
class DLL_EXPORT CInputListener {
public:
	virtual ~CInputListener() {}

	virtual bool Initialize(IDisplay *pDisplay) { return true; }

	void MouseButtonDown(dword x, dword y, dword nButton);
	void MouseButtonUp(dword x, dword y, dword nButton);
	void MouseMove(dword x, dword y);
	void MouseWheel(int nDelta);
	void KeyDown(EKeyCode eKeyCode);
	void KeyUp(EKeyCode eKeyCode);

	void AddInputHandler(IInputHandler *pHandler);
	void RemoveInputHandler(IInputHandler *pHandler);

	virtual void Capture() {}

private:
	Linklist<IInputHandler> m_lstHandlers;
};