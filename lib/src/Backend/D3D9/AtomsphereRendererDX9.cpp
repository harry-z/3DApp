#include "AtomsphereRendererDX9.h"
#include "TextureDX9.h"

dword GetConstantIndex(ID3DXConstantTable *pConstTable, LPCSTR pszConstName)
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

CAtomsphereRendererDX9::CAtomsphereRendererDX9()
{
    memset(m_ppDeltaRayleighScatteringSurface, 0, sizeof(intptr_t) * SCATTERING_TEXTURE_DEPTH);
    memset(m_ppDeltaMieScatteringSurface, 0, sizeof(intptr_t) * SCATTERING_TEXTURE_DEPTH);
    memset(m_ppRayleighScatteringSurface, 0, sizeof(intptr_t) * SCATTERING_TEXTURE_DEPTH);
    memset(m_ppMieScatteringSurface, 0, sizeof(intptr_t) * SCATTERING_TEXTURE_DEPTH);
}

CAtomsphereRendererDX9::~CAtomsphereRendererDX9()
{
    Clean();
}

bool CAtomsphereRendererDX9::Precompute(const AtmosphereParams &params)
{
    HRESULT hr = g_pDevice9->CreateTexture(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pTransmittance, nullptr);
    CHECK_ERROR(hr)
    hr = m_pTransmittance->GetSurfaceLevel(0, &m_pTransmittanceSurface);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateTexture(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pIrradiance, nullptr);
    CHECK_ERROR(hr)
    hr = m_pIrradiance->GetSurfaceLevel(0, &m_pIrradianceSurface);
    CHECK_ERROR(hr)

    hr = g_pDevice9->CreateVolumeTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 1, D3DUSAGE_DYNAMIC, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pDeltaRayleighScattering, nullptr);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateVolumeTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 1, D3DUSAGE_DYNAMIC, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pDeltaMieScattering, nullptr);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateVolumeTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 1, D3DUSAGE_DYNAMIC, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pRayleighScattering, nullptr);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateVolumeTexture(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, 1, D3DUSAGE_DYNAMIC, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pMieScattering, nullptr);
    CHECK_ERROR(hr)
    for (int i = 0; i < SCATTERING_TEXTURE_DEPTH; ++i)
    {
        hr = g_pDevice9->CreateRenderTarget(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, D3DFMT_A16B16G16R16F, D3DMULTISAMPLE_NONE, 0, TRUE, &m_ppDeltaRayleighScatteringSurface[i], nullptr);
        CHECK_ERROR(hr)
        hr = g_pDevice9->CreateRenderTarget(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, D3DFMT_A16B16G16R16F, D3DMULTISAMPLE_NONE, 0, TRUE, &m_ppDeltaMieScatteringSurface[i], nullptr);
        CHECK_ERROR(hr)
        hr = g_pDevice9->CreateRenderTarget(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, D3DFMT_A16B16G16R16F, D3DMULTISAMPLE_NONE, 0, TRUE, &m_ppRayleighScatteringSurface[i], nullptr);
        CHECK_ERROR(hr)
        hr = g_pDevice9->CreateRenderTarget(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, D3DFMT_A16B16G16R16F, D3DMULTISAMPLE_NONE, 0, TRUE, &m_ppMieScatteringSurface[i], nullptr);
        CHECK_ERROR(hr)
    }
    
    hr = g_pDevice9->CreateDepthStencilSurface(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, D3DFMT_D32, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pDSSurface, nullptr);
    CHECK_ERROR(hr)

    hr = g_pDevice9->CreateVertexBuffer(sizeof(float) * 16, 0, 0, D3DPOOL_MANAGED, &m_pVB, nullptr);
    CHECK_ERROR(hr)
    float vb[] = { 
        -1.0f, 1.0f, 0.0f, 0.0f
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f };
    void *p;
    m_pVB->Lock(0, sizeof(float) * 16, &p, 0);
    memcpy(p, vb, sizeof(float) * 16);
    m_pVB->Unlock();

    D3DVERTEXELEMENT9 VElem[] = {
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, sizeof(float) * 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0xFF, 0, D3DDECLTYPE_UNUSED, D3DDECLMETHOD_DEFAULT, 0, 0 }
    };
    hr = g_pDevice9->CreateVertexDeclaration(VElem, &m_pVDecl);
    CHECK_ERROR(hr)

    String szShaderHeader = ShaderHeader();
    String szVertexShader = VertexShader();

    ID3DXBuffer *pVertexShaderCode, *pErrorCode;
    hr = D3DXCompileShader(szVertexShader.c_str(), szVertexShader.length(), nullptr, nullptr, "DrawQuad", "vs_3_0", 0, &pVertexShaderCode, &pErrorCode, nullptr);
    SAFE_RELEASE(pErrorCode);
    CHECK_ERROR(hr)
    hr = g_pDevice9->CreateVertexShader((const DWORD*)pVertexShaderCode->GetBufferPointer(), &m_pVertexShader);
    SAFE_RELEASE(pVertexShaderCode);
    CHECK_ERROR(hr)

    std::function<bool(const String&, const AtmosphereParams&)> ProcessFuncs[] = {
        std::bind(&CAtomsphereRendererDX9::ProcessTransmittanceTexture, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&CAtomsphereRendererDX9::ProcessIrradianceTexture, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&CAtomsphereRendererDX9::ProcessSingleScatteringTexture, this, std::placeholders::_1, std::placeholders::_2)
    };

    g_pDevice9->BeginScene();

    g_pDevice9->SetDepthStencilSurface(m_pDSSurface);
    g_pDevice9->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
    g_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_pDevice9->SetVertexShader(m_pVertexShader);
    g_pDevice9->SetStreamSource(0, m_pVB, 0, sizeof(float) * 4);
    g_pDevice9->SetVertexDeclaration(m_pVDecl);
    g_pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    g_pDevice9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    g_pDevice9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

    dword nProcessFunc = 3;
    bool bFinished = true;
    for (dword i = 0; i < nProcessFunc; ++i)
    {
        if (!ProcessFuncs[i](szShaderHeader, params))
        {
            bFinished = false;
            break;
        }
    }

    g_pDevice9->EndScene();

    return bFinished;
}

