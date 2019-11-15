#include "InputListenerDInput.h"
#include "..\..\Windows\IDisplay_Windows.h"

#define KEYBOARD_BUFFER_SIZE 17
#define MOUSE_BUFFER_SIZE 64

CInputListenerDInput::CInputListenerDInput()
	: m_pDInput8(nullptr) {
	m_Keyboard.m_pSystem = this;
	m_Mouse.m_pSystem = this;
}

CInputListenerDInput::~CInputListenerDInput() {
	Shutdown();
}

bool CInputListenerDInput::Initialize(IDisplay *pDisplay) {
	CLog *pLog = Global::m_pLog;

	if (FAILED(DirectInput8Create(::GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)(&m_pDInput8), nullptr))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not create IDirectInput8 object");
		return false;
	}

	m_Keyboard.Initialize(m_pDInput8, pDisplay);
	m_Mouse.Initialize(m_pDInput8, pDisplay);
	if (!m_Keyboard.m_bOk && !m_Mouse.m_bOk) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not initialize input device");
		return false;
	}
	else
		return true;
}

void CInputListenerDInput::Capture() {
	m_Keyboard.Capture();
	m_Mouse.Capture();
}

void CInputListenerDInput::Shutdown() {
	m_Keyboard.Shutdown();
	m_Mouse.Shutdown();
	SAFE_RELEASE(m_pDInput8);
}

void CInputListenerDInput::Keyboard::Initialize(IDirectInput8 *pDInput8, IDisplay *pDisplay) {
	CLog *pLog = Global::m_pLog;

	m_bOk = false;

	if (FAILED(pDInput8->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not create keyboard input device");
		return;
	}

	IDisplayWindows *pDisplayWindows = (IDisplayWindows *)(pDisplay);
	HRESULT hr = m_pKeyboard->SetCooperativeLevel(pDisplayWindows->GetHWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(hr)) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set cooperative level for keyboard");
		return;
	}

	if (FAILED(m_pKeyboard->SetDataFormat(&c_dfDIKeyboard))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set input data format for keyboard");
		return;
	}

	DIPROPDWORD dipKeyboard;
	dipKeyboard.diph.dwSize = sizeof(DIPROPDWORD);
	dipKeyboard.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipKeyboard.diph.dwObj = 0;
	dipKeyboard.diph.dwHow = DIPH_DEVICE;
	dipKeyboard.dwData = KEYBOARD_BUFFER_SIZE;
	if (FAILED(m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipKeyboard.diph))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set data buffer size for keyboard");
		return;
	}

	hr = m_pKeyboard->Acquire();
	if(FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not acquire keyboard device");
		return;
	}

	m_bOk = true;
}

void CInputListenerDInput::Keyboard::Capture() {
	DIDEVICEOBJECTDATA diBuff[KEYBOARD_BUFFER_SIZE];
	DWORD entries = KEYBOARD_BUFFER_SIZE;
	HRESULT hr;

	hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
	if( hr != DI_OK ) {
		hr = m_pKeyboard->Acquire();
		while( hr == DIERR_INPUTLOST )
			hr = m_pKeyboard->Acquire();
		return;
	}

	for (dword i = 0; i < entries; ++i) {
		EKeyCode eKeyCode = (EKeyCode)diBuff[i].dwOfs;
		if (diBuff[i].dwData & 0x80)
			m_pSystem->KeyDown(eKeyCode);
		else
			m_pSystem->KeyUp(eKeyCode);
	}
}

void CInputListenerDInput::Keyboard::Shutdown() {
	if (m_pKeyboard) {
		m_pKeyboard->Unacquire();
		m_pKeyboard->Release();
	}
}

void CInputListenerDInput::Mouse::Initialize(IDirectInput8 *pDInput8, IDisplay *pDisplay) {
	CLog *pLog = Global::m_pLog;

	m_bOk = false;

	if (FAILED(pDInput8->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not create mouse input device");
		return;
	}

	IDisplayWindows *pDisplayWindows = (IDisplayWindows *)(pDisplay);
	m_hWnd = pDisplayWindows->GetHWnd();
	if (FAILED(m_pMouse->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set cooperative level for mouse");
		return;
	}

	if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse2))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set input data format for mouse");
		return;
	}

	DIPROPDWORD dipMouse;
	dipMouse.diph.dwSize = sizeof(DIPROPDWORD);
	dipMouse.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipMouse.diph.dwObj = 0;
	dipMouse.diph.dwHow = DIPH_DEVICE;
	dipMouse.dwData = MOUSE_BUFFER_SIZE;
	if (FAILED(m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipMouse.diph))) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not set data buffer size for mouse");
		return;
	}

	HRESULT hr = m_pMouse->Acquire();
	if(FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not acquire mouse device");
		return;
	}

	pDisplay->GetClientDimension(m_ClientWidth, m_ClientHeight);
	pDisplay->AddDisplayObserver(this);
	m_bOk = true;
}

void CInputListenerDInput::Mouse::Capture() {
	DIDEVICEOBJECTDATA diBuff[MOUSE_BUFFER_SIZE];
	DWORD entries = MOUSE_BUFFER_SIZE;

	HRESULT hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
	if (hr != DI_OK) {
		hr = m_pMouse->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = m_pMouse->Acquire();
		return;
	}

	bool axisMoved = false;
	for (dword i = 0; i < entries; ++i) {
		switch (diBuff[i].dwOfs) {
		case DIMOFS_BUTTON0:
			DoClick(diBuff[i].dwData, LBUTTON);
			break;
		case DIMOFS_BUTTON1:
			DoClick(diBuff[i].dwData, RBUTTON);
			break;
		case DIMOFS_BUTTON2:
			DoClick(diBuff[i].dwData, MBUTTON);
			break;
		case DIMOFS_X:
			axisMoved = true;
			break;
		case DIMOFS_Y:
			axisMoved = true;
			break;
		case DIMOFS_Z:
			m_pSystem->MouseWheel((int)diBuff[i].dwData / WHEEL_DELTA);
			break;
		}
	}

	if (axisMoved) {
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(m_hWnd, &point);
		if (point.x < 0) point.x = 0;
		if (point.x > (int)m_ClientWidth) point.x = m_ClientWidth;
		if (point.y < 0) point.y = 0;
		if (point.y > (int)m_ClientHeight) point.y = m_ClientHeight;
		m_x = point.x; m_y = point.y;
		m_pSystem->MouseMove(point.x, point.y);
	}
}

void CInputListenerDInput::Mouse::Shutdown() {
	if (m_pMouse) {
		m_pMouse->Unacquire();
		m_pMouse->Release();
	}
}

void CInputListenerDInput::Mouse::DoClick(DWORD dwData, dword button) {
	if (dwData & 0x80)
		m_pSystem->MouseButtonDown(m_x, m_y, button);
	else
		m_pSystem->MouseButtonUp(m_x, m_y, button);
}