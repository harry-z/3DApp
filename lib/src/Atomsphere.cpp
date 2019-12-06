#include "Atomsphere.h"
#ifdef RENDERAPI_DX9
#include "Backend/D3D9/AtomsphereRendererDX9.h"
#endif

inline double ClampCosine(double mu)
{
    return Clamp<double>(mu, -1.0, 1.0);
}

inline double GetUnitRangeFromTextureCoord(double u, int texture_size)
{
    return ((u - 0.5 / (double)(texture_size)) / (1.0 - 1.0 / (double)(texture_size)));
}

inline double ClampDistance(double d)
{
    return max(d, 0.0);
}

inline double SafeSqrt(double sqr)
{
    return sqrt(max(sqr, 0.0));
}

inline double Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength)
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

inline Vec3d InterpolateVec3(const AtmosphereParams &params, const CArray<double> &v, const double *lambdas, double scale)
{
    return Vec3d(
        Interpolate(params.m_arrWaveLength, v, lambdas[0]) * scale,
        Interpolate(params.m_arrWaveLength, v, lambdas[1]) * scale,
        Interpolate(params.m_arrWaveLength, v, lambdas[2]) * scale
    );
}

double DistanceToTopAtmosphereBoundary(const AtmosphereParams &params, double r, double mu)
{
    double discriminant = r*r*(mu*mu-1.0) + params.m_TopRadius*params.m_TopRadius;
    return ClampDistance(-r*mu + SafeSqrt(discriminant));
}

double GetLayerDensity(const DensityProfileLayer &layer, double altitude)
{
    double density = layer.m_ExpTerm * exp(layer.m_ExpScale * altitude) + layer.m_LinearTerm * altitude + layer.m_ConstantTerm;
    return Clamp<double>(density, 0.0, 1.0);
}

double GetProfileDensity(const CArray<DensityProfileLayer> &profile, double altitude)
{
    return altitude < profile[0].m_Width ? GetLayerDensity(profile[0], altitude) : GetLayerDensity(profile[1], altitude);
}

void GetRMuFromTransmittanceTextureUv(const AtmosphereParams &params, const Vec2d &uv, double &r, double &mu)
{
    double x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
    double x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
    double H = sqrt(params.m_TopRadius * params.m_TopRadius - params.m_BottomRadius * params.m_BottomRadius);
    double rho = H * x_r;
    r = sqrt(rho * rho + params.m_BottomRadius * params.m_BottomRadius);
    double d_min = params.m_TopRadius - r;
    double d_max = rho + H;
    double d = d_min + x_mu * (d_max - d_min);
    mu = d == 0.0 ? 1.0 : (H * H - rho * rho - d * d) / (2.0 * r * d);
    ClampCosine(mu);
}

double ComputeOpticalLengthToTopAtmosphereBoundary(const AtmosphereParams &params, const CArray<DensityProfileLayer> &profile, double r, double mu)
{
    constexpr int SAMPLE_COUNT = 500;
    double dx = DistanceToTopAtmosphereBoundary(params, r, mu) / (double)(SAMPLE_COUNT);
    double result = 0.0;
    for (int i = 0; i <= SAMPLE_COUNT; ++i)
    {
        double d_i = dx * i;
        double r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
        double y_i = GetProfileDensity(profile, r_i - params.m_BottomRadius);
        result += y_i * dx * (i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0);
    }
    return result;
}

Vec3d ComputeTransmittanceToTopAtmosphereBoundary(const AtmosphereParams &params, double r, double mu)
{
    Vec3d Transmittance = params.m_RayleighScattering * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.m_arrRayleighDensity, r, mu) + 
        params.m_MieExtinction * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.m_arrMieDensity, r, mu) + 
        params.m_AbsorptionExtinction * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.m_arrAbsorptionDensity, r, mu);
    return Vec3d(exp(-Transmittance.x), exp(-Transmittance.y), exp(-Transmittance.z));
}

Vec3d ComputeTransmittanceToTopAtmosphereBoundaryTexture(const AtmosphereParams &params, const Vec2d &uv)
{
    double r, mu;
    GetRMuFromTransmittanceTextureUv(params, Vec2d(uv.x / TRANSMITTANCE_TEXTURE_WIDTH, uv.y / TRANSMITTANCE_TEXTURE_HEIGHT), r, mu);
    return ComputeTransmittanceToTopAtmosphereBoundary(params, r, mu);
}

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

