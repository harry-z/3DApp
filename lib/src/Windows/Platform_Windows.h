#pragma once
#include "Platform.h"

class CPlatformWindows final : public IPlatform
{
public:
    virtual void SetCursorPos(int x, int y) override;
    virtual void GetCursorPos(int &x, int &y) override;
    virtual void GetDesktopDimension(int &w, int &h) override;
};