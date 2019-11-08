#include "Material.h"

#define TOKEN_MATERIAL "Material"
#define TOKEN_PASS "Pass"
#define TOKEN_TEXTURE "Texture"
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
	else if (szParamType == TOKEN_PASS && m_pCurrentMaterial != nullptr)
	{
		if (arrParam.Num() > 0)
			m_pCurrentPass = m_pCurrentMaterial->CreatePass(arrParam[0]);
		else
			m_pCurrentPass = m_pCurrentMaterial->CreatePass();
	}
	else if (szParamType == TOKEN_TEXTURE && m_pCurrentPass != nullptr)
	{
		if (arrParam.Num() > 0)
		{
			const String &szFileName = arrParam[0];

			EAutoGenmip AutoGenmip = EAutoGenmip::EAutoGenmip_AUTO;
			if (arrParam.Num() > 1)
			{
				if (arrParam[1] == "Enable")
					AutoGenmip = EAutoGenmip::EAutoGenmip_ENABLE;
				else if (arrParam[1] == "Disable")
					AutoGenmip = EAutoGenmip::EAutoGenmip_DISABLE;
			}
			
			bool bGamma = false;
			if (arrParam.Num() > 2)
				bGamma = arrParam[2] == "Gamma";
			
			m_pCurrentPass->LoadTextureSlot(szFileName, AutoGenmip, bGamma);
		}
		else
			Error();
	}
	else if (szParamType == TOKEN_VS && m_pCurrentPass != nullptr && arrParam.Num() > 0)
	{
		m_pCurrentShaderRef = m_pCurrentPass->CreateShaderRef(EShaderType::EShaderType_Vertex, arrParam[0]);
		assert(m_pCurrentShaderRef != nullptr);
	}
	else if (szParamType == TOKEN_PS && m_pCurrentPass != nullptr && arrParam.Num() > 0)
	{
		m_pCurrentShaderRef = m_pCurrentPass->CreateShaderRef(EShaderType::EShaderType_Pixel, arrParam[0]);
		assert(m_pCurrentShaderRef != nullptr);
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