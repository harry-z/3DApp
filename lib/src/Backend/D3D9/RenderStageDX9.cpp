#include "RenderStageDX9.h"
#include "Shader.h"
#include "Material.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

typedef void (*SetConstantF)(UINT, CONST FLOAT*, UINT);
typedef void (*SetConstantI)(UINT, CONST INT*, UINT);

void SetVertexShaderConstantF(UINT nIndex, CONST FLOAT *pData, UINT nCount)
{
    g_pDevice9->SetVertexShaderConstantF(nIndex, pData, nCount);
}

void SetVertexShaderConstantI(UINT nIndex, CONST INT *pData, UINT nCount)
{
    g_pDevice9->SetVertexShaderConstantI(nIndex, pData, nCount);
}

void SetPixelShaderConstantF(UINT nIndex, CONST FLOAT *pData, UINT nCount)
{
    g_pDevice9->SetPixelShaderConstantF(nIndex, pData, nCount);
}

void SetPixelShaderConstantI(UINT nIndex, CONST INT *pData, UINT nCount)
{
    g_pDevice9->SetPixelShaderConstantI(nIndex, pData, nCount);
}

void RenderStageDX9_SetShaderConstants(ShaderObject *pShaderObject, SetConstantF pFuncF, SetConstantI pFuncI)
{
    for (const auto &Variable : pShaderObject->m_arrShaderVar)
    {
        if (Variable.m_Type == EShaderConstantType::EShaderConstantType_Float)
            (*pFuncF)(Variable.m_nStartRegister, (CONST FLOAT*)Variable.m_pData, Variable.m_nUsedRegister);
        else if (Variable.m_Type == EShaderConstantType::EShaderConstantType_Int)
            (*pFuncI)(Variable.m_nStartRegister, (CONST INT*)Variable.m_pData, Variable.m_nUsedRegister);
    }
}

void RenderStageDX9_SetShaderAutoConstants(ShaderObject *pShaderObject, Matrix4 *pWorldTransform, CShader *pShader, SetConstantF pFuncF, SetConstantI pFuncI)
{
    CShaderManager * __restrict pShaderManager = Global::m_pShaderManager;
    for (const auto &AutoVariable : pShaderObject->m_arrAutoShaderVar)
    {
        dword nIndex = pShader->GetConstantIndexByName(AutoVariable);
        if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_WorldMatrix)
            (*pFuncF)(nIndex, pWorldTransform->m, 4);
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_WorldViewMatrix)
        {
            AutoUpdatedConstant &ViewConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_View);
            Matrix4 V((float *)ViewConstant.m_pData);
            (*pFuncF)(nIndex, ((*pWorldTransform) * V).m, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_WorldViewProjMatrix)
        {
            AutoUpdatedConstant &ViewProjConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_ViewProj);
            Matrix4 VP((float *)ViewProjConstant.m_pData);
            (*pFuncF)(nIndex, ((*pWorldTransform) * VP).m, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_InvWorldMatrix)
        {
            Matrix4 InvW = pWorldTransform->GetInverse();
            (*pFuncF)(nIndex, InvW.m, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_InvWorldViewMatrix)
        {
            AutoUpdatedConstant &InvViewConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_InvView);
            Matrix4 InvV((float *)InvViewConstant.m_pData);
            Matrix4 InvW = pWorldTransform->GetInverse();
            (*pFuncF)(nIndex, (InvV * InvW).m, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_InvWorldViewProjMatrix)
        {
            AutoUpdatedConstant &InvViewProjConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_InvViewProj);
            Matrix4 InvVP((float *)InvViewProjConstant.m_pData);
            Matrix4 InvW = pWorldTransform->GetInverse();
            (*pFuncF)(nIndex, (InvVP * InvW).m, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_ViewMatrix)
        {
            AutoUpdatedConstant &ViewConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_View);
            (*pFuncF)(nIndex, (CONST FLOAT*)ViewConstant.m_pData, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_ViewProjMatrix)
        {
            AutoUpdatedConstant &ViewProjConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_ViewProj);
            (*pFuncF)(nIndex, (CONST FLOAT*)ViewProjConstant.m_pData, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_ProjMatrix)
        {
            AutoUpdatedConstant &ProjConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_Proj);
            (*pFuncF)(nIndex, (CONST FLOAT*)ProjConstant.m_pData, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_InvProjMatrix)
        {
            AutoUpdatedConstant &InvProjConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_InvProj);
            (*pFuncF)(nIndex, (CONST FLOAT*)InvProjConstant.m_pData, 4);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_CamPos)
        {
            AutoUpdatedConstant &CamPosConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_CamPos);
            (*pFuncF)(nIndex, (CONST FLOAT*)CamPosConstant.m_pData, 1);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_CamDir)
        {
            AutoUpdatedConstant &CamDirConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_CamDir);
            (*pFuncF)(nIndex, (CONST FLOAT*)CamDirConstant.m_pData, 1);
        }
        else if (AutoVariable == AutoUpdatedShaderConstantIdStr::s_NearFarClip)
        {
            AutoUpdatedConstant &NearFarConstant = pShaderManager->GetAutoUpdatedConstant(EAutoUpdatedConstant_NearFar);
            (*pFuncF)(nIndex, (CONST FLOAT*)NearFarConstant.m_pData, 1);
        }
    }
}

