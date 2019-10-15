#pragma once
#include "3DAppLib.h"

class ISample
{
public:
    virtual void InitSample() = 0;
    virtual void UninitSample() = 0;
};

using SampleMap = CMap<String, ISample*>;
extern SampleMap g_SampleMap;