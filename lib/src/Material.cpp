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

CShaderRef::CShaderRef(EShaderType Type, word Id)
: m_ShaderType(Type), m_RefId(Id) 
{
	m_pShaderObj = ShaderObject::CreateShaderObject();
	m_Compiled = false;
}

CShaderRef::~CShaderRef()
{
	ShaderObject::DestroyShaderObject(m_pShaderObj);
}

bool CShaderRef::AddShaderConstantInfo(const CArray<String> &arrParam)
{
	EShaderConstantType Type;
	dword nElemCount, nTotalElem;
	if (!CheckParamIsValid(arrParam, Type, nTotalElem))
		return false;
	switch (Type)
	{
		case EShaderConstantType::EShaderConstantType_Float:
		{
			byte *pConstantInfoData = (byte *)MEMALLOC(sizeof(float) * nTotalElem); 
			memset(pConstantInfoData, 0, sizeof(float) * nTotalElem);
			float *pTypedDest = (float *)pConstantInfoData;
			for (dword i = 0; i < nTotalElem; ++i)
				*pTypedDest++ = atof(arrParam[2 + i].c_str());

			ShaderUniformBuffer *pConstantBuffer = m_arrShaderConstInfo.AddIndex(1);
			new (pConstantBuffer) ShaderUniformBuffer;
			pConstantBuffer->m_Info.m_Name = IdString(arrParam[0]);
			pConstantBuffer->m_Info.m_nLengthInBytes = sizeof(float) * nTotalElem;
			pConstantBuffer->m_Info.m_Type = EShaderConstantType::EShaderConstantType_Float;
			pConstantBuffer->m_pData = pConstantInfoData;
			break;
		}
		case EShaderConstantType::EShaderConstantType_Int:
		{
			byte *pConstantInfoData = (byte *)MEMALLOC(sizeof(int) * nTotalElem); 
			memset(pConstantInfoData, 0, sizeof(int) * nTotalElem);
			int *pTypedDest = (int *)pConstantInfoData;
			for (dword i = 0; i < nTotalElem; ++i)
				*pTypedDest++ = atoi(arrParam[2 + i].c_str());

			ShaderUniformBuffer *pConstantBuffer = m_arrShaderConstInfo.AddIndex(1);
			new (pConstantBuffer) ShaderUniformBuffer;
			pConstantBuffer->m_Info.m_Name = IdString(arrParam[0]);
			pConstantBuffer->m_Info.m_nLengthInBytes = sizeof(int) * nTotalElem;
			pConstantBuffer->m_Info.m_Type = EShaderConstantType::EShaderConstantType_Int;
			pConstantBuffer->m_pData = pConstantInfoData;
			break;	
		}
		default:
			return false;
	}
	return true;
}

bool CShaderRef::AddAutoShaderConstantInfo(const String &szParamName)
{
	m_arrAutoShaderConstInfo.Emplace(szParamName);
	return true;
}

ldword CShaderRef::Compile()
{
	if (IsCompiled())
		return g_ShaderRefMask & m_RefId;

	CShaderManager * __restrict pShaderMgr = Global::m_pShaderManager;
	CShader *pShader = nullptr;
	if (m_RefId != 0)
		pShader = pShaderMgr->FindShaderById(m_RefId);
	if (pShader == nullptr)
	{
		m_RefId = 0;
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

	const CArray<ShaderUniformBuffer> &arrShaderConstBuffer = GetShaderConstantBuffer();
	m_pShaderObj->m_arrShaderVar.Reserve(arrShaderConstBuffer.Num());
	for (const auto &ConstantBuffer : arrShaderConstBuffer)
	{
		ShaderVariable *pShaderVar = m_pShaderObj->m_arrShaderVar.AddIndex(1);
		const ShaderUniformInfo &UniformInfo = pShader->GetUniformInfoByName(ConstantBuffer.m_Info.m_Name);
		assert(ConstantBuffer.m_Info.m_Type == UniformInfo.m_ParamInfo.m_Type && 
			ConstantBuffer.m_Info.m_nLengthInBytes == UniformInfo.m_ParamInfo.m_nLengthInBytes);
		pShaderVar->m_Type = UniformInfo.m_ParamInfo.m_Type;
		pShaderVar->m_nLengthInBytes = UniformInfo.m_ParamInfo.m_nLengthInBytes;
		pShaderVar->m_nRegisterIndex = UniformInfo.m_nRegisterIndex;
		pShaderVar->m_nOffsetInBytes = UniformInfo.m_nOffsetInBytes;
		pShaderVar->m_pData = ConstantBuffer.m_pData;
	}

	m_pShaderObj->m_arrAutoShaderVar = GetAutoShaderConstantInfo();

	Compiled();

	return g_ShaderRefMask & m_RefId;
}

bool CShaderRef::CheckParamIsValid(const CArray<String> &arrParam, OUT EShaderConstantType &Type, OUT dword &nTotalElem) const
{
	Type = EShaderConstantType::EShaderConstantType_Unknown;
	nTotalElem = 0;
	dword nElemCount = 0;

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

CPool ShaderResources::m_ShaderResourcePool;

CPass::CPass() 
{
	m_Compiled = false;
	m_nHashId = g_InvalidId;
	m_pShaderResources = ShaderResources::CreateShaderResource();
}

CPass::CPass(const String &szName)
: m_IdStr(szName)
{
	m_Compiled = false;
	m_nHashId = g_InvalidId;
	m_pShaderResources = ShaderResources::CreateShaderResource();
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

	ShaderResources::DestroyShaderResource(m_pShaderResources);
}

ldword CPass::Compile()
{
	if (IsCompiled())
		return m_nHashId;

	ldword nVSCompile = 0;
	if (m_pVertexShaderRef != nullptr)
		nVSCompile = m_pVertexShaderRef->Compile();
	else
		return g_InvalidId;

	ldword nPSCompile = 0;
	if (m_pPixelShaderRef != nullptr)
		nPSCompile = m_pPixelShaderRef->Compile();
	else
		return g_InvalidId;

	ldword nSRCompile = 0;
	if (m_pShaderResources->IsValid())
	{
		bool bAllTextureLoaded = true;
		for (const auto &Texture : m_pShaderResources->m_arrTexture)
		{
			if (!Texture->IsCreatedOrLoaded())
			{
				bAllTextureLoaded = false;
				break;
			}
		}

		if (bAllTextureLoaded)
			nSRCompile = m_pShaderResources->m_arrTexture[0]->GetID();
		else
			return g_InvalidId;
	} 

	m_nHashId = SortVal(nVSCompile, nPSCompile, nSRCompile);
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

void CPass::LoadTextureSlot(const String &szTextureName, EAutoGenmip bAutoGenMipmap, bool bGamma)
{
	CTexture *pTexture = Global::m_pTextureManager->LoadTexture(szTextureName, bAutoGenMipmap, bGamma);
	m_pShaderResources->m_arrTexture.Emplace(pTexture);
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
	if (IsCompiled())
		return true;

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