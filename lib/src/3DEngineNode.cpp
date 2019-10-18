#include "3DEngine.h"
#include "CustomGeometryNode.h"

IRenderNode* C3DEngine::CreateRenderNodeInternal(ERNType eNodeType)
{
    IRenderNode *pNode = nullptr;
    switch (eNodeType)
    {
        case ERNType::ERNType_CustomGeometry:
            pNode = NEW_TYPE(CCustomGeometryNode);
        default:
            break;
    }
    if (pNode != nullptr)
        OnCreateRenderNode(pNode);
    return pNode;
}

void C3DEngine::DestroyRenderNode(IRenderNode *pNode)
{
    assert(pNode != nullptr);
    UnregisterNode(pNode);
    OnDestroyRenderNode(pNode);
}

void C3DEngine::RegisterNode(IRenderNode *pNode)
{
    assert(pNode);
    ENodeListType ListType = GetListTypeByNodeType(pNode->GetType());
    if (ListType != ENodeListType_Unknown)
    {
        bool bCanRegister = false;
        if (m_SceneNodelist[ListType].Empty())
            bCanRegister = true;
        else if (pNode->m_node.m_pNext == nullptr && pNode->m_node.m_pPrev == nullptr)
            bCanRegister = true;

        if (bCanRegister)
            m_SceneNodelist[ListType].PushBack(&pNode->m_node);
    }
}

void C3DEngine::UnregisterNode(IRenderNode *pNode) 
{
    assert(pNode);
    ENodeListType ListType = GetListTypeByNodeType(pNode->GetType());
    if (ListType != ENodeListType_Unknown)
	    m_SceneNodelist[ListType].Remove(&pNode->m_node);
}

void C3DEngine::OnCreateRenderNode(IRenderNode *pNode) 
{
	pNode->m_pTransform = (Matrix4 *)m_MatrixPool.Allocate_mt();
    new (pNode->m_pTransform) Matrix4;
	pNode->m_pTransform->MakeIdentity();
	pNode->m_pBoundingBox = (AxisAlignedBox *)m_AABBPool.Allocate_mt();
	new (pNode->m_pBoundingBox) AxisAlignedBox;
}

void C3DEngine::OnDestroyRenderNode(IRenderNode *pNode)
{
    m_MatrixPool.Free_mt(pNode->m_pTransform);
    m_AABBPool.Free_mt(pNode->m_pBoundingBox);
}

C3DEngine::ENodeListType C3DEngine::GetListTypeByNodeType(ERNType type) const
{
    switch (type)
    {
        case ERNType::ERNType_Mesh:
        case ERNType::ERNType_CustomGeometry:
            return ENodeListType_Geometry;
        default:
            return ENodeListType_Unknown;
    }
}