#pragma once
#include "..\..\RendererStableHeader.h"
#include "Shader.h"

class CShaderDX9 final : public CShader 
{
public:
    friend class CShaderManagerDX9;

    CShaderDX9();
    virtual ~CShaderDX9();
    virtual bool Load(EShaderType eType, const byte *pszShaderByteCode, dword nCodeSize) override;
    virtual const ShaderVariableInfo& GetUniformInfoByName(const IdString &szName) const override;

    union {
        LPDIRECT3DVERTEXSHADER9 m_pVertexShader;
        LPDIRECT3DPIXELSHADER9 m_pPixelShader;
    } m_Shader;

protected:
    virtual bool FillVariableMap(LPCVOID pFunction, dword nCodeSize) override;
};

class CShaderManagerDX9 final : public CShaderManager 
{
public:
    virtual ~CShaderManagerDX9();
    virtual bool LoadShaders() override;
};

EShaderConstantType MappingShaderConstantType(D3DXPARAMETER_TYPE d3dType);