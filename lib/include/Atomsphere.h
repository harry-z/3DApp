#pragma once
#include "Prereq.h"

constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

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

// struct AtmosphereParams
// {
//     CArray<double> m_arrWaveLength;
//     CArray<double> m_arrSolarIrradiance;
//     CArray<DensityProfileLayer> m_arrRayleighDensity;
//     CArray<DensityProfileLayer> m_arrMieDensity;
//     CArray<DensityProfileLayer> m_arrAbsorptionDensity;
//     Vec3d m_RayleighScattering;
//     Vec3d m_MieScattering;
//     Vec3d m_MieExtinction;
//     Vec3d m_AbsorptionExtinction;
//     Vec3d m_GroundAlbedo;
//     double m_SunAngularRadius;
//     double m_BottomRadius;
//     double m_TopRadius;
//     double m_MiePhaseFunctionG;
//     double m_MaxSunZenithAngle;
//     double m_LengthUnitInMeters;
// };

class IAtmosphereRenderer
{
public:
    virtual ~IAtmosphereRenderer() {}
    virtual bool Precompute(
        const Vec4d &TopBottomRadius,
        const Vec3d &RayleighScattering,
        const Vec3d &MieExtinction,
        const Vec3d &AbsorptionExtinction,
        const CArray<DensityProfileLayer> &RayleighDensity,
        const CArray<DensityProfileLayer> &MieExtinctionDensity,
        const CArray<DensityProfileLayer> &AbsorptionExtinctionDensity) = 0;
};

class CAtmosphere
{
public:
    CAtmosphere();
    ~CAtmosphere();
    bool Precompute();

private:
    // struct {
    //     Vec4d TopBottomRadius;
    //     Vec3d RayleighScattering;
    //     Vec3d MieExtinction;
    //     Vec3d AbsorptionExtinction;
    //     Vec3d RMAExpTerm[2];
    //     Vec3d RMAExpScale[2];
    //     Vec3d RMALinearTerm[2];
    //     Vec3d RMAConstTerm[2];
    //     Vec3d RMAWidth;
    // } TransmittanceData;
    
    IAtmosphereRenderer *m_pRenderer;
};

