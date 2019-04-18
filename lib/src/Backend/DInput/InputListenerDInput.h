#pragma once

#include "..\..\RendererStableHeader.h"
#include "InputListener.h"
#include "Display.h"

class CInputListenerDInput : public CInputListener {
public:
	CInputListenerDInput();
	virtual ~CInputListenerDInput();

	virtual bool Initialize(IDisplay *pDisplay) override;
	virtual void Capture() override;

private:
	void Shutdown();

	struct Keyboard {
		void Initialize(IDirectInput8 *pDInput8, IDisplay *pDisplay);
		void Capture();
		void Shutdown();
		LPDIRECTINPUTDEVICE8 m_pKeyboard;
		CInputListenerDInput *m_pSystem;
		bool m_bOk;
	};

	struct Mouse : public IDisplay::IObserver {
		void Initialize(IDirectInput8 *pDInput8, IDisplay *pDisplay);
		void Capture();
		void Shutdown();
		virtual void OnDisplayResized(dword w, dword h) override {
			m_ClientWidth = w;
			m_ClientHeight = h;
		}
		void DoClick(DWORD dwData, dword button);
		LPDIRECTINPUTDEVICE8 m_pMouse;
		CInputListenerDInput *m_pSystem;
		dword m_ClientWidth, m_ClientHeight;
		dword m_x, m_y;
		HWND m_hWnd;
		bool m_bOk;
	};

private:
	IDirectInput8 *m_pDInput8;
	Keyboard m_Keyboard;
	Mouse m_Mouse;
};
