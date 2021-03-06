#include "RenderNode.h"
#include "RenderItem.h"
#include "Camera.h"

void IRenderNode::UpdateWSBoundingBox()
{
	if (BIT_CHECK(m_nInternalFlag, RN_FLAG_INTERNAL_BBOX_DIRTY))
	{
		assert(m_pBoundingBox != nullptr);
    	m_pBoundingBox->Transform(*m_pTransform);
		BIT_REMOVE(m_nInternalFlag, RN_FLAG_INTERNAL_BBOX_DIRTY);
	}
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

				if (Pass->IsTranslucentPass())
				{
					pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_Transluent);
					pRenderItem->m_Distance = (m_pTransform->GetTranslation() - pCamera->GetEye()).SquareLength();
				}
				else
				{
					pRenderItem = RenderItem::AddItem(EForwardShading_ShaderBatch_Opaque);
				}

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