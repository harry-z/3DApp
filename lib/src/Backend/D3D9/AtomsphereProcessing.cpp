#include "AtomsphereRendererDX9.h"

bool CAtomsphereRendererDX9::ProcessTransmittanceTexture(const String &szShaderHeader, const AtmosphereParams &params)
{
    String szTransmittanceShader = szShaderHeader + TransmittanceShader();

    ID3DXBuffer *pTransmittanceBufferCode, *pErrorCode;
    HRESULT hr = D3DXCompileShader(szTransmittanceShader.c_str(), szTransmittanceShader.length(), nullptr, nullptr, "ComputeTransmittanceToTopAtmosphereBoundaryTexture", "ps_3_0", 0, &pTransmittanceBufferCode, &pErrorCode, &m_pTransmittanceShaderCode);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreatePixelShader((const DWORD*)pTransmittanceBufferCode->GetBufferPointer(), &m_pTransmittanceShader);
    SAFE_RELEASE(pTransmittanceBufferCode);
    CHECK_ERROR(hr)

    dword nTextureDimIndex = GetConstantIndex(m_pTransmittanceShaderCode, "TextureDim");
    CHECK_INDEX_ERROR(nTextureDimIndex)
    dword nTopBottomRadius = GetConstantIndex(m_pTransmittanceShaderCode, "TopBottomRadius");
    CHECK_INDEX_ERROR(nTopBottomRadius)
    dword nRayleighScattering = GetConstantIndex(m_pTransmittanceShaderCode, "RayleighScattering");
    CHECK_INDEX_ERROR(nRayleighScattering)
    dword nMieExtinction = GetConstantIndex(m_pTransmittanceShaderCode, "MieExtinction");
    CHECK_INDEX_ERROR(nMieExtinction)
    dword nAbsorptionExtinction = GetConstantIndex(m_pTransmittanceShaderCode, "AbsorptionExtinction");
    CHECK_INDEX_ERROR(nAbsorptionExtinction)
    dword nRMAExpTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMAExpTerm");
    CHECK_INDEX_ERROR(nRMAExpTerm)
    dword nRMAExpScale = GetConstantIndex(m_pTransmittanceShaderCode, "RMAExpScale");
    CHECK_INDEX_ERROR(nRMAExpScale)
    dword nRMALinearTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMALinearTerm");
    CHECK_INDEX_ERROR(nRMALinearTerm)
    dword nRMAConstTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMAConstTerm");
    CHECK_INDEX_ERROR(nRMAConstTerm)
    dword nRMAWidth = GetConstantIndex(m_pTransmittanceShaderCode, "RMAWidth");
    CHECK_INDEX_ERROR(nRMAWidth)

    float TextureDim[] = { TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT };
    Vec4 fTopBottomRadius((float)params.m_TopBottomRadius.x, (float)params.m_TopBottomRadius.y, (float)params.m_TopBottomRadius.z, (float)params.m_TopBottomRadius.w);
    Vec3 fRayleighScattering((float)params.m_RayleighScattering.x, (float)params.m_RayleighScattering.y, (float)params.m_RayleighScattering.z);
    Vec3 fMieExtinction((float)params.m_MieExtinction.x, (float)params.m_MieExtinction.y, (float)params.m_MieExtinction.z);
    Vec3 fAbsorptionExtinction((float)params.m_AbsorptionExtinction.x, (float)params.m_AbsorptionExtinction.y, (float)params.m_AbsorptionExtinction.z);
    float RMAExpTerm[] = {
        (float)params.m_arrRayleighDensity[0].m_ExpTerm,
        (float)params.m_arrMieDensity[0].m_ExpTerm,
        (float)params.m_arrAbsorptionDensity[0].m_ExpTerm,
        0.0f,
        (float)params.m_arrRayleighDensity[1].m_ExpTerm,
        (float)params.m_arrMieDensity[1].m_ExpTerm,
        (float)params.m_arrAbsorptionDensity[1].m_ExpTerm,
        0.0f
    };
    float RMAExpScale[] = {
        (float)params.m_arrRayleighDensity[0].m_ExpScale,
        (float)params.m_arrMieDensity[0].m_ExpScale,
        (float)params.m_arrAbsorptionDensity[0].m_ExpScale,
        0.0f,
        (float)params.m_arrRayleighDensity[1].m_ExpScale,
        (float)params.m_arrMieDensity[1].m_ExpScale,
        (float)params.m_arrAbsorptionDensity[1].m_ExpScale,
        0.0f
    };
    float RMALinearTerm[] = {
        (float)params.m_arrRayleighDensity[0].m_LinearTerm,
        (float)params.m_arrMieDensity[0].m_LinearTerm,
        (float)params.m_arrAbsorptionDensity[0].m_LinearTerm,
        0.0f,
        (float)params.m_arrRayleighDensity[1].m_LinearTerm,
        (float)params.m_arrMieDensity[1].m_LinearTerm,
        (float)params.m_arrAbsorptionDensity[1].m_LinearTerm,
        0.0f
    };
    float RMAConstTerm[] = {
        (float)params.m_arrRayleighDensity[0].m_ConstantTerm,
        (float)params.m_arrMieDensity[0].m_ConstantTerm,
        (float)params.m_arrAbsorptionDensity[0].m_ConstantTerm,
        0.0f,
        (float)params.m_arrRayleighDensity[1].m_ConstantTerm,
        (float)params.m_arrMieDensity[1].m_ConstantTerm,
        (float)params.m_arrAbsorptionDensity[1].m_ConstantTerm,
        0.0f
    };
    float RMAWidth[] = {
        (float)params.m_arrRayleighDensity[0].m_Width,
        (float)params.m_arrMieDensity[0].m_Width,
        (float)params.m_arrAbsorptionDensity[0].m_Width,
    };

    g_pDevice9->SetRenderTarget(0, m_pTransmittanceSurface);
    g_pDevice9->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
    g_pDevice9->SetPixelShader(m_pTransmittanceShader);
    g_pDevice9->SetPixelShaderConstantF(nTextureDimIndex, TextureDim, 1);
    g_pDevice9->SetPixelShaderConstantF(nTopBottomRadius, &fTopBottomRadius.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nRayleighScattering, &fRayleighScattering.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nMieExtinction, &fMieExtinction.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nAbsorptionExtinction, &fAbsorptionExtinction.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nRMAExpTerm, RMAExpTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAExpScale, RMAExpScale, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMALinearTerm, RMALinearTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAConstTerm, RMAConstTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAWidth, RMAWidth, 1);
    g_pDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    return true;
}

