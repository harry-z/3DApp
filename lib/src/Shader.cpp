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

#define INIT_AUTO_UPDATED_CONSTANT(AutoUpdatedConstantType, SizeInByte) \
    byte *p##AutoUpdatedConstantType##Data = (byte *)MEMALLOC((SizeInByte)); \
    m_AutoUpdatedConstants[(dword)AutoUpdatedConstantType].m_nSizeInByte = (SizeInByte); \
    m_AutoUpdatedConstants[(dword)AutoUpdatedConstantType].m_pData = p##AutoUpdatedConstantType##Data;

void CShaderManager::InitializeAutoShaderConstantMap()
{
    m_AutoUpdatedConstants.Reserve((dword)EAutoUpdatedConstant_Num);
    m_AutoUpdatedConstants.SetNum((dword)EAutoUpdatedConstant_Num);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_View, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_ViewProj, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvView, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvViewProj, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_Proj, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_InvProj, sizeof(float) * 16);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_CamPos, sizeof(float) * 3);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_CamDir, sizeof(float) * 3);
    INIT_AUTO_UPDATED_CONSTANT(EAutoUpdatedConstant_NearFar, sizeof(float) * 2);
}

AutoUpdatedUniform& CShaderManager::GetAutoUpdatedUniform(EAutoUpdatedConstant Constant)
{
    dword nIndex = (dword)Constant;
    assert(m_AutoUpdatedConstants.IsValidIndex(nIndex));
    return m_AutoUpdatedConstants[nIndex];
}

const AutoUpdatedUniform& CShaderManager::GetAutoUpdatedUniform(EAutoUpdatedConstant Constant) const
{
    dword nIndex = (dword)Constant;
    assert(m_AutoUpdatedConstants.IsValidIndex(nIndex));
    return m_AutoUpdatedConstants[nIndex];   
}

void CShaderManager::UpdateShaderUniformPerFrame(CCamera *pCamera)
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
            AutoUpdatedUniform &ViewConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_View);
            memcpy(ViewConstant.m_pData, ViewMatrix.m, ViewConstant.m_nSizeInByte);

            AutoUpdatedUniform &InvViewConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_InvView);
            memcpy(InvViewConstant.m_pData, InvViewMatrix.m, InvViewConstant.m_nSizeInByte);

            const Vec3 &Eye = pCamera->GetEye();
            AutoUpdatedUniform &CamPosConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_CamPos);
            memcpy(CamPosConstant.m_pData, &Eye.x, CamPosConstant.m_nSizeInByte);

            const Vec3 &Lookat = pCamera->GetLookat();
            Vec3 Dir = Lookat - Eye; Dir.Normalize();
            AutoUpdatedUniform &CamDirConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_CamDir);
            memcpy(CamDirConstant.m_pData, &Dir.x, CamDirConstant.m_nSizeInByte);

            pCamera->ViewMatrixShaderConstUpdated();    
        }

        if (bProjConstNeedUpdate)
        {
            AutoUpdatedUniform &ProjConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_Proj);
            memcpy(ProjConstant.m_pData, ProjMatrix.m, ProjConstant.m_nSizeInByte);

            AutoUpdatedUniform &InvProjConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_InvProj);
            memcpy(InvProjConstant.m_pData, InvProjMatrix.m, InvProjConstant.m_nSizeInByte);

            AutoUpdatedUniform &NearFar = GetAutoUpdatedUniform(EAutoUpdatedConstant_NearFar);
            float *pf = (float *)NearFar.m_pData;
            *pf = pCamera->GetNearClip();
            *(pf + 1) = pCamera->GetFarClip();

            pCamera->ProjectionMatrixShaderConstUpdated();
        }

        AutoUpdatedUniform &ViewProjConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_ViewProj);
        memcpy(ViewProjConstant.m_pData, ViewProjMatrix.m, ViewProjConstant.m_nSizeInByte);

        AutoUpdatedUniform &InvViewProjConstant = GetAutoUpdatedUniform(EAutoUpdatedConstant_InvViewProj);
        memcpy(InvViewProjConstant.m_pData, InvViewProjMatrix.m, InvViewProjConstant.m_nSizeInByte);
    }
}