void CAtomsphereRendererDX9::Clean()
{
    SAFE_RELEASE(m_pVertexShader)

    SAFE_RELEASE(m_pTransmittanceShader)
    SAFE_RELEASE(m_pTransmittanceShaderCode)
    SAFE_RELEASE(m_pIrradianceShader)
    SAFE_RELEASE(m_pIrradianceShaderCode)
    SAFE_RELEASE(m_pSingleScatteringShader)
    SAFE_RELEASE(m_pSingleScatteringShaderCode)

    SAFE_RELEASE(m_pTransmittance)
    SAFE_RELEASE(m_pTransmittanceSurface)
    SAFE_RELEASE(m_pIrradiance)
    SAFE_RELEASE(m_pIrradianceSurface)
    SAFE_RELEASE(m_pDeltaRayleighScattering)
    SAFE_RELEASE(m_pDeltaMieScattering)
    SAFE_RELEASE(m_pRayleighScattering)
    SAFE_RELEASE(m_pMieScattering)
    for (int i = 0; i < SCATTERING_TEXTURE_DEPTH; ++i)
    {
        SAFE_RELEASE(m_ppDeltaRayleighScatteringSurface[i])
        SAFE_RELEASE(m_ppDeltaMieScatteringSurface[i])    
        SAFE_RELEASE(m_ppRayleighScatteringSurface[i])
        SAFE_RELEASE(m_ppMieScatteringSurface[i])
    }
    SAFE_RELEASE(m_pDSSurface)

    SAFE_RELEASE(m_pVB)
    SAFE_RELEASE(m_pVDecl)
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