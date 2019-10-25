#pragma once
#include "Job.h"
#include "Resource.h"
#include "Shader.h"
#include "ScriptParser.h"

struct ShaderConstantBuffer
{
    ShaderConstantInfo m_Info;
    byte *m_pData = nullptr;
    ~ShaderConstantBuffer()
    {
        if (m_pData != nullptr)
        {
            MEMFREE(m_pData);
        }
    }
};

struct ShaderObject;
class DLL_EXPORT CShaderRef
{
public:
    friend class CPass;

    bool AddShaderConstantInfo(const CArray<String> &arrParam);
    inline const CArray<ShaderConstantBuffer>& GetShaderConstantBuffer() const { return m_arrShaderConstInfo; }

    bool AddAutoShaderConstantInfo(const String &szParamName);
    inline const CArray<IdString>& GetAutoShaderConstantInfo() const { return m_arrAutoShaderConstInfo; }

    ldword Compile();

    inline ShaderObject* GetShaderObject() { return m_pShaderObj; }

private:
    CShaderRef(EShaderType Type, word Id) : m_ShaderType(Type), m_RefId(Id) {}
    ~CShaderRef();
    bool CheckParamIsValid(const CArray<String> &arrParam, OUT EShaderConstantType &Type, OUT dword &nTotalElem, OUT dword &nElemCount) const;
    void GetShaderConstantTypeAndCount(const String &szTypeName, OUT EShaderConstantType &Type, OUT dword &nElemCount) const;

    word m_RefId = 0xFFFF;
    EShaderType m_ShaderType;
    ShaderObject *m_pShaderObj = nullptr;
    CArray<ShaderConstantBuffer> m_arrShaderConstInfo;
    CArray<IdString> m_arrAutoShaderConstInfo;
};

class DLL_EXPORT CPass
{
public:
    friend class CMaterial;

    CShaderRef* CreateShaderRef(EShaderType eShaderType, const String &szShaderName);
    void AddTextureSlot(const String &szTextureName);

    inline ldword GetHashId() const { return m_nHashId; }
    inline CShaderRef* GetVertexShaderRef() { return m_pVertexShaderRef; }
    inline CShaderRef* GetPixelShaderRef() { return m_pPixelShaderRef; }

private:
    CPass();
    CPass(const String &szName);
    ~CPass();

    ldword Compile();
    inline void Compiled() { m_Compiled = true; }
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
    inline const CArray<CPass*>& GetPasses() const { return m_Passes; }

    const String& GetName() const { return m_szReadableName; }

    bool Compile();
    bool IsCompiled() const;

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

    CReferencedPointer<CMaterial> GetDefaultMaterial() const { return m_DefaultMtlPtr; }

    bool IsCompiled() const;

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