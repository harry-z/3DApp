#include "3DApp.h"

SampleMap g_SampleMap;

int main(int argc, char **argv)
{
    dword nSelectIndex = 0xFFFFFFFF;

    C3DEngine *p3DEngine = NEW_TYPE(C3DEngine);
    if (p3DEngine->Initialize())
    {
        printf("Sample list\n");

        SampleMap::_MyIterType Iter = g_SampleMap.CreateIterator();
        dword nIndex = 1;
        for (; Iter; ++Iter)
        {
            printf("%d: %s\n", nIndex++, Iter.Key().c_str());
        }

        printf("Select Sample: ");
        scanf("%d", &nSelectIndex);
        nIndex = nSelectIndex;

        Iter = g_SampleMap.CreateIterator();
        while (nIndex > 1)
        {
            ++Iter;
            --nIndex;
        }
        Iter.Value()->InitSample();

        p3DEngine->Run();
    }

    SampleMap::_MyIterType Iter = g_SampleMap.CreateIterator();
    while (nSelectIndex > 1)
    {
        ++Iter;
        --nSelectIndex;
    }
    Iter.Value()->UninitSample();

    DELETE_TYPE(p3DEngine, C3DEngine);
    return 0;
}