#pragma once
#include "Job.h"
#include "Resource.h"
#include "Shader.h"
#include "ScriptParser.h"

class DLL_EXPORT CShaderRef
{
public:
    friend class CPass;

    bool AddShaderConstantInfo(const CArray<String> &arrParam);
    inline const CArray<ShaderConstantInfo>& GetShaderConstantInfo() const { return m_arrShaderConstInfo; }

    bool AddAutoShaderConstantInfo(const String &szParamName);

private:
    CShaderRef(word Id) : m_RefId(Id) {}
    bool CheckParamIsValid(const CArray<String> &arrParam) const;
    template <class T>
    void AddShaderConstantInfo(const String &szName, EShaderConstantType ConstType, dword nCount, T *pOriginData)
    {
        byte *pData = FillConstantInfoData(ConstType, nCount, pOriginData);
        m_arrShaderConstInfo.Emplace(szName, ConstType, nCount, pData);
    }
    EShaderConstantType GetShaderConstantType(const String &szTypeName) const;
    template <class T>
    byte* FillConstantInfoData(EShaderConstantType ConstType, dword nCount, T *pData)
    {
        dword nElemCount = GetShaderConstantElementCount(ConstType);

        byte *pConstantInfoData = (byte *)MEMALLOC(sizeof(T) * 4 * nCount); 
        memset(pConstantInfoData, 0, sizeof(T) * 4 * nCount);

        dword nSkip = 4 - nElemCount;

        T *pTypedDest = (T *)pConstantInfoData;
        for (dword i = 0; i < nCount; ++i, pTypedDest += nSkip)
        {
            for (dword j = 0; j < nElemCount; ++j)
                *pTypedDest++ = *pData++;
        }
        return pConstantInfoData;
    }
    dword GetShaderConstantElementCount(EShaderConstantType ConstType) const;

    word m_RefId = 0xFFFF;
    CArray<ShaderConstantInfo> m_arrShaderConstInfo;
    CArray<IdString> m_arrAutoShaderConstInfo;
};

class DLL_EXPORT CPass
{
public:
    friend class CTechnique;

    CShaderRef* CreateShaderRef(EShaderType eShaderType, const String &szShaderName);

private:
    CPass();
    CPass(const String &szName);
    ~CPass();

private:
    CShaderRef *m_pVertexShaderRef = nullptr;
    CShaderRef *m_pPixelShaderRef = nullptr;
    IdString m_IdStr;
};

class DLL_EXPORT CTechnique
{
public:
    friend class CMaterial;

    CPass* CreatePass();
    CPass* CreatePass(const String &szName);

private:
    CTechnique();
    CTechnique(const String &szName);
    ~CTechnique();

private:
    CArray<CPass*> m_Passes;
    IdString m_IdStr;
};

class DLL_EXPORT CMaterial final : public CBaseResource 
{
public:
    friend class CMaterialManager;
    friend class CMaterialParser;

    CTechnique* CreateTechnique();
    CTechnique* CreateTechnique(const String &szName);

    const String& GetName() const { return m_szReadableName; }

private:
    CMaterial() {}
    virtual ~CMaterial();

    void Load(const String &szFilePath);
    virtual void Destroy() override {}
    inline void SetName(const String &szName) { m_szReadableName = szName; }

protected:
    dword m_Id;
    IdString m_IdStr;
    String m_szReadableName;

    CArray<CTechnique*> m_Techniques;
};

class CMaterialParser final : public IScriptParserListener
{
public:
    virtual void OnProcessNode(const String &szParamType, const CArray<String> &arrParam) override;
    inline void Error() { m_bIsError = true; }
    inline bool IsError() const { return m_bIsError; }
    CMaterial *m_pCurrentMaterial = nullptr;
    CTechnique *m_pCurrentTechnique = nullptr;
    CPass *m_pCurrentPass = nullptr;
    CShaderRef *m_pCurrentShaderRef = nullptr;
    bool m_bIsError = false;
};

class DLL_EXPORT CMaterialManager 
{
public:
    CMaterialManager();
    ~CMaterialManager();

    CMaterial* LoadMaterial(const String &szFilePath, bool bBackground = false);

private:
	typedef CMap<IdString, CMaterial*> MaterialMap;
	MaterialMap m_MaterialMap;
    CPool m_MaterialPool;
	std::mutex m_MaterialMapLock;
	dword m_MaterialId;
};