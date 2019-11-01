#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

#define WORLD_MATRIX "WorldMatrix"
#define WORLD_VIEW_MATRIX "WorldViewMatrix"
#define WORLD_VIEW_PROJECTION_MATRIX "WorldViewProjectionMatrix"
#define INVERSE_WORLD_MATRIX "InverseWorldMatrix"
#define INVERSE_WORLD_VIEW_MATRIX "InverseWorldViewMatrix"
#define INVERSE_WORLD_VIEW_PROJECTION_MATRIX "InverseWorldViewProjectionMatrix"
#define VIEW_MATRIX "ViewMatrix"
#define VIEW_PROJECTION_MATRIX "ViewProjectionMatrix"
#define INVERSE_VIEW_MATRIX "InverseViewMatrix"
#define INVERSE_VIEW_PROJECTION_MATRIX "InverseViewProjectionMatrix"
#define PROJECTION_MATRIX "ProjectionMatrix"
#define INVERSE_PROJECTION_MATRIX "InverseProjectionMatrix"
#define CAMERA_POSITION "CameraPosition"
#define CAMERA_DIRECTION "CameraDirection"
#define NEAR_FAR_CLIP "NearFarClip"

struct AutoUpdatedShaderConstantIdStr
{
    static IdString s_WorldMatrix;
    static IdString s_WorldViewMatrix;
    static IdString s_WorldViewProjMatrix;
    static IdString s_InvWorldMatrix;
    static IdString s_InvWorldViewMatrix;
    static IdString s_InvWorldViewProjMatrix;
    static IdString s_ViewMatrix;
    static IdString s_ViewProjMatrix;
    static IdString s_InvViewMatrix;
    static IdString s_InvViewProjMatrix;
    static IdString s_ProjMatrix;
    static IdString s_InvProjMatrix;
    static IdString s_CamPos;
    static IdString s_CamDir;
    static IdString s_NearFarClip;
};

struct ShaderConstantInfo
{
    IdString m_Name;
    EShaderConstantType m_Type;
    dword m_RegisterCount;

    ShaderConstantInfo()
    : m_Type(EShaderConstantType::EShaderConstantType_Unknown)
    , m_RegisterCount(0) {}

    ShaderConstantInfo(const String &szStr, EShaderConstantType Type, dword nCount)
    : m_Name(szStr)
    , m_Type(Type)
    , m_RegisterCount(nCount) {}
};

struct AutoUpdatedConstant
{
    dword m_nConstantCount = 0;
    byte *m_pData = nullptr;

    ~AutoUpdatedConstant()
    {
        if (m_pData != nullptr)
        {
            MEMFREE(m_pData);
        }
    }
};

class CShader 
{
public:
	virtual bool Load(EShaderType eType, const byte *pszShaderByteCode) = 0;
    virtual dword GetConstantIndexByName(const IdString &szName) const = 0;
    inline word GetId() const { return m_nId; } 
    inline EShaderType GetShaderType() const { return m_Type; }

protected:
    CShader() {}
	virtual ~CShader() {}

protected:
	word m_nId;
    EShaderType m_Type = EShaderType::EShaderType_Unknown;
};

class CCamera;
class CShaderManager 
{
public:
    CShaderManager() { InitializeAutoShaderConstantMap(); }
    virtual ~CShaderManager() {}
    virtual bool LoadShaders() = 0;
    inline CShader* FindShaderByName(const String &szName) {
        IdString IdStr(szName);
        ShaderMap::_MyIterType ShaderIterator = m_ShaderMap.Find(IdStr);
        return ShaderIterator ? ShaderIterator.Value() : nullptr;
    }
    inline CShader* FindShaderById(word nShaderId) {
        return m_ShaderArr.IsValidIndex(nShaderId - 1) ? m_ShaderArr[nShaderId - 1] : nullptr;
    }
    CShader* GetDefaultVertexShader() { return m_pDefaultVertexShader; }
    CShader* GetDefaultPixelShader() { return m_pDefaultPixelShader; }

    bool IsAutoUpdatedShaderConstant(const IdString &idStr) const;
    const ShaderConstantInfo* FindAutoUpdatedShaderConstantInfo(const IdString &idStr) const;

    AutoUpdatedConstant& GetAutoUpdatedConstant(EAutoUpdatedConstant Constant);
    const AutoUpdatedConstant& GetAutoUpdatedConstant(EAutoUpdatedConstant AutoUpdatedConstant) const;

    void UpdateShaderConstantInfoPerFrame(CCamera *pCamera);

private:
    void InitializeAutoShaderConstantMap();

protected:
    using ShaderMap = CMap<IdString, CShader*>;
    ShaderMap m_ShaderMap;
    using AutoUpdatedShaderConstantMap = CMap<IdString, ShaderConstantInfo>;
    AutoUpdatedShaderConstantMap m_AutoUpdatedShaderConstMap;
    using AutoUpdatedConstants = CArray<AutoUpdatedConstant>;
    AutoUpdatedConstants m_AutoUpdatedConstants;
    using ShaderArr = CArray<CShader*>;
    ShaderArr m_ShaderArr;
    CShader *m_pDefaultVertexShader = nullptr;
    CShader *m_pDefaultPixelShader = nullptr;
};