#pragma once
#include "Platform.h"

class CPlatformWindows final : public IPlatform
{
public:
    virtual void SetCursorPos(int x, int y) override;
};