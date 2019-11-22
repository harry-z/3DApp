#include "Atomsphere.h"
#ifdef RENDERAPI_DX9
#include "Backend/D3D9/AtomsphereRendererDX9.h"
#endif

CAtmosphere::CAtmosphere()
{
#ifdef RENDERAPI_DX9
    m_pRenderer = NewObject<CAtomsphereRendererDX9>();
#endif
}

bool CAtmosphere::Init()
{
    constexpr int LambdaMin = 360;
    constexpr int LambdaMax = 830;
    constexpr float Rayleigh = 1.24062e-6;
    constexpr float MieAngstromBeta = 5.328e-3;
    constexpr float MieAngstromAlpha = 0.0f;
    constexpr float MieScaleHeight = 1200.0f;
    constexpr float MieSingleScatteringAlbedo = 0.9f;

    AtmosphereParams params;
    for (int l = LambdaMin; l <= LambdaMax; ++l)
    {
        float lambda = 1e-3 * l;
        float mie = MieAngstromBeta / MieScaleHeight * pow(lambda, -MieAngstromAlpha);
        params.m_arrWaveLength.Add(l);
        params.m_arrRayleighScattering.Add(Rayleigh * powf(lambda, -4.0f));
        params.m_arrMieScattering.Add(mie * MieSingleScatteringAlbedo);
    }
    return m_pRenderer->Init(params);
}