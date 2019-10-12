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
	if (!CheckParamIsValid(arrParam))
		return false;
	EShaderConstantType Type = GetShaderConstantType(arrParam[1]);
	dword nElemCount = GetShaderConstantElementCount(Type);
	dword nTotalElem = arrParam.Num() - 2;
	switch (Type)
	{
		case EShaderConstantType::EShaderConstantType_Float:
		case EShaderConstantType::EShaderConstantType_Float2:
		case EShaderConstantType::EShaderConstantType_Float3:
		case EShaderConstantType::EShaderConstantType_Float4:
		{
			float *pFloat = (float *)MEMALLOC(sizeof(float) * nTotalElem);
			for (dword i = 0; i < nTotalElem; ++i)
			{
				pFloat[i] = atof(arrParam[i + 2].c_str());
			}
			AddShaderConstantInfo(arrParam[0], Type, nTotalElem / nElemCount, pFloat);
			MEMFREE(pFloat);
			break;
		}
		case EShaderConstantType::EShaderConstantType_Int:
		case EShaderConstantType::EShaderConstantType_Int2:
		case EShaderConstantType::EShaderConstantType_Int3:
		case EShaderConstantType::EShaderConstantType_Int4:
		{
			int *pInt = (int *)MEMALLOC(sizeof(int) * nTotalElem);
			for (dword i = 0; i < nTotalElem; ++i)
			{
				pInt[i] = atoi(arrParam[i + 2].c_str());
			}
			AddShaderConstantInfo(arrParam[0], Type, nTotalElem / nElemCount, pInt);
			MEMFREE(pInt);
			break;	
		}
	}
	return true;
}

ldword CShaderRef::Compile()
{
	CShaderManager * __restrict pShaderMgr = Global::m_pShaderManager;
	CShader *pShader = nullptr;
	ldword nId = m_RefId;
	if (nId != 0xFFFF)
		pShader = pShaderMgr->FindShaderById(nId);
	if (pShader == nullptr)
	{
		nId = 0;
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
	const CArray<ShaderConstantInfo> &arrShaderConstInfo = GetShaderConstantInfo();
	const CArray<IdString> &arrAutoShaderConstInfo = GetAutoShaderConstantInfo();
	m_pShaderObj->m_arrShaderVar.Reserve(arrShaderConstInfo.Num() + arrAutoShaderConstInfo.Num());
	for (const auto &ConstantInfo : arrShaderConstInfo)
	{
		ShaderVariable *pShaderVar = m_pShaderObj->m_arrShaderVar.AddIndex(1);
		pShaderVar->m_Type = ConstantInfo.m_Type;
		pShaderVar->m_nStartRegister = pShader->GetConstantIndexByName(ConstantInfo.m_Name);
		pShaderVar->m_nUsedRegister = ConstantInfo.m_RegisterCount;
		pShaderVar->m_pData = ConstantInfo.m_pData;
	}
	for (const auto &AutoConstantInfo : arrAutoShaderConstInfo)
	{
		const ShaderConstantInfo *pAutoConstantInfo = pShaderMgr->FindShaderConstantInfo(AutoConstantInfo);
		if (pAutoConstantInfo != nullptr)
		{
			ShaderVariable *pShaderVar = m_pShaderObj->m_arrShaderVar.AddIndex(1);
			pShaderVar->m_Type = pAutoConstantInfo->m_Type;
			pShaderVar->m_nStartRegister = pShader->GetConstantIndexByName(pAutoConstantInfo->m_Name);
			pShaderVar->m_nUsedRegister = pAutoConstantInfo->m_RegisterCount;
			pShaderVar->m_pData = pAutoConstantInfo->m_pData;
		}
	}

	return g_ShaderRefMask | nId;
}

bool CShaderRef::AddAutoShaderConstantInfo(const String &szParamName)
{
	IdString idStr(szParamName);
	if (Global::m_pShaderManager->IsAutoShaderConstant(idStr))
	{
		m_arrAutoShaderConstInfo.Emplace(idStr);
		return true;
	}
	else
		return false;
}

bool CShaderRef::CheckParamIsValid(const CArray<String> &arrParam) const
{
	if (arrParam.Num() <= 2)
		return false;
	EShaderConstantType Type = GetShaderConstantType(arrParam[1]);
	if (Type == EShaderConstantType::EShaderConstantType_Unknown)
		return false;
	dword nElemCount = GetShaderConstantElementCount(Type);
	dword nTotalElem = arrParam.Num() - 2;
	return nTotalElem % nElemCount == 0;
}

EShaderConstantType CShaderRef::GetShaderConstantType(const String &szTypeName) const
{
	if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT2) != String::npos)
		return EShaderConstantType::EShaderConstantType_Float2;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT3) != String::npos)
		return EShaderConstantType::EShaderConstantType_Float3;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT4) != String::npos)
		return EShaderConstantType::EShaderConstantType_Float4;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_FLOAT) != String::npos)
		return EShaderConstantType::EShaderConstantType_Float;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT2) != String::npos)
		return EShaderConstantType::EShaderConstantType_Int2;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT3) != String::npos)
		return EShaderConstantType::EShaderConstantType_Int3;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT4) != String::npos)
		return EShaderConstantType::EShaderConstantType_Int4;
	else if (szTypeName.find(SHADER_CONSTANT_TYPE_KEYWORD_INT) != String::npos)
		return EShaderConstantType::EShaderConstantType_Int;
	else
		return EShaderConstantType::EShaderConstantType_Unknown;
}

dword CShaderRef::GetShaderConstantElementCount(EShaderConstantType ConstType) const
{
	switch (ConstType)
	{
		case EShaderConstantType::EShaderConstantType_Float:
		case EShaderConstantType::EShaderConstantType_Int:
			return 1;
		case EShaderConstantType::EShaderConstantType_Int2:
		case EShaderConstantType::EShaderConstantType_Float2:
			return 2;
		case EShaderConstantType::EShaderConstantType_Int3:
		case EShaderConstantType::EShaderConstantType_Float3:
			return 3;
		case EShaderConstantType::EShaderConstantType_Int4:
		case EShaderConstantType::EShaderConstantType_Float4:
			return 4;
		default:
			return 0;
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
	if (m_pVertexShaderRef == nullptr)
		m_pVertexShaderRef = CreateShaderRef(EShaderType::EShaderType_Vertex, "VS_Default");
	ldword nVSCompile = m_pVertexShaderRef->Compile();
	if (m_pPixelShaderRef == nullptr)
		m_pPixelShaderRef = CreateShaderRef(EShaderType::EShaderType_Pixel, "PS_Default");
	ldword nPSCompile = m_pPixelShaderRef->Compile();
	m_nHashId = nVSCompile << 52 | nPSCompile << 40;
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
		return nullptr;
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
	pPass->CreateShaderRef(EShaderType::EShaderType_Vertex, )
}

CMaterial* CMaterialManager::CreateMaterial(const String &szName)
{
	CMaterial *pNewMaterial = CreateInstance(szName);
	pMaterial->CreatedOrLoaded();
	return pMaterial;
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
		bHas = m_MaterialMap.Find(pMaterial->m_IdStr);
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