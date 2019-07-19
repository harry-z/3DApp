#pragma once
#include "Job.h"
#include "Resource.h"
#include "ScriptParser.h"

class DLL_EXPORT CMaterial final : public CBaseResource, public IScriptParserListener {
public:
    friend class CMaterialManager;

protected:
    CMaterial() {}
    virtual ~CMaterial() {}

    void Load(const String &szFilePath);

    virtual void OnProcessChunkTitle(const String &szChunkType, const String &szChunkParam) override {}
    virtual void OnProcessParam(const String &szParamType, const String &szParam1, const String &szParam2) override {}

    virtual void Destroy() override {}

protected:
    dword m_Id;
    IdString m_IdStr;
};

class DLL_EXPORT CMaterialManager {
public:
    CMaterialManager();
    ~CMaterialManager();

    CMaterial* LoadMaterial(const String &szFilePath, bool bBackground = false);

private:
	typedef CHashmap<IdString, CMaterial*> MaterialMap;
	MaterialMap m_MaterialMap;
    CPool m_MaterialPool;
	std::mutex m_MaterialMapLock;
	dword m_MaterialId;
};