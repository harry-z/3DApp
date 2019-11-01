#include "Material.h"
#include "Shader.h"
#include "RenderItem.h"

#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT "Float"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT2 "Float2"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT3 "Float3"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT4 "Float4"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT "Int"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT2 "Int2"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT3 "Int3"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT4 "Int4"

constexpr ldword g_ShaderRefMask = 0x000000000000FFFF;
constexpr ldword g_InvalidId = 0xFFFFFFFFFFFFFFFF;

CShaderRef::~CShaderRef()
{
	if (m_pShaderObj != nullptr)
		ShaderObject::DestroyShaderObject(m_pShaderObj);
}

bool CShaderRef::AddShaderConstantInfo(const CArray<String> &arrParam)
{
	EShaderConstantType Type;
	dword nElemCount, nTotalElem;
	if (!CheckParamIsValid(arrParam, Type, nTotalElem, nElemCount))
		return false;
	dword nRegisterCount = nTotalElem / nElemCount;
	dword nSkip = 4 - nElemCount;
	switch (Type)
	{
		case EShaderConstantType::EShaderConstantType_Float:
		{
			byte *pConstantInfoData = (byte *)MEMALLOC(sizeof(float) * 4 * nRegisterCount); 
			memset(pConstantInfoData, 0, sizeof(float) * 4 * nRegisterCount);
			float *pTypedDest = (float *)pConstantInfoData;
			for (dword i = 0; i < nRegisterCount; ++i, pTypedDest += nSkip)
			{
				for (dword j = 0; j < nElemCount; ++j)
					*pTypedDest++ = atof(arrParam[2 + i * nElemCount + j].c_str());
			}

			ShaderConstantBuffer ConstantBuffer;
			ConstantBuffer.m_Info.m_Name = IdString(arrParam[0]);
			ConstantBuffer.m_Info.m_RegisterCount = nRegisterCount;
			ConstantBuffer.m_Info.m_Type = EShaderConstantType::EShaderConstantType_Float;
			ConstantBuffer.m_pData = pConstantInfoData;
			m_arrShaderConstInfo.Add(ConstantBuffer);
			break;
		}
		case EShaderConstantType::EShaderConstantType_Int:
		{
			byte *pConstantInfoData = (byte *)MEMALLOC(sizeof(int) * 4 * nRegisterCount); 
			memset(pConstantInfoData, 0, sizeof(int) * 4 * nRegisterCount);
			int *pTypedDest = (int *)pConstantInfoData;
			for (dword i = 0; i < nRegisterCount; ++i, pTypedDest += nSkip)
			{
				for (dword j = 0; j < nElemCount; ++j)
					*pTypedDest++ = atoi(arrParam[2 + i * nElemCount + j].c_str());
			}

			ShaderConstantBuffer ConstantBuffer;
			ConstantBuffer.m_Info.m_Name = IdString(arrParam[0]);
			ConstantBuffer.m_Info.m_RegisterCount = nRegisterCount;
			ConstantBuffer.m_Info.m_Type = EShaderConstantType::EShaderConstantType_Int;
			ConstantBuffer.m_pData = pConstantInfoData;
			m_arrShaderConstInfo.Add(ConstantBuffer);
			break;	
		}
		default:
			return false;
	}
	return true;
}

ldword CShaderRef::Compile()
{
	CShaderManager * __restrict pShaderMgr = Global::m_pShaderManager;
	CShader *pShader = nullptr;
	if (m_RefId != 0)
		pShader = pShaderMgr->FindShaderById(m_RefId);
	if (pShader == nullptr)
	{
		switch (m_ShaderType)
		{
			case EShaderType::EShaderType_Vertex:
				pShader = pShaderMgr->GetDefaultVertexShader();
				AddAutoShaderConstantInfo(WORLD_VIEW_PROJECTION_MATRIX);
				break;
			case EShaderType::EShaderType_Pixel:
				pShader = pShaderMgr->GetDefaultPixelShader();
				break;
		}
	}

	assert(pShader != nullptr);

	m_pShaderObj = ShaderObject::CreateShaderObject();

	const CArray<ShaderConstantBuffer> &arrShaderConstBuffer = GetShaderConstantBuffer();
	m_pShaderObj->m_arrShaderVar.Reserve(arrShaderConstBuffer.Num());
	for (const auto &ConstantBuffer : arrShaderConstBuffer)
	{
		ShaderVariable *pShaderVar = m_pShaderObj->m_arrShaderVar.AddIndex(1);
		pShaderVar->m_Type = ConstantBuffer.m_Info.m_Type;
		pShaderVar->m_nStartRegister = pShader->GetConstantIndexByName(ConstantBuffer.m_Info.m_Name);
		pShaderVar->m_nUsedRegister = ConstantBuffer.m_Info.m_RegisterCount;
		pShaderVar->m_pData = ConstantBuffer.m_pData;
	}

	m_pShaderObj->m_arrAutoShaderVar = GetAutoShaderConstantInfo();

	return g_ShaderRefMask & m_RefId;
}

