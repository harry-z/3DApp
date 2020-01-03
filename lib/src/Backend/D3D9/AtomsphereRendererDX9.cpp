#include "AtomsphereRendererDX9.h"
#include "TextureDX9.h"

#define CHECK_ERROR(hr) \
    if (FAILED(hr)) \
    { \
        Clean(); \
        return false; \
    }

#define CHECK_INDEX_ERROR(index) \
    if (index == 0xFFFFFFFF)
    {
        Clean();
        return false;
    }

UINT GetConstantIndex(ID3DXConstantTable *pConstTable, LPCSTR pszConstName)
{
    D3DXHANDLE h = pConstTable->GetConstantByName(NULL, pszConstName);
    if (h != nullptr)
    {
        D3DXCONSTANT_DESC desc;
        UINT nCount;
        if (SUCCEEDED(pConstTable->GetConstantDesc(h, &desc, &nCount)))
            return desc.RegisterIndex;
    }
    return 0xFFFFFFFF;
}

CAtomsphereRendererDX9::~CAtomsphereRendererDX9()
{
    Clean();
}

bool CAtomsphereRendererDX9::Precompute(
    const Vec4d &TopBottomRadius,
    const Vec3d &RayleighScattering,
    const Vec3d &MieExtinction,
    const Vec3d &AbsorptionExtinction,
    const CArray<DensityProfileLayer> &RayleighDensity,
    const CArray<DensityProfileLayer> &MieExtinctionDensity,
    const CArray<DensityProfileLayer> &AbsorptionExtinctionDensity
)
{
    HRESULT hr = g_pDevice9->CreateTexture(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pTransmittance, nullptr);
    CHECK_ERROR(hr)
    hr = m_pTransmittance->GetSurfaceLevel(0, &m_pTransmittanceSurface);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateTexture(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_pIrradiance, nullptr);
    CHECK_ERROR(hr)
    hr = m_pIrradiance->GetSurfaceLevel(0, &m_pIrradianceSurface);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateDepthStencilSurface(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, D3DFMT_D32, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pDSSurface, nullptr);
    CHECK_ERROR(hr)

    hr = g_pDevice9->CreateVertexBuffer(sizeof(float) * 8, 0, 0, D3DPOOL_MANAGED, &m_pVB, nullptr);
    CHECK_ERROR(hr)
    float vb[] = { -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f };
    void *p;
    m_pVB->Lock(0, sizeof(float) * 8, &p, 0);
    memcpy(p, vb, sizeof(float) * 8);
    m_pVB->Unlock();

    D3DVERTEXELEMENT9 VElem[] = {
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }
        { 0xFF, 0, D3DDECLTYPE_UNUSED, D3DDECLMETHOD_DEFAULT, 0, 0 }
    };
    hr = g_pDevice9->CreateVertexDeclaration(VElem, &m_pVDecl);
    CHECK_ERROR(hr)

    String szShaderHeader = ShaderHeader();
    String szVertexShader = VertexShader();
    String szTransmittanceShader = szShaderHeader + TransmittanceShader();
    String szIrradianceShader = szShaderHeader + IrradianceShader();

    ID3DXBuffer *pVertexShaderCode, pErrorCode;
    hr = D3DXCompileShader(szVertexShader.c_str(), szVertexShader.length(), nullptr, nullptr, "DrawQuad", "vs_3_0", 0, &pVertexShaderCode, &pErrorCode, nullptr);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateVertexShader((const DWORD*)pVertexShaderCode->GetBufferPointer(), &m_pVertexShader);
    SAFE_RELEASE(pVertexShaderCode);
    CHECK_ERROR(hr)

    ID3DXBuffer *pTransmittanceBufferCode;
    hr = D3DXCompileShader(szTransmittanceShader.c_str(), szTransmittanceShader.length(), nullptr, nullptr, "ComputeTransmittanceToTopAtmosphereBoundaryTexture", "ps_3_0", 0, &pTransmittanceBufferCode, &pErrorCode, &m_pTransmittanceShaderCode);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreatePixelShader((const DWORD*)pTransmittanceBufferCode->GetBufferPointer(), &m_pTransmittanceShader);
    SAFE_RELEASE(pTransmittanceBufferCode);
    CHECK_ERROR(hr)

    ID3DXBuffer *pIrradianceBufferCode;
    hr = D3DXCompileShader(szIrradianceShader.c_str(), szIrradianceShader.length(), nullptr, nullptr, "ComputeDirectIrradianceTexture", "ps_3_0", 0, &pIrradianceBufferCode, &pErrorCode, &m_pIrradianceShaderCode);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreatePixelShader((const DWORD*)pIrradianceBufferCode->GetBufferPointer(), &m_pIrradianceShader);
    SAFE_RELEASE(pIrradianceBufferCode);
    CHECK_ERROR(hr)
    
    UINT nTextureDimIndex = GetConstantIndex(m_pTransmittanceShaderCode, "TextureDim");
    CHECK_INDEX_ERROR(nTextureDimIndex);
    UINT nTopBottomRadius = GetConstantIndex(m_pTransmittanceShaderCode, "TopBottomRadius");
    CHECK_INDEX_ERROR(nTopBottomRadius);
    UINT nRayleighScattering = GetConstantIndex(m_pTransmittanceShaderCode, "RayleighScattering");
    CHECK_INDEX_ERROR(nRayleighScattering);
    UINT nMieExtinction = GetConstantIndex(m_pTransmittanceShaderCode, "MieExtinction");
    CHECK_INDEX_ERROR(nMieExtinction);
    UINT nAbsorptionExtinction = GetConstantIndex(m_pTransmittanceShaderCode, "AbsorptionExtinction");
    CHECK_INDEX_ERROR(nAbsorptionExtinction);
    UINT nRMAExpTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMAExpTerm");
    CHECK_INDEX_ERROR(nRMAExpTerm);
    UINT nRMAExpScale = GetConstantIndex(m_pTransmittanceShaderCode, "RMAExpScale");
    CHECK_INDEX_ERROR(nRMAExpScale);
    UINT nRMALinearTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMALinearTerm");
    CHECK_INDEX_ERROR(nRMALinearTerm);
    UINT nRMAConstTerm = GetConstantIndex(m_pTransmittanceShaderCode, "RMAConstTerm");
    CHECK_INDEX_ERROR(nRMAConstTerm);
    UINT nRMAWidth = GetConstantIndex(m_pTransmittanceShaderCode, "RMAWidth");
    CHECK_INDEX_ERROR(nRMAWidth);

    float TextureDim[] = { TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT };
    Vec4 fTopBottomRadius((float)TopBottomRadius.x, (float)TopBottomRadius.y, (float)TopBottomRadius.z, (float)TopBottomRadius.w);
    Vec3 fRayleighScattering((float)RayleighScattering.x, (float)RayleighScattering.y, (float)RayleighScattering.z);
    Vec3 fMieExtinction((float)MieExtinction.x, (float)MieExtinction.y, (float)MieExtinction.z);
    Vec3 fAbsorptionExtinction((float)AbsorptionExtinction.x, (float)AbsorptionExtinction.y, (float)AbsorptionExtinction.z);
    float RMAExpTerm[] = {
        (float)arrRayleighDensity[0].m_ExpTerm,
        (float)arrMieDensity[0].m_ExpTerm,
        (float)arrAbsorptionExtinction[0].m_ExpTerm,
        0.0f,
        (float)arrRayleighDensity[1].m_ExpTerm,
        (float)arrMieDensity[1].m_ExpTerm,
        (float)arrAbsorptionExtinction[1].m_ExpTerm,
        0.0f
    };
    float RMAExpScale[] = {
        (float)arrRayleighDensity[0].m_ExpScale,
        (float)arrMieDensity[0].m_ExpScale,
        (float)arrAbsorptionExtinction[0].m_ExpScale,
        0.0f,
        (float)arrRayleighDensity[1].m_ExpScale,
        (float)arrMieDensity[1].m_ExpScale,
        (float)arrAbsorptionExtinction[1].m_ExpScale,
        0.0f
    };
    float RMALinearTerm[] = {
        (float)arrRayleighDensity[0].m_LinearTerm,
        (float)arrMieDensity[0].m_LinearTerm,
        (float)arrAbsorptionDensity[0].m_LinearTerm,
        0.0f,
        (float)arrRayleighDensity[1].m_LinearTerm,
        (float)arrMieDensity[1].m_LinearTerm,
        (float)arrAbsorptionDensity[1].m_LinearTerm,
        0.0f
    };
    float RMAConstTerm[] = {
        (float)arrRayleighDensity[0].m_ConstantTerm,
        (float)arrMieDensity[0].m_ConstantTerm,
        (float)arrAbsorptionDensity[0].m_ConstantTerm,
        0.0f,
        (float)arrRayleighDensity[1].m_ConstantTerm,
        (float)arrMieDensity[1].m_ConstantTerm,
        (float)arrAbsorptionDensity[1].m_ConstantTerm,
        0.0f
    };
    float RMAWidth[] = {
        (float)arrRayleighDensity[0].m_Width,
        (float)arrMieDensity[0].m_Width,
        (float)arrAbsorptionDensity[0].m_Width,
    };

    g_pDevice9->BeginScene();

    g_pDevice9->SetRenderTarget(0, m_pTransmittanceSurface);
    g_pDevice9->SetDepthStencilSurface(m_pDSSurface);
    g_pDevice9->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
    g_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    g_pDevice9->SetVertexShader(m_pVertexShader);
    g_pDevice9->SetPixelShader(m_pTransmittanceShader);
    g_pDevice9->SetPixelShaderConstantF(nTextureDim, TextureDim, 1);
    g_pDevice9->SetPixelShaderConstantF(nTopBottomRadius, &fTopBottomRadius.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nRayleighScattering, &fRayleighScattering.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nMieExtinction, &fMieExtinction.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nAbsorptionExtinction, &fAbsorptionExtinction.x, 1);
    g_pDevice9->SetPixelShaderConstantF(nRMAExpTerm, RMAExpTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAExpScale, RMAExpScale, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMALinearTerm, RMALinearTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAConstTerm, RMAConstTerm, 2);
    g_pDevice9->SetPixelShaderConstantF(nRMAWidth, RMAWidth, 1);
    g_pDevice9->SetStreamSource(0, m_pVB, 0, sizeof(float) * 2);
    g_pDevice9->SetVertexDeclaration(m_pVDecl);
    g_pDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    g_pDevice9->EndScene();

}

