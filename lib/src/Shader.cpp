#include "Shader.h"

#define WORLD_MATRIX "WorldMatrix"
#define WORLD_VIEW_MATRIX "WorldViewMatrix"
#define WORLD_VIEW_PROJECTION_MATRIX "WorldViewProjectionMatrix"
#define INVERSE_WORLD_MATRIX "InverseWorldMatrix"
#define INVERSE_WORLD_VIEW_MATRIX "InverseWorldViewMatrix"
#define INVERSE_WORLD_VIEW_PROJECTION_MATRIX "InverseWorldViewProjectionMatrix"
#define CAMERA_POSITION "CameraPosition"
#define CAMERA_DIRECTION "CameraDirection"

#define INIT_CONSTANT_INFO(InfoName, ValueType, ValueCount, ConstantType, ConstantCount) \
    ValueType *pData##InfoName = (ValueType*)MEMALLOC(sizeof(ValueType) * ValueCount); \
    memset(pData##InfoName, 0, sizeof(ValueType) * ValueCount); \
    ShaderConstantInfo ConstantInfo##InfoName(#InfoName, ConstantType, ConstantCount, (byte*)pData##InfoName); \
    m_AutoShaderConstMap.Insert(IdString(#InfoName), ConstantInfo##InfoName);

void CShaderManager::InitializeAutoShaderConstantMap()
{
    INIT_CONSTANT_INFO(WORLD_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(WORLD_VIEW_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(WORLD_VIEW_PROJECTION_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(INVERSE_WORLD_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(INVERSE_WORLD_VIEW_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(INVERSE_WORLD_VIEW_PROJECTION_MATRIX, float, 16, EShaderConstantType::EShaderConstantType_Float4, 4);
    INIT_CONSTANT_INFO(CAMERA_POSITION, float, 3, EShaderConstantType::EShaderConstantType_Float4, 1);
    INIT_CONSTANT_INFO(CAMERA_DIRECTION, float, 3, EShaderConstantType::EShaderConstantType_Float4, 1);
}

bool CShaderManager::IsAutoShaderConstant(const IdString &idStr) const
{
    return (bool)(m_AutoShaderConstMap.Find(idStr));
}

const ShaderConstantInfo* CShaderManager::FindShaderConstantInfo(const IdString &idStr) const
{
    AutoShaderConstantMap::_MyConstIterType CIter = m_AutoShaderConstMap.Find(idStr);
    return CIter ? &CIter.Value() : nullptr;
}

void CShaderManager::UpdateShaderConstantInfoPerFrame()
{
    
}