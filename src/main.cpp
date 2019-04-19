#include "3DEngine.h"

int main(int argc, char **argv)
{
    C3DEngine *p3DEngine = NEW_TYPE(C3DEngine);
    if (p3DEngine->Initialize())
    {
        p3DEngine->Run();
    }
    DELETE_TYPE(p3DEngine, C3DEngine);
    return 0;
}