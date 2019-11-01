#include "RenderStageDX9_Opaque.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

bool _SortOpaque(const RenderItem &LHS, const RenderItem &RHS) {
    return LHS.m_nSortVal < RHS.m_nSortVal;
}

CRenderStageDX9_Opaque::CRenderStageDX9_Opaque(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_Opaque::Prepare(CCamera *pCamera)
{
    if (!m_pShaderBatch->Empty())
        std::sort(&((*m_pShaderBatch)[0]), &((*m_pShaderBatch)[m_pShaderBatch->Num() - 1]), _SortOpaque);
}

void CRenderStageDX9_Opaque::Render(CCamera *pCamera)
{
    RenderItem::RenderItems &items = *m_pShaderBatch;
    if (!items.Empty())
    {
        CShaderManager * __restrict pShaderManager = Global::m_pShaderManager;
        IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;

        pRenderBackend->RestoreTarget();
        pRenderBackend->ClearTarget(EClearFlag_Color, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

        // Enable Early-Z
        g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

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
                RenderStageDX9_SetGeometryData(&item, pRenderBackend);
            }
        }

        g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }
}