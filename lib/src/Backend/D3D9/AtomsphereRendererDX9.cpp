#include "AtomsphereRendererDX9.h"
#include "TextureDX9.h"

CAtomsphereRendererDX9::~CAtomsphereRendererDX9()
{
    SAFE_RELEASE(m_pTransmittance);
}

bool CAtomsphereRendererDX9::Init(const AtmosphereParams &params)
{
    HRESULT hr = g_pDevice9->CreateTexture(
        TRANSMITTANCE_TEXTURE_WIDTH, 
        TRANSMITTANCE_TEXTURE_HEIGHT, 
        1, 
        D3DUSAGE_RENDERTARGET, 
        D3DFMT_A32B32G32R32F, 
        D3DPOOL_DEFAULT, 
        &m_pTransmittance,
        nullptr);
    if (FAILED(hr))
        return false;
}

float CAtomsphereRendererDX9::Interpolate(const CArray<float> &arrWaveLength, const CArray<float> &arrWaveLengthFunction, float WaveLength) const
{
    if (WaveLength < arrWaveLength[0])
        return arrWaveLengthFunction[0];
    for (dword i = 0; i < arrWaveLength.Num() - 1; ++i)
    {
        if (WaveLength < arrWaveLength[i + 1])
        {
            float u = (WaveLength - arrWaveLength[i]) / (arrWaveLength[i + 1] - arrWaveLength[i]);
            return arrWaveLengthFunction[i] * (1.0f - u) + arrWaveLengthFunction[i + 1] * u;
        }
    }
    return arrWaveLengthFunction[arrWaveLengthFunction.Num() - 1];
}