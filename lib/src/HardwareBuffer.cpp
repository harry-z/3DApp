#include "HardwareBuffer.h"

IVertexLayout* CHardwareBufferManager::GetPredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout)
{
    dword nIndex = (dword)PredefinedLayout;
    if (m_arrPredefinedVertexLayout.IsValidIndex(nIndex))
    {
        return m_arrPredefinedVertexLayout[nIndex];
    }
    else
    {
        return nullptr;
    }
}

IVertexLayout* CHardwareBufferManager::GetVertexLayout(const String &szName)
{
    IdString idStr(szName);
    VertexLayoutMap::_MyIterType Iter = m_VertexLayoutMap.Find(idStr);
    return Iter ? Iter.Value() : nullptr;
}