void CAtmosphere::Init()
{
    constexpr int cLayerCount = 2;
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

    const double lambdas[] = { 680.0, 550.0, 440.0 };

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
    params.m_arrRayleighDensity.Emplace(DensityProfileLayer());
    params.m_arrMieDensity.Emplace(0.0,
        1.0 * bUseMieScattering,
        -1.0 / MieScaleHeight * bUseMieScattering, 
        0.0,
        0.0);
    params.m_arrMieDensity.Emplace(DensityProfileLayer());
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
    CArray<double> arrRayleighScattering;
    CArray<double> arrMieScattering;
    CArray<double> arrMieExtinction;
    CArray<double> arrAbsorptionExtinction;
    CArray<double> arrGroundAlbedo;
    for (int l = LambdaMin; l <= LambdaMax; l += 10)
    {
        double lambda = 1e-3 * l;
        double mie = MieAngstromBeta / MieScaleHeight * pow(lambda, -MieAngstromAlpha);
        double WaveLength = (double)l;
        params.m_arrWaveLength.Add(WaveLength);
        double RayleighScattering = Rayleigh * pow(lambda, -4.0);
        arrRayleighScattering.Add(RayleighScattering);
        double MieScattering = mie * MieSingleScatteringAlbedo;
        arrMieScattering.Add(MieScattering);
        arrMieExtinction.Add(mie);
        double AbsorptionExtinction = MaxOzoneNumberDensity * OzoneCrossSection[(l - LambdaMin) / 10] * bUseOzone;
        arrAbsorptionExtinction.Add(AbsorptionExtinction);
        double GroundA = GroundAlbedo;
        arrGroundAlbedo.Add(GroundA);
    }

    params.m_RayleighScattering = InterpolateVec3(params, arrRayleighScattering, lambdas, params.m_LengthUnitInMeters);
    params.m_MieScattering = InterpolateVec3(params, arrMieScattering, lambdas, params.m_LengthUnitInMeters);
    params.m_MieExtinction = InterpolateVec3(params, arrMieExtinction, lambdas, params.m_LengthUnitInMeters);
    params.m_AbsorptionExtinction = InterpolateVec3(params, arrAbsorptionExtinction, lambdas, params.m_LengthUnitInMeters);
    params.m_GroundAlbedo = InterpolateVec3(params, arrGroundAlbedo, lambdas, params.m_LengthUnitInMeters);

    CArray<DensityProfileLayer> arrRayleighDensity = params.m_arrRayleighDensity;
    if (arrRayleighDensity.Num() < cLayerCount)
    {
        while (arrRayleighDensity.Num() < cLayerCount)
            arrRayleighDensity.Add(DensityProfileLayer());
    }

    CArray<DensityProfileLayer> arrMieDensity = params.m_arrMieDensity;
    if (arrMieDensity.Num() < cLayerCount)
    {
        while (arrMieDensity.Num() < cLayerCount)
            arrMieDensity.Add(DensityProfileLayer());
    }

    CArray<DensityProfileLayer> arrAbsorptionDensity = params.m_arrAbsorptionDensity;
    if (arrAbsorptionDensity.Num() < cLayerCount)
    {
        while (arrAbsorptionDensity.Num() < cLayerCount)
            arrAbsorptionDensity.Add(DensityProfileLayer());
    }

    Precompute(params);
}

void CAtmosphere::Precompute(const AtmosphereParams &params)
{
    double *pTransmittanceData = (double*)MEMALLOC(sizeof(double) * 3 * TRANSMITTANCE_TEXTURE_WIDTH * TRANSMITTANCE_TEXTURE_HEIGHT);
    double *pTransmittanceDataGuard = pTransmittanceData;
    int rIter = TRANSMITTANCE_TEXTURE_HEIGHT - 1;
    int cIter = TRANSMITTANCE_TEXTURE_WIDTH - 1;
    for (int r = 0; r < TRANSMITTANCE_TEXTURE_HEIGHT; ++r)
    {
        for (int c = 0; c < TRANSMITTANCE_TEXTURE_WIDTH; ++c)
        {
            Vec3d data = ComputeTransmittanceToTopAtmosphereBoundaryTexture(params, Vec2d(c / cIter, r / rIter));
            *pTransmittanceDataGuard++ = data.x;
            *pTransmittanceDataGuard++ = data.y;
            *pTransmittanceDataGuard++ = data.z;
        }
    }
}