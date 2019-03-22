#pragma once

#include "IDisplay_Windows.h"


enum class EBorderMode {
	EBorder_None,
	EBorder_Fixed,
	EBorder_Resized
};

struct DisplayParameters {
	int m_nLeft, m_nTop;
	dword m_nWidth, m_nHeight;
	bool m_bFullScreen;
	EBorderMode m_eBorder;
	const char *m_pszTitle;
	HWND m_hParent;
};

class DLL_EXPORT CDisplayWindows : public IDisplayWindows {
public:
	CDisplayWindows(const DisplayParameters &params);
	
	virtual void Move(int x, int y) override;
	virtual void Resize(dword w, dword h) override;
	virtual bool MessagePump() override;
	
	

private:
	
	DWORD m_dwStyle;
	
};