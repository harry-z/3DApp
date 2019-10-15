#include "3DApp.h"

SampleMap g_SampleMap;

int main(int argc, char **argv)
{
    C3DEngine *p3DEngine = NEW_TYPE(C3DEngine);
    if (p3DEngine->Initialize())
    {
        printf("Sample list\n");
        SampleMap::_MyIterType Iter = g_SampleMap.CreateIterator();
        dword nIndex = 1;
        for (; Iter; ++Iter)
        {
            printf("%d: %s", nIndex++, Iter.Key().c_str());
            Iter.Value()->InitSample();
        }
        p3DEngine->Run();
    }
    SampleMap::_MyIterType Iter = g_SampleMap.CreateIterator();
    for (; Iter; ++Iter)
        Iter.Value()->UninitSample();
    DELETE_TYPE(p3DEngine, C3DEngine);
    return 0;
}