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

CAtmosphere::~CAtmosphere()
{
#ifdef RENDERAPI_DX9
    CAtomsphereRendererDX9 *pRendererDX9 = static_cast<CAtomsphereRendererDX9*>(m_pRenderer);
    DeleteObject(pRendererDX9);
#endif
}

bool CAtmosphere::Init()
{
    constexpr int LambdaMin = 360;
    constexpr int LambdaMax = 830;
    constexpr double DobsonUnit = 2.687e20;
    constexpr double MaxOzoneNumberDensity = 300.0 * DobsonUnit / 15000.0;
    constexpr double Rayleigh = 1.24062e-6;
    constexpr double RayleighScaleHeight = 8000.0;
    constexpr double MieScaleHeight = 1200.0;
    constexpr double MieAngstromAlpha = 0.0;
    constexpr double MieAngstromBeta = 5.328e-3;
    constexpr double MieSingleScatteringAlbedo = 0.9;
    constexpr double GroundAlbedo = 0.1;
    // http://www.iup.uni-bremen.de/gruppen/molspec/databases
    constexpr double OzoneCrossSection[48] = {
        1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
        8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
        1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
        4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
        2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
        6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
        2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
    };

    int bUseOzone = 1;
    int bUseRayleighScattering = 1;
    int bUseMieScattering = 1;
    int bUseHalfPrecision = 1;

    AtmosphereParams params;
    params.m_SunAngularRadius = 0.00935 / 2.0;
    params.m_BottomRadius = 6360000.0;
    params.m_TopRadius = 6420000.0;
    params.m_arrRayleighDensity.Emplace(0.0, 
        1.0 * bUseRayleighScattering, 
        -1.0 / RayleighScaleHeight * bUseRayleighScattering, 
        0.0, 
        0.0);
    params.m_arrMieDensity.Emplace(0.0,
        1.0 * bUseMieScattering,
        -1.0 / MieScaleHeight * bUseMieScattering, 
        0.0,
        0.0);
    params.m_arrAbsorptionDensity.Emplace(25000.0, 
        0.0,
        0.0,
        1.0 / 15000.0, 
        -2.0 / 3.0);
    params.m_arrAbsorptionDensity.Emplace(0.0,
        0.0,
        0.0,
        -1.0 / 15000.0, 
        8.0 / 3.0);
    params.m_MiePhaseFunctionG = 0.8;
    params.m_MaxSunZenithAngle = ((bUseHalfPrecision == 1) ? 102.0 : 120.0) / 180.0 * PI;
    params.m_LengthUnitInMeters = 1000.0;
    for (int l = LambdaMin; l <= LambdaMax; l += 10)
    {
        double lambda = 1e-3 * l;
        double mie = MieAngstromBeta / MieScaleHeight * pow(lambda, -MieAngstromAlpha);
        double WaveLength = (double)l;
        params.m_arrWaveLength.Add(WaveLength);
        double RayleighScattering = Rayleigh * pow(lambda, -4.0);
        params.m_arrRayleighScattering.Add(RayleighScattering);
        double MieScattering = mie * MieSingleScatteringAlbedo;
        params.m_arrMieScattering.Add(MieScattering);
        params.m_arrMieExtinction.Add(mie);
        double AbsorptionExtinction = MaxOzoneNumberDensity * OzoneCrossSection[(l - LambdaMin) / 10] * bUseOzone;
        params.m_arrAbsorptionExtinction.Add(AbsorptionExtinction);
        double GroundA = GroundAlbedo;
        params.m_arrGroundAlbedo.Add(GroundA);
    }

    return m_pRenderer->Init(params);
}