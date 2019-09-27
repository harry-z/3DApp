#include "Material.h"
#include "Shader.h"

#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT "Float"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT2 "Float2"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT3 "Float3"
#define SHADER_CONSTANT_TYPE_KEYWORD_FLOAT4 "Float4"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT "Int"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT2 "Int2"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT3 "Int3"
#define SHADER_CONSTANT_TYPE_KEYWORD_INT4 "Int4"

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

CPass::CPass() {}

CPass::CPass(const String &szName)
: m_IdStr(szName)
{}

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

CShaderRef* CPass::CreateShaderRef(EShaderType eShaderType, const String &szShaderName)
{
	CShader *pShader = Global::m_pShaderManager->FindShaderByName(szShaderName);
	if (pShader != nullptr)
	{
		switch (eShaderType)
		{
			case EShaderType::EShaderType_Vertex:
				m_pVertexShaderRef = NEW_TYPE(CShaderRef)(pShader->GetId());
				return m_pVertexShaderRef;
			case EShaderType::EShaderType_Pixel:
				m_pPixelShaderRef = NEW_TYPE(CShaderRef)(pShader->GetId());
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

CPass* CTechnique::CreatePass()
{
	CPass *NewPass = NEW_TYPE(CPass);
	m_Passes.Add(NewPass);
	return NewPass;	
}

CPass* CTechnique::CreatePass(const String &szName)
{
	CPass *NewPass = NEW_TYPE(CPass)(szName);
	m_Passes.Add(NewPass);
	return NewPass;
}

CTechnique::CTechnique() {}

CTechnique::CTechnique(const String &szName)
: m_IdStr(szName)
{}

CTechnique::~CTechnique()
{
	for (auto &Pass : m_Passes)
	{
		DELETE_TYPE(Pass, CPass);
	}
}

CTechnique* CMaterial::CreateTechnique()
{
	CTechnique *NewTechnique = NEW_TYPE(CTechnique);
	m_Techniques.Add(NewTechnique);
	return NewTechnique;	
}

CTechnique* CMaterial::CreateTechnique(const String &szName)
{
	CTechnique *NewTechnique = NEW_TYPE(CTechnique)(szName);
	m_Techniques.Add(NewTechnique);
	return NewTechnique;
}

CMaterial::~CMaterial()
{
	for (auto &Technique : m_Techniques)
	{
		DELETE_TYPE(Technique, CTechnique);
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



CMaterialManager::CMaterialManager()
: m_MaterialId(1)
{
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
		MaterialMap::_MyIterType Iter = m_MaterialMap.Find(idStr);
		if (Iter)
			return Iter.Value();

		pNewMaterial = new (m_MaterialPool.Allocate_mt()) CMaterial;
		pNewMaterial->m_IdStr = idStr;
		pNewMaterial->m_Id = m_MaterialId++;
		m_MaterialMap.Insert(idStr, pNewMaterial);
	}

    pNewMaterial->Load(szFilePath);
    return pNewMaterial;
}