bool CShaderRef::AddAutoShaderConstantInfo(const String &szParamName)
{
	// IdString idStr(szParamName);
	// if (Global::m_pShaderManager->IsAutoUpdatedShaderConstant(idStr))
	// {
	// 	m_arrAutoShaderConstInfo.Emplace(idStr);
	// 	return true;
	// }
	// else
	// 	return false;

	m_arrAutoShaderConstInfo.Emplace(szParamName);
	return true;
}

bool CShaderRef::CheckParamIsValid(const CArray<String> &arrParam, OUT EShaderConstantType &Type, OUT dword &nTotalElem, OUT dword &nElemCount) const
{
	Type = EShaderConstantType::EShaderConstantType_Unknown;
	nTotalElem = 0;
	nElemCount = 0;

	if (arrParam.Num() <= 2)
		return false;
	GetShaderConstantTypeAndCount(arrParam[1], Type, nElemCount);
	if (Type == EShaderConstantType::EShaderConstantType_Unknown || nElemCount == 0)
		return false;
	nTotalElem = arrParam.Num() - 2;
	return nTotalElem % nElemCount == 0;
}

void CShaderRef::GetShaderConstantTypeAndCount(const String &szTypeName, OUT EShaderConstantType &Type, OUT dword &nElemCount) const
{
	Type = EShaderConstantType::EShaderConstantType_Unknown;
	nElemCount = 0;
	if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT2) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Float;
		nElemCount = 2;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT3) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Float;
		nElemCount = 3;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT4) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Float;
		nElemCount = 4;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Float;
		nElemCount = 1;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT2) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Int;
		nElemCount = 2;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT3) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Int;
		nElemCount = 3;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT4) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Int;
		nElemCount = 4;
	}
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT) != String::npos)
	{
		Type = EShaderConstantType::EShaderConstantType_Int;
		nElemCount = 1;
	}
}

CPass::CPass() 
{
	m_Compiled = false;
	m_nHashId = g_InvalidId;
}

CPass::CPass(const String &szName)
: m_IdStr(szName)
{
	m_Compiled = false;
	m_nHashId = g_InvalidId;
}

CPass::~CPass()
{
	if (m_pVertexShaderRef != nullptr)
	{
		DELETE_TYPE(m_pVertexShaderRef, CShaderRef);
		m_pVertexShaderRef = nullptr;
	}
	if (m_pPixelShaderRef != nullptr)
	{
		DELETE_TYPE(m_pPixelShaderRef, CShaderRef);
		m_pPixelShaderRef = nullptr;
	}
}

ldword CPass::Compile()
{
	ldword nVSCompile = m_pVertexShaderRef->Compile();
	ldword nPSCompile = m_pPixelShaderRef->Compile();
	m_nHashId = SortVal(nVSCompile, nPSCompile);
	Compiled();
	return m_nHashId;
}

CShaderRef* CPass::CreateShaderRef(EShaderType eShaderType, const String &szShaderName)
{
	CShader *pShader = Global::m_pShaderManager->FindShaderByName(szShaderName);
	if (pShader != nullptr)
	{
		switch (eShaderType)
		{
			case EShaderType::EShaderType_Vertex:
				m_pVertexShaderRef = NEW_TYPE(CShaderRef)(EShaderType::EShaderType_Vertex, pShader->GetId());
				return m_pVertexShaderRef;
			case EShaderType::EShaderType_Pixel:
				m_pPixelShaderRef = NEW_TYPE(CShaderRef)(EShaderType::EShaderType_Pixel, pShader->GetId());
				return m_pPixelShaderRef;
			default:
				return nullptr;
		}
	}
	else
	{
		switch (eShaderType)
		{
			case EShaderType::EShaderType_Vertex:
				m_pVertexShaderRef = NEW_TYPE(CShaderRef)(EShaderType::EShaderType_Vertex, 0);
				return m_pVertexShaderRef;
			case EShaderType::EShaderType_Pixel:
				m_pPixelShaderRef = NEW_TYPE(CShaderRef)(EShaderType::EShaderType_Pixel, 0);
				return m_pPixelShaderRef;
			default:
				return nullptr;
		}
	}
}

