#include "RenderNode.h"
#include "RenderItem.h"

void IRenderNode::UpdateWSBoundingBox()
{
    assert(m_pBoundingBox != nullptr);
    m_pBoundingBox->Transform(*m_pTransform);
}

void IRenderNode::PreRenderInternal(CCamera *pCamera, EPreRenderMode mode, struct RenderObject *pRenderObj, CMaterial *pMtl)
{
	RenderItem *pRenderItem = nullptr;
#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
	switch (mode)
	{
		// case IRenderNode::EPreRenderMode_ShadowCaster:
		// 	pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_ShadowDepth);
		// 	break;
		// case IRenderNode::EPreRenderMode_ShadowReceiver:
		// 	pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_ShadowMask);
		// 	break;
		case IRenderNode::EPreRenderMode_General:
		{
			const CArray<CPass*> &Passes = pMtl->GetPasses();
			for (const auto &Pass : Passes)
			{
				pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_SceneDepth);
				pRenderItem->m_pRenderObj = pRenderObj;

				pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_Opaque);
				pRenderItem->m_pRenderObj = pRenderObj;
				pRenderItem->m_pVSShaderObj = Pass->GetVertexShaderRef()->GetShaderObject();
				pRenderItem->m_pPSShaderObj = Pass->GetPixelShaderRef()->GetShaderObject();
				pRenderItem->m_pShaderResources = Pass->GetShaderResources();
				pRenderItem->m_nSortVal = Pass->GetHashId();
			}
			break;
		}
	}
#endif
}