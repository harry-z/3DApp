#include "3DEngine.h"

int main(int argc, char **argv)
{
    C3DEngine *p3DEngine = new C3DEngine;
    if (p3DEngine->Initialize())
    {
        p3DEngine->Run();
    }
    delete p3DEngine;
    return 0;
}