void RenderStageDX9_SetVertexShaderAndConstants(RenderItem *pRI, IRenderBackend *pRenderBackend, CShader *pVertexShader)
{
    if (pRenderBackend->m_Cache.NeedUpdateVS(pVertexShader->GetId()))
    {
        pRenderBackend->SetShader(pVertexShader);
        pRenderBackend->m_Cache.m_nCurrentVS = pVertexShader->GetId();
    }

    RenderStageDX9_SetShaderConstants(pRI->m_pVSShaderObj, 
        SetVertexShaderConstantF,
        SetVertexShaderConstantI);

    RenderStageDX9_SetShaderAutoConstants(pRI->m_pVSShaderObj, 
        pRI->m_pRenderObj->m_pWorldTransform, 
        pVertexShader,
        SetVertexShaderConstantF,
        SetVertexShaderConstantI);
}

void RenderStageDX9_SetPixelShaderAndConstants(RenderItem *pRI, IRenderBackend *pRenderBackend, CShader *pPixelShader)
{
    if (pRenderBackend->m_Cache.NeedUpdatePS(pPixelShader->GetId()))
    {
        pRenderBackend->SetShader(pPixelShader);
        pRenderBackend->m_Cache.m_nCurrentPS = pPixelShader->GetId();
    }

    RenderStageDX9_SetShaderConstants(pRI->m_pPSShaderObj, 
        SetPixelShaderConstantF,
        SetPixelShaderConstantI);

    RenderStageDX9_SetShaderAutoConstants(pRI->m_pPSShaderObj, 
        pRI->m_pRenderObj->m_pWorldTransform, 
        pPixelShader,
        SetPixelShaderConstantF,
        SetPixelShaderConstantI);
}

void RenderStageDX9_SetShaderResources(ShaderResources *pSR, IRenderBackend *pRenderBackend)
{
    if (pRenderBackend->m_Cache.NeedUpdateShaderResource(pSR->m_arrTexture[0]->GetID()))
    {
        for (dword nSlot = 0; nSlot < pSR->m_arrTexture.Num(); ++nSlot)
        {
            pRenderBackend->SetTexture(nSlot, pSR->m_arrTexture[nSlot].Get());
        }
    }
}

void RenderStageDX9_SetGeometryData(RenderItem *pRI, IRenderBackend *pRenderBackend)
{
    pRenderBackend->SetVertexLayout(pRI->m_pRenderObj->m_pVertexLayout);
    pRenderBackend->SetVertexBuffers(pRI->m_pRenderObj->m_arrHwBuffer);
    if (pRI->m_pRenderObj->m_pIB != nullptr)
    {
        pRenderBackend->SetIndexBuffer(pRI->m_pRenderObj->m_pIB);
        pRenderBackend->Draw(pRI->m_pRenderObj->m_PrimType, 0, pRI->m_pRenderObj->m_nVertexCount, 0, pRI->m_pRenderObj->m_nPrimitiveCount);
    }
    else
    {
        pRenderBackend->Draw(pRI->m_pRenderObj->m_PrimType, 0, pRI->m_pRenderObj->m_nPrimitiveCount);
    }
}