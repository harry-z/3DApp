#pragma once
#include "Prereq.h"

class IPlatform
{
public:
    virtual ~IPlatform() {}

    virtual void SetCursorPos(int x, int y) {}
};