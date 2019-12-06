#pragma once
#include "Prereq.h"

constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

struct DensityProfileLayer
{
    DensityProfileLayer()
    : m_Width(0.0)
    , m_ExpTerm(0.0)
    , m_ExpScale(0.0)
    , m_LinearTerm(0.0)
    , m_ConstantTerm(0.0)
    {}

    DensityProfileLayer(double Width, double ExpTerm, double ExpScale, double LinearTerm, double ConstantTerm)
    : m_Width(Width)
    , m_ExpTerm(ExpTerm)
    , m_ExpScale(ExpScale)
    , m_LinearTerm(LinearTerm)
    , m_ConstantTerm(ConstantTerm)
    {}

    double m_Width;
    double m_ExpTerm;
    double m_ExpScale;
    double m_LinearTerm;
    double m_ConstantTerm;
};

struct AtmosphereParams
{
    CArray<double> m_arrWaveLength;
    CArray<double> m_arrSolarIrradiance;
    CArray<DensityProfileLayer> m_arrRayleighDensity;
    CArray<DensityProfileLayer> m_arrMieDensity;
    CArray<DensityProfileLayer> m_arrAbsorptionDensity;
    Vec3d m_RayleighScattering;
    Vec3d m_MieScattering;
    Vec3d m_MieExtinction;
    Vec3d m_AbsorptionExtinction;
    Vec3d m_GroundAlbedo;
    double m_SunAngularRadius;
    double m_BottomRadius;
    double m_TopRadius;
    double m_MiePhaseFunctionG;
    double m_MaxSunZenithAngle;
    double m_LengthUnitInMeters;
};

class IAtmosphereRenderer
{
public:
    virtual ~IAtmosphereRenderer() {}
    virtual bool Init(const AtmosphereParams &params) = 0;
};

class CAtmosphere
{
public:
    CAtmosphere();
    ~CAtmosphere();
    void Init();
    void Precompute(const AtmosphereParams &params);

private:
    IAtmosphereRenderer *m_pRenderer;
};

