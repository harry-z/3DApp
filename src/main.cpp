#include <iostream>
#include "src/Windows/Display_Windows.h"

int main(int argc, char **argv)
{
    DisplayParameters DisplayParams;
    DisplayParams.m_nLeft = 0;
    DisplayParams.m_nTop = 0;
    DisplayParams.m_nWidth = 1280;
    DisplayParams.m_nHeight = 720;
    DisplayParams.m_bFullScreen = false;
    DisplayParams.m_eBorder = EBorderMode::EBorder_Fixed;
    DisplayParams.m_pszTitle = "3DApp";
    DisplayParams.m_hParent = 0;
    CDisplayWindows *pWindowsDisplay = new CDisplayWindows(DisplayParams);
    while(pWindowsDisplay->MessagePump())
    {}
    delete pWindowsDisplay;
    return 0;
}