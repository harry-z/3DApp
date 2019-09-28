#include "Material.h"

#define TOKEN_MATERIAL "Material"
#define TOKEN_TECHNIQUE "Technique"
#define TOKEN_PASS "Pass"
#define TOKEN_VS "VertexShader"
#define TOKEN_PS "PixelShader"
#define TOKEN_SHADER_PARAM "ShaderParam"
#define TOKEN_AUTO_SHADER_PARAM "AutoShaderParam"

void CMaterialParser::OnProcessNode(const String &szParamType, const CArray<String> &arrParam)
{
	if (IsError())
		return;

	if (szParamType == TOKEN_MATERIAL && m_pCurrentMaterial != nullptr)
	{
		if (arrParam.Num() > 0)
			m_pCurrentMaterial->SetName(arrParam[0]);
		else
			Error();
	}
	else if (szParamType == TOKEN_TECHNIQUE && m_pCurrentMaterial != nullptr)
	{
		if (arrParam.Num() > 0)
			m_pCurrentTechnique = m_pCurrentMaterial->CreateTechnique(arrParam[0]);
		else
			m_pCurrentTechnique = m_pCurrentMaterial->CreateTechnique();
	}
	else if (szParamType == TOKEN_PASS && m_pCurrentTechnique != nullptr)
	{
		if (arrParam.Num() > 0)
			m_pCurrentPass = m_pCurrentTechnique->CreatePass(arrParam[0]);
		else
			m_pCurrentPass = m_pCurrentTechnique->CreatePass();
	}
	else if (szParamType == TOKEN_VS && m_pCurrentPass != nullptr && arrParam.Num() > 0)
	{
		m_pCurrentShaderRef = m_pCurrentPass->CreateShaderRef(EShaderType::EShaderType_Vertex, arrParam[0]);
		if (m_pCurrentShaderRef == nullptr)
			Error();
	}
	else if (szParamType == TOKEN_PS && m_pCurrentPass != nullptr && arrParam.Num() > 0)
	{
		m_pCurrentShaderRef = m_pCurrentPass->CreateShaderRef(EShaderType::EShaderType_Pixel, arrParam[0]);
		if (m_pCurrentShaderRef == nullptr)
			Error();
	}
	else if (szParamType == TOKEN_SHADER_PARAM && m_pCurrentShaderRef != nullptr)
	{
		if (!m_pCurrentShaderRef->AddShaderConstantInfo(arrParam))
			Error();
	}
	else if (szParamType == TOKEN_AUTO_SHADER_PARAM && m_pCurrentShaderRef != nullptr && arrParam.Num() > 0)
	{
		if (!m_pCurrentShaderRef->AddAutoShaderConstantInfo(arrParam[0]))
			Error();
	}
	else
		Error();
}