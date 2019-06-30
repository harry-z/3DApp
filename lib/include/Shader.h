#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

class CShader {
public:
	virtual bool Load(EShaderType eType, const byte *pszShaderByteCode) = 0;
    inline word GetId() const { return m_nId; } 
    inline EShaderType GetShaderType() const { return m_Type; }

protected:
    CShader() {}
	virtual ~CShader() {}

protected:
	word m_nId;
    EShaderType m_Type = EShaderType::EShaderType_Unknown;
};

class CShaderManager {
public:
    CShaderManager();
    virtual ~CShaderManager() {}
    virtual bool LoadShaders() = 0;
    inline CShader* FindShaderByName(const String &szName) {
        IdString IdStr(szName);
        ShaderMap::_ValuePointerType ppShader = m_ShaderMap.Find(IdStr);
        return ppShader != nullptr ? *ppShader : nullptr;
    }
    inline CShader* FindShaderById(word nShaderId) {
        return m_ShaderArr.IsValidIndex(nShaderId) ? m_ShaderArr[nShaderId] : nullptr;
    }
    CShader* GetDefaultVertexShader() { return m_pDefaultVertexShader; }
    CShader* GetDefaultPixelShader() { return m_pDefaultPixelShader; }

protected:
    using ShaderMap = CHashmap<IdString, CShader*>;
    ShaderMap m_ShaderMap;
    using ShaderArr = CArray<CShader*>;
    ShaderArr m_ShaderArr;
    CShader *m_pDefaultVertexShader = nullptr;
    CShader *m_pDefaultPixelShader = nullptr;
};