void CPass::AddTextureSlot(const String &szTextureName)
{
	m_arrTexture.Emplace(szTextureName);
}

CPass* CMaterial::CreatePass()
{
	CPass *NewPass = NEW_TYPE(CPass);
	m_Passes.Add(NewPass);
	return NewPass;	
}

CPass* CMaterial::CreatePass(const String &szName)
{
	CPass *NewPass = NEW_TYPE(CPass)(szName);
	m_Passes.Add(NewPass);
	return NewPass;
}

bool CMaterial::Compile()
{
	for (auto &Pass : m_Passes)
	{
		if (Pass->Compile() == g_InvalidId)
			return false;
	}
	return true;
}

bool CMaterial::IsCompiled() const
{
	if (m_Passes.Num() == 0)
		return false;
	
	for (const auto &Pass : m_Passes)
	{
		if (!Pass->IsCompiled())
			return false;
	}
	return true;
}

CMaterial::~CMaterial()
{
	for (auto &Pass : m_Passes)
	{
		DELETE_TYPE(Pass, CPass);
	}
}

void CMaterial::Load(const String &szFilePath)
{
	CMaterialParser MtlParser;
	MtlParser.m_pCurrentMaterial = this;
	CScriptParser ScriptParser;
	ScriptParser.AddScriptParserListener(&MtlParser);
	bool bParseSucceeded = ScriptParser.ParseFromFile(szFilePath.c_str());
	m_CreatedOrLoaded = bParseSucceeded && (!MtlParser.IsError());
}

void CMaterial::Destroy()
{
	Global::m_pMaterialManager->DestroyMaterial(this);
}


CMaterialManager::CMaterialManager()
: m_MaterialId(1)
{
	m_MaterialPool.Initialize(sizeof(CMaterial));
}

CMaterialManager::~CMaterialManager()
{

}

void CMaterialManager::Initialize()
{
	m_DefaultMtlPtr = CreateMaterial("DefaultMtl");
	CPass *pPass = m_DefaultMtlPtr->CreatePass();
	pPass->CreateShaderRef(EShaderType::EShaderType_Vertex, "");
	pPass->CreateShaderRef(EShaderType::EShaderType_Pixel, "");
}

CMaterial* CMaterialManager::CreateMaterial(const String &szName)
{
	CMaterial *pNewMaterial = CreateInstance(szName);
	pNewMaterial->CreatedOrLoaded();
	return pNewMaterial;
}

CMaterial* CMaterialManager::LoadMaterial(const String &szFilePath, bool bBackground)
{
	CMaterial *pNewMaterial = CreateInstance(szFilePath);
    pNewMaterial->Load(szFilePath);
    return pNewMaterial;
}

CMaterial* CMaterialManager::FindMaterial(const String &szName) 
{
	IdString idStr(szName);
	return FindMaterial(idStr);
}

CMaterial* CMaterialManager::FindMaterial(const IdString &idStr) {
	std::lock_guard<std::mutex> l(m_MaterialMapLock);
	MaterialMap::_MyIterType iter = m_MaterialMap.Find(idStr);
	return (iter) ? iter.Value() : nullptr;
}

void CMaterialManager::DestroyMaterial(CMaterial *pMaterial)
{
	if (pMaterial->CheckRefCount())
		return;

	bool bHas;
	{
		std::lock_guard<std::mutex> l(m_MaterialMapLock);
		bHas = (bool)m_MaterialMap.Find(pMaterial->m_IdStr);
		if (bHas)
			m_MaterialMap.Remove(pMaterial->m_IdStr);
	}

	if (bHas) {
		pMaterial->~CMaterial();
		m_MaterialPool.Free_mt(pMaterial);
	}
}

CMaterial* CMaterialManager::CreateInstance(const IdString &idStr)
{
	std::lock_guard<std::mutex> l(m_MaterialMapLock);
	MaterialMap::_MyIterType Iter = m_MaterialMap.Find(idStr);
	if (Iter)
		return Iter.Value();

	CMaterial *pNewMaterial = new (m_MaterialPool.Allocate_mt()) CMaterial;
	pNewMaterial->m_IdStr = idStr;
	pNewMaterial->m_Id = m_MaterialId++;
	m_MaterialMap.Insert(idStr, pNewMaterial);
	
	return pNewMaterial;
}