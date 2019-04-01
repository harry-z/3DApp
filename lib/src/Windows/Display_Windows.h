#pragma once

#include "IDisplay_Windows.h"

class CDisplayWindows : public IDisplayWindows {
public:
	CDisplayWindows();
	
	virtual void Move(int x, int y) override;
	virtual void Resize(dword w, dword h) override;
	virtual bool MessagePump() override;
	
	

private:
	
	DWORD m_dwStyle;
	
};