#pragma once
#include "Prereq.h"

class IPlatform
{
public:
    virtual ~IPlatform() {}

    virtual void SetCursorPos(int x, int y) {}
    virtual void GetCursorPos(int &x, int &y) { x = 0; y = 0; }
    virtual void GetDesktopDimension(int &w, int &h) { w = 0; h = 0; }
};