bool CAtomsphereRendererDX9::ProcessIrradianceTexture(const String &szShaderHeader, const AtmosphereParams &params)
{
    String szIrradianceShader = szShaderHeader + IrradianceShader();

    ID3DXBuffer *pIrradianceBufferCode, *pErrorCode;
    HRESULT hr = D3DXCompileShader(szIrradianceShader.c_str(), szIrradianceShader.length(), nullptr, nullptr, "ComputeDirectIrradianceTexture", "ps_3_0", 0, &pIrradianceBufferCode, &pErrorCode, &m_pIrradianceShaderCode);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreatePixelShader((const DWORD*)pIrradianceBufferCode->GetBufferPointer(), &m_pIrradianceShader);
    SAFE_RELEASE(pIrradianceBufferCode);
    CHECK_ERROR(hr)    

    dword nTextureDimIndex = GetConstantIndex(m_pIrradianceShaderCode, "TextureDim");
    CHECK_INDEX_ERROR(nTextureDimIndex)
    dword nTopBottomRadius = GetConstantIndex(m_pIrradianceShaderCode, "TopBottomRadius");
    CHECK_INDEX_ERROR(nTopBottomRadius)
    dword nSunAngularRadius = GetConstantIndex(m_pIrradianceShaderCode, "SunAngularRadius");
    CHECK_INDEX_ERROR(nSunAngularRadius)

    float TextureDim[] = { TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT }; 
    Vec4 fTopBottomRadius((float)params.m_TopBottomRadius.x, (float)params.m_TopBottomRadius.y, (float)params.m_TopBottomRadius.z, (float)params.m_TopBottomRadius.w);
    float fSunAngularRadius = (float)params.m_SunAngularRadius;

    g_pDevice9->SetRenderTarget(0, m_pIrradianceSurface);
    g_pDevice9->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
    g_pDevice9->SetTexture(0, m_pIrradiance);
    g_pDevice9->SetPixelShaderConstantF(nTextureDimIndex, TextureDim, 1);
    g_pDevice9->SetPixelShaderConstantF(nTopBottomRadius, &fTopBottomRadius.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nSunAngularRadius, &fSunAngularRadius, 1);
    g_pDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    return true;
}

bool CAtomsphereRendererDX9::ProcessSingleScatteringTexture(const String &szShaderHeader, const AtmosphereParams &params)
{
    String szSingleScatteringShader = szShaderHeader + SingleScatteringShader();

    ID3DXBuffer *pSingleScatteringBufferCode, *pErrorCode;
    HRESULT hr = D3DXCompileShader(szSingleScatteringShader.c_str(), szSingleScatteringShader.length(), nullptr, nullptr, "ComputeDirectIrradianceTexture", "ps_3_0", 0, &pSingleScatteringBufferCode, &pErrorCode, &m_pSingleScatteringShaderCode);
    // SAFE_RELEASE(pErrorCode);
    // CHECK_ERROR(hr)
    // hr = g_pDevice9->CreatePixelShader((const DWORD*)pIrradianceBufferCode->GetBufferPointer(), &m_pIrradianceShader);
    // SAFE_RELEASE(pIrradianceBufferCode);
    // CHECK_ERROR(hr)    
}