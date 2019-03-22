#include "Display_Windows.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
	//CInputListener *pInputListener = Global::m_pInputListener;
	switch (nMsg) {
	case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 640;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 480;
			return DefWindowProc(hWnd, nMsg, wParam, lParam);
		}
	/*case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			dword nExtraInfo = 0;
			nExtraInfo |= (wParam & MK_LBUTTON) ? INPUT_EXTRAINFO_LBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_RBUTTON) ? INPUT_EXTRAINFO_RBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_MBUTTON) ? INPUT_EXTRAINFO_MBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_CONTROL) ? INPUT_EXTRAINFO_CTRL_PRESSED : 0;
			nExtraInfo |= (wParam & MK_SHIFT) ? INPUT_EXTRAINFO_SHIFT_PRESSED : 0;
			pInputListener->MouseButtonDown(LOWORD(lParam), HIWORD(lParam), nExtraInfo);
		}
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			dword nExtraInfo = 0;
			nExtraInfo |= (wParam & MK_LBUTTON) ? INPUT_EXTRAINFO_LBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_RBUTTON) ? INPUT_EXTRAINFO_RBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_MBUTTON) ? INPUT_EXTRAINFO_MBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_CONTROL) ? INPUT_EXTRAINFO_CTRL_PRESSED : 0;
			nExtraInfo |= (wParam & MK_SHIFT) ? INPUT_EXTRAINFO_SHIFT_PRESSED : 0;
			pInputListener->MouseButtonUp(LOWORD(lParam), HIWORD(lParam), nExtraInfo);
		}
		return 0;
	case WM_MOUSEMOVE:
		{
			dword nExtraInfo = 0;
			nExtraInfo |= (wParam & MK_LBUTTON) ? INPUT_EXTRAINFO_LBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_RBUTTON) ? INPUT_EXTRAINFO_RBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_MBUTTON) ? INPUT_EXTRAINFO_MBUTTON_PRESSED : 0;
			nExtraInfo |= (wParam & MK_CONTROL) ? INPUT_EXTRAINFO_CTRL_PRESSED : 0;
			nExtraInfo |= (wParam & MK_SHIFT) ? INPUT_EXTRAINFO_SHIFT_PRESSED : 0;
			pInputListener->MouseMove(LOWORD(lParam), HIWORD(lParam), nExtraInfo);
		}
		return 0;
	case WM_MOUSEWHEEL:
		{
			dword nExtraInfo = 0, nFlag = LOWORD(wParam);
			nExtraInfo |= (nFlag & MK_LBUTTON) ? INPUT_EXTRAINFO_LBUTTON_PRESSED : 0;
			nExtraInfo |= (nFlag & MK_RBUTTON) ? INPUT_EXTRAINFO_RBUTTON_PRESSED : 0;
			nExtraInfo |= (nFlag & MK_MBUTTON) ? INPUT_EXTRAINFO_MBUTTON_PRESSED : 0;
			nExtraInfo |= (nFlag & MK_CONTROL) ? INPUT_EXTRAINFO_CTRL_PRESSED : 0;
			nExtraInfo |= (nFlag & MK_SHIFT) ? INPUT_EXTRAINFO_SHIFT_PRESSED : 0;
			pInputListener->MouseWheel(LOWORD(lParam), HIWORD(lParam), GET_WHEEL_DELTA_WPARAM(wParam), nExtraInfo);
		}
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_CHAR:
	case WM_SYSCHAR:
		{
			dword nExtraInfo = 0;
			nExtraInfo |= (nMsg == WM_SYSKEYDOWN || nMsg == WM_SYSCHAR) ? INPUT_EXTRAINFO_IS_SYSKEY : 0;
			nExtraInfo |= (nMsg == WM_SYSCHAR || nMsg == WM_CHAR) ? INPUT_EXTRAINFO_IS_CHARKEY : 0;
			nExtraInfo |= (lParam & 0x40000000U) ? INPUT_EXTRAINFO_LAST_PRESSED : 0;
			pInputListener->KeyDown((EKeyCode)wParam, nExtraInfo);
		}
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			dword nExtraInfo = 0;
			nExtraInfo |= (nMsg == WM_SYSKEYUP) ? INPUT_EXTRAINFO_IS_SYSKEY : 0;
			pInputListener->KeyUp((EKeyCode)wParam, nExtraInfo);
		}
		return 0;*/
	case WM_SIZE:
		{
			dword nWidth = LOWORD(lParam);
			dword nHeight = HIWORD(lParam);
			IDisplay *pDisplay = Global::m_pDisplay;
			if (pDisplay != nullptr)
				pDisplay->OnDisplayResized(nWidth, nHeight);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, nMsg, wParam, lParam);
	}
}

CDisplayWindows::CDisplayWindows(const DisplayParameters &params) {
	DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;
	DWORD dwStyleEx = 0;
	int nLeft, nTop;
	dword nWidth, nHeight;
	if (params.m_bFullScreen) {
		dwStyle |= WS_POPUP;
		dwStyleEx |= WS_EX_TOPMOST;
		nLeft = nTop = 0;
		RECT rect;
		GetWindowRect(GetDesktopWindow(), &rect);
		nWidth = rect.right - rect.left;
		nHeight = rect.bottom - rect.top;
	}
	else {
		switch (params.m_eBorder) {
		case EBorderMode::EBorder_None:
			dwStyle |= WS_POPUP;
			break;
		case EBorderMode::EBorder_Resized:
			dwStyle |= WS_OVERLAPPEDWINDOW;
			break;
		case EBorderMode::EBorder_Fixed:
		default:
			dwStyle |= (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
			break;
		}
		nLeft = params.m_nLeft;
		nTop = params.m_nTop;
		nWidth = params.m_nWidth;
		nHeight = params.m_nHeight;
	}
	HINSTANCE hInst = GetModuleHandle(nullptr);
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInst;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = nullptr;
	WndClass.lpszClassName = params.m_pszTitle;
	RegisterClass(&WndClass);

	RECT rect;
	SetRect(&rect, 0, 0, nWidth, nHeight);
	AdjustWindowRect(&rect, dwStyle, FALSE);
	nWidth = rect.right - rect.left;
	nHeight = rect.bottom - rect.top;
	HWND hWnd = CreateWindowEx(dwStyleEx, params.m_pszTitle, params.m_pszTitle,
		dwStyle, nLeft, nTop, nWidth, nHeight, params.m_hParent, nullptr, hInst, nullptr);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	m_nLeft = nLeft; m_nTop = nTop;
	m_nWidth = params.m_nWidth; m_nHeight = params.m_nHeight;
	m_dwStyle = dwStyle;
	m_hWnd = hWnd;
	m_bFullScreen = params.m_bFullScreen;
}
void CDisplayWindows::Move(int x, int y) {
	SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	__super::Move(x, y);
}
void CDisplayWindows::Resize(dword w, dword h) {
	RECT rect;
	SetRect(&rect, 0, 0, w, h);
	AdjustWindowRect(&rect, m_dwStyle, FALSE);
	SetWindowPos(m_hWnd, nullptr, 0, 0, 
		rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	__super::Resize(w, h);
}
bool CDisplayWindows::MessagePump() {
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}
	return true;
}