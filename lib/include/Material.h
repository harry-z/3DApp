#pragma once
#include "Job.h"
#include "Resource.h"
#include "Shader.h"
#include "ScriptParser.h"

struct ShaderObject;
class DLL_EXPORT CShaderRef
{
public:
    friend class CPass;

    bool AddShaderConstantInfo(const CArray<String> &arrParam);
    inline const CArray<ShaderConstantInfo>& GetShaderConstantInfo() const { return m_arrShaderConstInfo; }

    bool AddAutoShaderConstantInfo(const String &szParamName);
    inline const CArray<IdString>& GetAutoShaderConstantInfo() const { return m_arrAutoShaderConstInfo; }

    ldword Compile();

private:
    CShaderRef(EShaderType Type, word Id) : m_ShaderType(Type), m_RefId(Id) {}
    ~CShaderRef();
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
    EShaderType m_ShaderType;
    ShaderObject *m_pShaderObj = nullptr;
    CArray<ShaderConstantInfo> m_arrShaderConstInfo;
    CArray<IdString> m_arrAutoShaderConstInfo;
};

class DLL_EXPORT CPass
{
public:
    friend class CMaterial;

    CShaderRef* CreateShaderRef(EShaderType eShaderType, const String &szShaderName);
    void AddTextureSlot(const String &szTextureName);

    inline ldword GetHashId() const { return m_nHashId; }

private:
    CPass();
    CPass(const String &szName);
    ~CPass();

    ldword Compile();
    inline void NeedCompile() { m_Compiled = false; }
    inline bool IsCompiled() const { return m_Compiled; }

private:
    CShaderRef *m_pVertexShaderRef = nullptr;
    CShaderRef *m_pPixelShaderRef = nullptr;
    CArray<IdString> m_arrTexture;
    IdString m_IdStr;
    std::atomic_bool m_Compiled;
    ldword m_nHashId;
};

class DLL_EXPORT CMaterial final : public CBaseResource 
{
public:
    friend class CMaterialManager;
    friend class CMaterialParser;

    CPass* CreatePass();
    CPass* CreatePass(const String &szName);

    const String& GetName() const { return m_szReadableName; }

    bool Compile();

private:
    CMaterial() {}
    virtual ~CMaterial();

    void Load(const String &szFilePath);
    virtual void Destroy() override;
    inline void SetName(const String &szName) { m_szReadableName = szName; }

protected:
    dword m_Id;
    IdString m_IdStr;
    String m_szReadableName;

    CArray<CPass*> m_Passes;
};

class CMaterialParser final : public IScriptParserListener
{
public:
    virtual void OnProcessNode(const String &szParamType, const CArray<String> &arrParam) override;
    inline void Error() { m_bIsError = true; }
    inline bool IsError() const { return m_bIsError; }
    CMaterial *m_pCurrentMaterial = nullptr;
    CPass *m_pCurrentPass = nullptr;
    CShaderRef *m_pCurrentShaderRef = nullptr;
    bool m_bIsError = false;
};

class DLL_EXPORT CMaterialManager 
{
public:
    CMaterialManager();
    ~CMaterialManager();

    void Initialize();
    CMaterial* CreateMaterial(const String &szName);
    CMaterial* LoadMaterial(const String &szFilePath, bool bBackground = false);
    CMaterial* FindMaterial(const String &szName);
    CMaterial* FindMaterial(const IdString &idStr);
    void DestroyMaterial(CMaterial *pMaterial);

private:
    CMaterial* CreateInstance(const IdString &idStr);

private:
	typedef CMap<IdString, CMaterial*> MaterialMap;
	MaterialMap m_MaterialMap;
    CReferencedPointer<CMaterial> m_DefaultMtlPtr;
    CPool m_MaterialPool;
	std::mutex m_MaterialMapLock;
	dword m_MaterialId;
};