#include "Material.h"

void CMaterial::Load(const String &szFilePath)
{
    CScriptParser Parser;
    Parser.AddScriptParserListener(this);

    Parser.ParseFromFile(szFilePath.c_str());
}

CMaterialManager::CMaterialManager()
: m_MaterialId(1)
{
	m_MaterialMap.SetSlotCount(100);
	m_MaterialPool.Initialize(sizeof(CMaterial));
}

CMaterialManager::~CMaterialManager()
{

}

CMaterial* CMaterialManager::LoadMaterial(const String &szFilePath, bool bBackground)
{
	IdString idStr(szFilePath);
	
	CMaterial *pNewMaterial = nullptr;
	{
		std::lock_guard<std::mutex> l(m_MaterialMapLock);
		MaterialMap::_ValuePointerType pMaterial = m_MaterialMap.Find(idStr);
		if (pMaterial != nullptr)
			return *pMaterial;

		pNewMaterial = new (m_MaterialPool.Allocate_mt()) CMaterial;
		pNewMaterial->m_IdStr = idStr;
		pNewMaterial->m_Id = m_MaterialId++;
		m_MaterialMap.Insert(idStr, pNewMaterial);
	}

    pNewMaterial->Load(szFilePath);
    return pNewMaterial;
}