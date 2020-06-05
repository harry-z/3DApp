#pragma once
#include "..\..\RendererStableHeader.h"
#include "Shader.h"

class CShaderDX11 final : public CShader 
{
public:
    friend class CShaderManagerDX11;

    CShaderDX11();
    virtual ~CShaderDX11();
    virtual bool Load(EShaderType eType, const byte *pszShaderByteCode, dword nCodeSize) override;
    virtual const ShaderVariableInfo& GetUniformInfoByName(const IdString &szName) const override;
    bool FillConstantMap(LPCVOID pFunction, dword nCodeSize);

    union {
        ID3D11VertexShader *m_pVertexShader;
        ID3D11PixelShader *m_pPixelShader;
    } m_Shader;

    CArray<void*> m_ConstantBuffers;

protected:
    virtual bool FillVariableMap(LPCVOID pFunction, dword nCodeSize) override;
};

class CShaderManagerDX11 final : public CShaderManager 
{
public:
    virtual ~CShaderManagerDX11();
    virtual bool LoadShaders() override;
};

EShaderConstantType MappingShaderConstantType(D3D_SHADER_VARIABLE_TYPE d3dType);