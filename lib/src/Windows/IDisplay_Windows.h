#pragma once

#include "Display.h"

class IDisplayWindows : public IDisplay {
public:
	inline HWND GetHWnd() const { return m_hWnd; }
	inline bool IsFullScreen() const { return m_bFullScreen; }
protected:
	HWND m_hWnd;
	bool m_bFullScreen;
};