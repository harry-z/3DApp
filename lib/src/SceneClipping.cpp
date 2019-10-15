#include "SceneClipping.h"

void ISceneClippingStrategy::RenderNode(CCamera *pCamera, IRenderNode *pNode)
{
    pNode->PreRender(pCamera, IRenderNode::EPreRenderMode_General);
}

void CDefaultSceneClippingStrategy::SceneClipping(CCamera *pCamera, Linklist<IRenderNode> &lstSceneNode)
{
    const Plane * __restrict pFrustumPlanes = pCamera->GetFrustumPlanes();
    Linklist<IRenderNode>::_NodeType *pTemp = lstSceneNode.m_pRoot;
    while (pTemp != nullptr)
    {
    	IRenderNode *pNode = pTemp->m_pOwner;
    	const AxisAlignedBox &aabb = pNode->GetWSBox();
    	bool bVisible = true;
    	for (dword i = 0, iEnd = (dword)EFrustumPlane::EFrustumPlane_Num; i < iEnd; ++i)
    	{
    		if (pFrustumPlanes[i].Side(aabb) == -1)
    		{
    			bVisible = false;
    			break;
    		}
    	}
    	bVisible = bVisible || pNode->CheckFlag(RN_FLAG_ALWAYSVISIBLE);
    	if (bVisible)
    		RenderNode(pCamera, pNode);
    }
}