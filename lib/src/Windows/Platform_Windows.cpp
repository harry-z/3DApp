#include "Platform_Windows.h"

void CPlatformWindows::SetCursorPos(int x, int y)
{
    ::SetCursorPos(x, y);
}