#include "Platform_Windows.h"

void CPlatformWindows::SetCursorPos(int x, int y)
{
    ::SetCursorPos(x, y);
}

void CPlatformWindows::GetCursorPos(int &x, int &y)
{
    POINT pt;
    ::GetCursorPos(&pt);
    x = pt.x;
    y = pt.y;
}

void CPlatformWindows::GetDesktopDimension(int &w, int &h)
{
    RECT rect;
    ::GetWindowRect(::GetDesktopWindow(), &rect);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;
}