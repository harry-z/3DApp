#pragma once

#include "Display.h"

class IDisplayWindows : public IDisplay {
public:
	FORCE_INLINE HWND GetHWnd() const { return m_hWnd; }
	FORCE_INLINE bool IsFullScreen() const { return m_bFullScreen; }
protected:
	HWND m_hWnd;
	bool m_bFullScreen;
};