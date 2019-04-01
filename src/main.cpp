#include <iostream>
#include "3DEngine.h"

int main(int argc, char **argv)
{
    // DisplayParameters DisplayParams;
    // DisplayParams.m_nLeft = 0;
    // DisplayParams.m_nTop = 0;
    // DisplayParams.m_nWidth = 1280;
    // DisplayParams.m_nHeight = 720;
    // DisplayParams.m_bFullScreen = false;
    // DisplayParams.m_eBorder = EBorderMode::EBorder_Fixed;
    // DisplayParams.m_pszTitle = "3DApp";
    // DisplayParams.m_hParent = 0;
    // CDisplayWindows *pWindowsDisplay = new CDisplayWindows(DisplayParams);
    C3DEngine *p3DEngine = new C3DEngine;
    if (p3DEngine->Initialize())
    {
        p3DEngine->Run();
    }
    delete p3DEngine;
    // delete pWindowsDisplay;
    return 0;
}