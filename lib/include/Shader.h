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

struct ShaderParamInfo
{
    IdString m_Name;
    EShaderConstantType m_Type;
    dword m_nLengthInBytes;

    ShaderParamInfo()
    : m_Type(EShaderConstantType::EShaderConstantType_Unknown)
    , m_nLengthInBytes(0) {}

    ShaderParamInfo(const String &szStr, EShaderConstantType Type, dword nSizeInBytes)
    : m_Name(szStr)
    , m_Type(Type)
    , m_nLengthInBytes(nSizeInBytes) {}
};

struct ShaderUniformInfo
{
    ShaderParamInfo m_ParamInfo;
    dword m_nRegisterIndex;
    dword m_nOffsetInBytes;

    ShaderUniformInfo()
    : m_nRegisterIndex(0xFFFFFFFF)
    , m_nOffsetInBytes(0)
    {}
    ShaderUniformInfo(const String &szStr, EShaderConstantType Type, dword nRegisterIndex, dword nLengthInBytes, dword nOffsetInBytes)
    : m_ParamInfo(szStr, Type, nLengthInBytes)
    , m_nRegisterIndex(nRegisterIndex)
    , m_nOffsetInBytes(nOffsetInBytes)
    {}
};

struct AutoUpdatedUniform
{
    dword m_nSizeInByte = 0;
    byte *m_pData = nullptr;

    ~AutoUpdatedUniform()
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
	virtual bool Load(EShaderType eType, const byte *pszShaderByteCode, dword nCodeSize) = 0;
    virtual const ShaderUniformInfo& GetUniformInfoByName(const IdString &szName) const = 0;
    inline word GetId() const { return m_nId; } 
    inline EShaderType GetShaderType() const { return m_Type; }
    inline dword GetShaderByteCodeLength() const { return m_nByteCodeLen; }
    inline const byte* GetShaderByteCode() const { return m_pByteCode; }

protected:
    CShader() {}
	virtual ~CShader() {}
    virtual bool FillVariableMap(LPCVOID pFunction, dword nCodeSize) = 0;

protected:
	word m_nId;
    EShaderType m_Type = EShaderType::EShaderType_Unknown;
    dword m_nByteCodeLen = 0;
    byte *m_pByteCode = nullptr;
    CMap<IdString, ShaderUniformInfo> m_VariableMap;
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

    AutoUpdatedUniform& GetAutoUpdatedUniform(EAutoUpdatedConstant Constant);
    const AutoUpdatedUniform& GetAutoUpdatedUniform(EAutoUpdatedConstant Constant) const;

    void UpdateShaderUniformPerFrame(CCamera *pCamera);

private:
    void InitializeAutoShaderConstantMap();

protected:
    using ShaderMap = CMap<IdString, CShader*>;
    ShaderMap m_ShaderMap;
    using AutoUpdatedConstants = CArray<AutoUpdatedUniform>;
    AutoUpdatedConstants m_AutoUpdatedConstants;
    using ShaderArr = CArray<CShader*>;
    ShaderArr m_ShaderArr;
    CShader *m_pDefaultVertexShader = nullptr;
    CShader *m_pDefaultPixelShader = nullptr;
};