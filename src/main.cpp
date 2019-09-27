#include "3DEngine.h"
#include "Material.h"

int main(int argc, char **argv)
{
    C3DEngine *p3DEngine = NEW_TYPE(C3DEngine);
    if (p3DEngine->Initialize())
    {
        // Global::m_pMaterialManager->LoadMaterial("Mtl/Template.mtl");
        p3DEngine->Run();
    }
    DELETE_TYPE(p3DEngine, C3DEngine);
    return 0;
}