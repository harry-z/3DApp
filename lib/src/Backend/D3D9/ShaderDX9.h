#pragma once
#include "..\..\RendererStableHeader.h"
#include "Shader.h"

class CShaderDX9 final : public CShader 
{
public:
    friend class CShaderManagerDX9;

    CShaderDX9();
    virtual ~CShaderDX9();
    virtual bool Load(EShaderType eType, const byte *pszShaderByteCode) override;
    virtual dword GetConstantIndexByName(const IdString &szName) const override;
    bool FillConstantMap(const DWORD *pFunction);

    union {
        LPDIRECT3DVERTEXSHADER9 m_pVertexShader;
        LPDIRECT3DPIXELSHADER9 m_pPixelShader;
    } m_Shader;

    CMap<IdString, dword> m_ConstantMap;
};

class CShaderManagerDX9 final : public CShaderManager 
{
public:
    virtual ~CShaderManagerDX9();
    virtual bool LoadShaders() override;
};