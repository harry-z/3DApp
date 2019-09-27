#pragma once
#include "Prereq.h"
#include "RendererTypes.h"

struct ShaderConstantInfo
{
    IdString m_Name;
    EShaderConstantType m_Type;
    dword m_RegisterCount;
    byte *m_pData = nullptr;
    ShaderConstantInfo()
    : m_Type(EShaderConstantType::EShaderConstantType_Unknown)
    , m_RegisterCount(0)
    , m_pData(nullptr) {}
    ShaderConstantInfo(const String &szStr, EShaderConstantType Type, dword nCount, byte *pData)
    : m_Name(szStr)
    , m_Type(Type)
    , m_RegisterCount(nCount)
    , m_pData(pData) {}
    ~ShaderConstantInfo()
    {
        if (m_pData)
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
        return m_ShaderArr.IsValidIndex(nShaderId) ? m_ShaderArr[nShaderId] : nullptr;
    }
    CShader* GetDefaultVertexShader() { return m_pDefaultVertexShader; }
    CShader* GetDefaultPixelShader() { return m_pDefaultPixelShader; }

    bool IsAutoShaderConstant(const IdString &idStr) const;
    const ShaderConstantInfo* FindShaderConstantInfo(const IdString &idStr) const;
    void UpdateShaderConstantInfoPerFrame();

private:
    void InitializeAutoShaderConstantMap();

protected:
    using ShaderMap = CMap<IdString, CShader*>;
    ShaderMap m_ShaderMap;
    using AutoShaderConstantMap = CMap<IdString, ShaderConstantInfo>;
    AutoShaderConstantMap m_AutoShaderConstMap;
    using ShaderArr = CArray<CShader*>;
    ShaderArr m_ShaderArr;
    CShader *m_pDefaultVertexShader = nullptr;
    CShader *m_pDefaultPixelShader = nullptr;
};