void CAtomsphereRendererDX9::Clean()
{
    SAFE_RELEASE(m_pVertexShader);

    SAFE_RELEASE(m_pTransmittanceShader);
    SAFE_RELEASE(m_pTransmittanceShaderCode);
    SAFE_RELEASE(m_pIrradianceShader);
    SAFE_RELEASE(m_pIrradianceShaderCode);

    SAFE_RELEASE(m_pTransmittance);
    SAFE_RELEASE(m_pTransmittanceSurface);
    SAFE_RELEASE(m_pIrradiance);
    SAFE_RELEASE(m_pIrradianceSurface);
    SAFE_RELEASE(m_pDSSurface);

    SAFE_RELEASE(m_pVB);
    SAFE_RELEASE(m_pVDecl);
}

double CAtomsphereRendererDX9::Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength) const
{
    if (WaveLength < arrWaveLength[0])
        return arrWaveLengthFunction[0];
    for (dword i = 0; i < arrWaveLength.Num() - 1; ++i)
    {
        if (WaveLength < arrWaveLength[i + 1])
        {
            double u = (WaveLength - arrWaveLength[i]) / (arrWaveLength[i + 1] - arrWaveLength[i]);
            return arrWaveLengthFunction[i] * (1.0 - u) + arrWaveLengthFunction[i + 1] * u;
        }
    }
    return arrWaveLengthFunction[arrWaveLengthFunction.Num() - 1];
}