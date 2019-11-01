#include "Shader.h"
#include "Camera.h"

IdString AutoUpdatedShaderConstantIdStr::s_WorldMatrix(WORLD_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_WorldViewMatrix(WORLD_VIEW_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_WorldViewProjMatrix(WORLD_VIEW_PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvWorldMatrix(INVERSE_WORLD_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvWorldViewMatrix(INVERSE_WORLD_VIEW_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvWorldViewProjMatrix(INVERSE_WORLD_VIEW_PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_ViewMatrix(VIEW_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_ViewProjMatrix(VIEW_PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvViewMatrix(INVERSE_VIEW_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvViewProjMatrix(INVERSE_VIEW_PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_ProjMatrix(PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_InvProjMatrix(INVERSE_PROJECTION_MATRIX);
IdString AutoUpdatedShaderConstantIdStr::s_CamPos(CAMERA_POSITION);
IdString AutoUpdatedShaderConstantIdStr::s_CamDir(CAMERA_DIRECTION);
IdString AutoUpdatedShaderConstantIdStr::s_NearFarClip(NEAR_FAR_CLIP);

#define INIT_AUTO_UPDATED_CONSTANT(AutoUpdatedConstantType, ConstantType, ConstantCount) \
    ConstantType *p##AutoUpdatedConstantType##Data = (ConstantType *)MEMALLOC(sizeof(ConstantType) * ConstantCount); \
    m_AutoUpdatedConstants[(dword)AutoUpdatedConstantType].m_nConstantCount = ConstantCount; \
    m_AutoUpdatedConstants[(dword)AutoUpdatedConstantType].m_pData = (byte *)p##AutoUpdatedConstantType##Data;

void CShaderManager::InitializeAutoShaderConstantMap()
{
    m_AutoUpdatedConstants.Reserve((dword)EAutoUpdatedConstant_Num);
    m_AutoUpdatedConstants.SetNum((dword)EAutoUpdatedConstant_Num);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_View, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_ViewProj, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvView, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvViewProj, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_Proj, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvProj, float, 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_CamPos, float, 3);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_CamDir, float, 3);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_NearFar, float, 2);
}

bool CShaderManager::IsAutoUpdatedShaderConstant(const IdString &idStr) const
{
    return (bool)(m_AutoUpdatedShaderConstMap.Find(idStr));
}

const ShaderConstantInfo* CShaderManager::FindAutoUpdatedShaderConstantInfo(const IdString &idStr) const
{
    AutoUpdatedShaderConstantMap::_MyConstIterType CIter = m_AutoUpdatedShaderConstMap.Find(idStr);
    return CIter ? &CIter.Value() : nullptr;
}

AutoUpdatedConstant& CShaderManager::GetAutoUpdatedConstant(EAutoUpdatedConstant Constant)
{
    dword nIndex = (dword)Constant;
    assert(m_AutoUpdatedConstants.IsValidIndex(nIndex));
    return m_AutoUpdatedConstants[nIndex];
}

const AutoUpdatedConstant& CShaderManager::GetAutoUpdatedConstant(EAutoUpdatedConstant Constant) const
{
    dword nIndex = (dword)Constant;
    assert(m_AutoUpdatedConstants.IsValidIndex(nIndex));
    return m_AutoUpdatedConstants[nIndex];   
}

void CShaderManager::UpdateShaderConstantInfoPerFrame(CCamera *pCamera)
{
    bool bViewConstNeedUpdate = pCamera->IsViewMatrixShaderConstNeedUpdate();
    bool bProjConstNeedUpdate = pCamera->IsProjectionMatrixShaderConstNeedUpdate();
    if (bViewConstNeedUpdate || bProjConstNeedUpdate)
    {
        const Matrix4 &ViewMatrix = pCamera->GetViewMatrix();
        Matrix4 InvViewMatrix = ViewMatrix.GetInverse();
        const Matrix4 &ProjMatrix = pCamera->GetProjMatrix();
        Matrix4 InvProjMatrix = ProjMatrix.GetInverse();
        Matrix4 ViewProjMatrix = ViewMatrix * ProjMatrix;
        Matrix4 InvViewProjMatrix = InvProjMatrix * InvViewMatrix;

        if (bViewConstNeedUpdate)
        {
            AutoUpdatedConstant &ViewConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_View);
            memcpy(ViewConstant.m_pData, ViewMatrix.m, sizeof(float) * ViewConstant.m_nConstantCount);

            AutoUpdatedConstant &InvViewConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_InvView);
            memcpy(InvViewConstant.m_pData, InvViewMatrix.m, sizeof(float) * InvViewConstant.m_nConstantCount);

            const Vec3 &Eye = pCamera->GetEye();
            AutoUpdatedConstant &CamPosConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_CamPos);
            memcpy(CamPosConstant.m_pData, &Eye.x, sizeof(float) * CamPosConstant.m_nConstantCount);

            const Vec3 &Lookat = pCamera->GetLookat();
            Vec3 Dir = Lookat - Eye; Dir.Normalize();
            AutoUpdatedConstant &CamDirConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_CamDir);
            memcpy(CamDirConstant.m_pData, &Dir.x, sizeof(float) * CamDirConstant.m_nConstantCount);

            pCamera->ViewMatrixShaderConstUpdated();    
        }

        if (bProjConstNeedUpdate)
        {
            AutoUpdatedConstant &ProjConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_Proj);
            memcpy(ProjConstant.m_pData, ProjMatrix.m, sizeof(float) * ProjConstant.m_nConstantCount);

            AutoUpdatedConstant &InvProjConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_InvProj);
            memcpy(InvProjConstant.m_pData, InvProjMatrix.m, sizeof(float) * InvProjConstant.m_nConstantCount);

            AutoUpdatedConstant &NearFar = GetAutoUpdatedConstant(EAutoUpdatedConstant_NearFar);
            float *pf = (float *)NearFar.m_pData;
            *pf = pCamera->GetNearClip();
            *(pf + 1) = pCamera->GetFarClip();

            pCamera->ProjectionMatrixShaderConstUpdated();
        }

        AutoUpdatedConstant &ViewProjConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_ViewProj);
        memcpy(ViewProjConstant.m_pData, ViewProjMatrix.m, sizeof(float) * ViewProjConstant.m_nConstantCount);

        AutoUpdatedConstant &InvViewProjConstant = GetAutoUpdatedConstant(EAutoUpdatedConstant_InvViewProj);
        memcpy(InvViewProjConstant.m_pData, InvViewProjMatrix.m, sizeof(float) * InvViewProjConstant.m_nConstantCount);
    }
}