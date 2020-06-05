#include "HardwareBuffer.h"

IVertexLayout* CHardwareBufferManager::GetVertexLayout(const String &szName)
{
    IdString idStr(szName);
    VertexLayoutMap::_MyIterType Iter = m_VertexLayoutMap.Find(idStr);
    return Iter ? Iter.Value() : nullptr;
}