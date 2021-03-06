#include "RenderStageDX9_SceneDepth.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

CRenderStageDX9_SceneDepth::CRenderStageDX9_SceneDepth(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_SceneDepth::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_SceneDepth::Render(CCamera *pCamera)
{
    RenderItem::RenderItems &items = *m_pShaderBatch;
	if (!items.Empty()) 
    {
        CShaderManager * __restrict pShaderManager = Global::m_pShaderManager;
        IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
        CShader *pVertexShader = pShaderManager->FindShaderByName("VS_SceneDepth");
        CShader *pPixelShader = pShaderManager->FindShaderByName("PS_SceneDepth");
        if (pVertexShader && pPixelShader)
        {
            pRenderBackend->RestoreTarget();
            pRenderBackend->m_Cache.m_nCurrentRenderTarget = 0;

            pRenderBackend->SetTarget(InternalTextures::s_pViewDepth.Get(), nullptr);
            pRenderBackend->ClearTarget(EClearFlag_Color | EClearFlag_Depth | EClearFlag_Stencil, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
            pRenderBackend->m_Cache.m_nCurrentRenderTarget = InternalTextures::s_pViewDepth->GetID();

            const AutoUpdatedUniform &ViewProjConstant = pShaderManager->GetAutoUpdatedUniform(EAutoUpdatedConstant_ViewProj);
            Matrix4 VP((float *)ViewProjConstant.m_pData);
            const AutoUpdatedUniform &ViewConstant = pShaderManager->GetAutoUpdatedUniform(EAutoUpdatedConstant_View);
            Matrix4 V((float *)ViewConstant.m_pData);

            pRenderBackend->SetShader(pVertexShader);
            pRenderBackend->m_Cache.m_nCurrentVS = pVertexShader->GetId();
            dword nWVPIndex = pVertexShader->GetConstantIndexByName(AutoUpdatedShaderConstantIdStr::s_WorldViewProjMatrix);
            dword nWVIndex = pVertexShader->GetConstantIndexByName(AutoUpdatedShaderConstantIdStr::s_WorldViewMatrix);

            pRenderBackend->SetShader(pPixelShader);
            pRenderBackend->m_Cache.m_nCurrentPS = pPixelShader->GetId();
            dword nNearFarIndex = pPixelShader->GetConstantIndexByName(AutoUpdatedShaderConstantIdStr::s_NearFarClip);
            const AutoUpdatedUniform &NearFarConstant = pShaderManager->GetAutoUpdatedUniform(EAutoUpdatedConstant_NearFar);
            g_pDevice9->SetPixelShaderConstantF(nNearFarIndex, (float*)NearFarConstant.m_pData, 1);

            g_pDevice9->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

            for (auto &item : items)
            {
                g_pDevice9->SetVertexShaderConstantF(nWVPIndex, ((*item.m_pRenderObj->m_pWorldTransform) * VP).m, 4);
                g_pDevice9->SetVertexShaderConstantF(nWVIndex, ((*item.m_pRenderObj->m_pWorldTransform) * V).m, 4);

                RenderStageDX9_SetGeometryData(&item, pRenderBackend);
            }
            
            g_pDevice9->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        }
    }
}