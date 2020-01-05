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

CAtomsphereRendererDX9::~CAtomsphereRendererDX9()
{
    Clean();
}

bool CAtomsphereRendererDX9::Precompute(const AtmosphereParams &params)
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
        { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
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

    g_pDevice9->BeginScene();

    g_pDevice9->SetDepthStencilSurface(m_pDSSurface);
    g_pDevice9->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);
    g_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    g_pDevice9->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_pDevice9->SetVertexShader(m_pVertexShader);
    g_pDevice9->SetStreamSource(0, m_pVB, 0, sizeof(float) * 2);
    g_pDevice9->SetVertexDeclaration(m_pVDecl);
    g_pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    g_pDevice9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    g_pDevice9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    g_pDevice9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

    ProcessTransmittanceTexture(szShaderHeader, params);
    ProcessIrradianceTexture(szShaderHeader, params);

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