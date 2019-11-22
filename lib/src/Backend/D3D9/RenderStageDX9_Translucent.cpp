#include "RenderStageDX9_Translucent.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

bool _SortTranslucent(const RenderItem &LHS, const RenderItem &RHS) {
    return LHS.m_Distance > RHS.m_Distance;
}

CRenderStageDX9_Translucent::CRenderStageDX9_Translucent(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_Translucent::Prepare(CCamera *pCamera)
{
    if (!m_pShaderBatch->Empty())
        std::sort(&((*m_pShaderBatch)[0]), &((*m_pShaderBatch)[m_pShaderBatch->Num() - 1]), _SortTranslucent);
}

void CRenderStageDX9_Translucent::Render(CCamera *pCamera)
{
    RenderItem::RenderItems &items = *m_pShaderBatch;
    if (!items.Empty())
    {
        CShaderManager * __restrict pShaderManager = Global::m_pShaderManager;
        IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;

        if (pRenderBackend->m_Cache.NeedUpdateRenderTarget(0)) {
			pRenderBackend->RestoreTarget();
            pRenderBackend->ClearTarget(EClearFlag_Color, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
            pRenderBackend->m_Cache.m_nCurrentRenderTarget = 0;
		}

        g_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        for (auto &item : items)
        {
            word nCurrentVS = (word)GetVertexShaderId(item.m_nSortVal);
            CShader *pVertexShader = nCurrentVS != 0 ? pShaderManager->FindShaderById(nCurrentVS) : pShaderManager->GetDefaultVertexShader();
            word nCurrentPS = (word)GetPixelShaderId(item.m_nSortVal);
            CShader *pPixelShader = nCurrentPS != 0 ? pShaderManager->FindShaderById(nCurrentPS) : pShaderManager->GetDefaultPixelShader();
            if (pVertexShader && pPixelShader)
            {
                RenderStageDX9_SetVertexShaderAndConstants(&item, pRenderBackend, pVertexShader);
                RenderStageDX9_SetPixelShaderAndConstants(&item, pRenderBackend, pPixelShader);
                RenderStageDX9_SetShaderResources(item.m_pShaderResources, pRenderBackend);
                RenderStageDX9_SetGeometryData(&item, pRenderBackend);
            }
        }

        g_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		g_pDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		g_pDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    }
}