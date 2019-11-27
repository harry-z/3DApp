#include "AtomsphereRendererDX9.h"

String CAtomsphereRendererDX9::ShaderHeader(const AtmosphereParams &params) const
{
    constexpr dword cLayerCount = 2;
    const double lambdas[] = { 680.0, 550.0, 440.0 };
    // const Vec3 lambdas(680.0f, 550.0f, 440.0f);

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

    auto ToFloat3String = [this](const AtmosphereParams &params, const CArray<double> &v, const double *lambdas, double scale) -> String {
        double r = Interpolate(params.m_arrWaveLength, v, lambdas[0]) * scale;
        double g = Interpolate(params.m_arrWaveLength, v, lambdas[1]) * scale;
        double b = Interpolate(params.m_arrWaveLength, v, lambdas[2]) * scale;
        return String("float3(") + ToString(r) + "," + ToString(g) + "," + ToString(b) + ")";
    };

    auto ToDensityProfileString = [](const DensityProfileLayer &layer) -> String {
        char szParam[CSTR_MAX];
        sprintf(szParam, "%f,%f,%f,%f,%f", 
            layer.m_Width, 
            layer.m_ExpTerm, 
            layer.m_ExpScale, 
            layer.m_LinearTerm, 
            layer.m_ConstantTerm);
        return String(szParam);
    };

    String szHeader = String("const int TRANSMITTANCE_TEXTURE_WIDTH = ") +
        ToString(TRANSMITTANCE_TEXTURE_WIDTH) + 
        ";\nconst int TRANSMITTANCE_TEXTURE_HEIGHT = " +
        ToString(TRANSMITTANCE_TEXTURE_HEIGHT) +
        ";\n" + 
        String(
            "struct DensityProfileLayer {\n" \
            "float width;\n" \
            "float exp_term;\n" \
            "float exp_scale;\n" \
            "float linear_term;\n" \
            "float constant_term;};\n" \

            "struct DensityProfile {\n" \
            "DensityProfileLayer layers[2];};\n" \

            "struct AtmosphereParameters {\n" \
            "float sun_angular_radius;\n" \
            "float bottom_radius;\n" \
            "float top_radius;\n" \
            "DensityProfile rayleigh_density;\n" \
            "float3 rayleigh_scattering;\n" \
            "DensityProfile mie_density;\n" \
            "float3 mie_scattering;\n" \
            "float3 mie_extinction;\n" \
            "float mie_phase_function_g;\n" \
            "DensityProfile absorption_density;\n" \
            "float3 absorption_extinction;\n" \
            "float3 ground_albedo;\n" \
            "float mu_s_min;};\n" \

            "void InitDensityProfileLayer(float width, float exp_term, float exp_scale, float linear_term, float constant_term, out DensityProfileLayer layer) {\n" \
            "layer.width = width;\n" \
            "layer.exp_term = exp_term;\n" \
            "layer.exp_scale = exp_scale;\n" \
            "layer.linear_term = linear_term;\n" \
            "layer.constant_term = constant_term;}\n" \
            
            "float ClampCosine(float mu) {\n" \
            "return clamp(mu, -1.0f, 1.0f);}\n" \

            "float GetUnitRangeFromTextureCoord(float u, int texture_size) {\n" \
            "return ((u - 0.5f / float(texture_size)) / (1.0f - 1.0f / float(texture_size)));}\n" \

            "float ClampDistance(float d) {\n" \
            "return max(d, 0.0f);}\n" \

            "float SafeSqrt(float a) {\n" \
            "return sqrt(max(a, 0.0f));}\n" \

            "float DistanceToTopAtmosphereBoundary(AtmosphereParameters params, float r, float mu) {\n" \
            "float discriminant = r*r*(mu*mu-1.0f) + params.top_radius*params.top_radius;\n" \
            "return ClampDistance(-r*mu + SafeSqrt(discriminant));}\n" \

            "float GetLayerDensity(DensityProfileLayer layer, float altitude) {\n" \
            "float density = layer.exp_term * exp(layer.exp_scale * altitude) +\n" \
            "layer.linear_term * altitude + layer.constant_term;\n" \
            "return clamp(density, 0.0f, 1.0f);}\n" \

            "float GetProfileDensity(DensityProfile profile, float altitude) {\n" \
            "return altitude < profile.layers[0].width ?\n" \
            "GetLayerDensity(profile.layers[0], altitude) : GetLayerDensity(profile.layers[1], altitude);}\n"
        ) +
        "void InitParams(out AtmosphereParameters params) {\n" +
        "params.sun_angular_radius = " + ToString(params.m_SunAngularRadius) +
        ";\nparams.bottom_radius = " + ToString(params.m_BottomRadius / params.m_LengthUnitInMeters) +
        ";\nparams.top_radius = " + ToString(params.m_TopRadius / params.m_LengthUnitInMeters) +
        ";\nparams.rayleigh_scattering = " + ToFloat3String(params, params.m_arrRayleighScattering, lambdas, params.m_LengthUnitInMeters) +
        ";\nparams.mie_scattering = " + ToFloat3String(params, params.m_arrMieScattering, lambdas, params.m_LengthUnitInMeters) +
        ";\nparams.mie_extinction = " + ToFloat3String(params, params.m_arrMieExtinction, lambdas, params.m_LengthUnitInMeters) +
        ";\nparams.mie_phase_function_g = " + ToString(params.m_MiePhaseFunctionG) +
        ";\nparams.absorption_extinction = " + ToFloat3String(params, params.m_arrAbsorptionExtinction, lambdas, params.m_LengthUnitInMeters) + 
        ";\nparams.ground_albedo = " + ToFloat3String(params, params.m_arrGroundAlbedo, lambdas, 1.0) +
        ";\nparams.mu_s_min = " + ToString(cos(params.m_MaxSunZenithAngle)) +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrRayleighDensity[0]) + ",params.rayleigh_density.layers[0])" +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrRayleighDensity[1]) + ",params.rayleigh_density.layers[1])" +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrMieDensity[0]) + ",params.mie_density.layers[0])" +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrMieDensity[1]) + ",params.mie_density.layers[1])" +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrAbsorptionDensity[0]) + ",params.absorption_density.layers[0])" +
        ";\nInitDensityProfileLayer(" + ToDensityProfileString(arrAbsorptionDensity[1]) + ",params.absorption_density.layers[1]);}\n"
        ;

    Global::m_pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, szHeader.c_str());
    return szHeader;
}

String CAtomsphereRendererDX9::TransmittanceShader() const
{
    return String();
    // return String(
    //     "void GetRMuFromTransmittanceTextureUv(AtmosphereParameters params, float2 uv, out float r, out float mu) {\n" \
    //     "float x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);\n" \
    //     "float x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);\n" \
    //     "float H = sqrt(params.top_radius * params.top_radius - params.bottom_radius * params.bottom_radius);\n" \
    //     "float rho = H * x_r;\n" \
    //     "float r = sqrt(rho * rho + params.bottom_radius * params.bottom_radius);\n" \
    //     "float d_min = params.top_radius - r;\n" \
    //     "float d_max = rho + H;\n" \
    //     "float d = d_min + x_mu * (d_max - d_min);\n" \
    //     "mu = d == 0.0f ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * r * d);\n" \
    //     "return ClampCosine(mu);}\n" \

    //     "float ComputeOpticalLengthToTopAtmosphereBoundary(AtmosphereParameters params, DensityProfile profile, float r, float mu) {\n"
    //     "const int SAMPLE_COUNT = 500;\n" \
    //     "float dx = DistanceToTopAtmosphereBoundary(params, r, mu) / float(SAMPLE_COUNT);\n" \
    //     "float result = 0.0f;\n" \
    //     "for (int i = 0; i <= SAMPLE_COUNT; ++i) {\n" \
    //     "float d_i = float(i) * dx;\n" \
    //     "float r_i = sqrt(d_i * d_i + 2.0f * r * mu * d_i + r * r);\n" \
    //     "float y_i = GetProfileDensity(profile, r_i - params.bottom_radius);\n" \
    //     "result += y_i * dx * (i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f);}\n" \
    //     "return result;}\n" \

    //     "float3 ComputeTransmittanceToTopAtmosphereBoundary(AtmosphereParameters params, float r, float mu) {\n" \
    //     "return exp(-(\n" \
    //     "params.rayleigh_scattering * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.rayleigh_density, r, mu) +\n" \
    //     "params.mie_extinction * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.mie_density, r, mu) +\n" \
    //     "params.absorption_extinction * ComputeOpticalLengthToTopAtmosphereBoundary(params, params.absorption_density, r, mu)));}\n" \

    //     "float3 ComputeTransmittanceToTopAtmosphereBoundaryTexture(AtmosphereParameters params, float2 uv) {\n" \
    //     "float2 TRANSMITTANCE_TEXTURE_SIZE = float2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);\n" \
    //     "float r;\n" \
    //     "float mu;\n" \
    //     "GetRMuFromTransmittanceTextureUv(params, uv / TRANSMITTANCE_TEXTURE_SIZE, r, mu);\n" \
    //     "return	ComputeTransmittanceToTopAtmosphereBoundary(params, r, mu);}\n" \

    //     "float3 ComputeTransmittance(AtmosphereParameters params, float2 uv) {\n" \
    //     "return ComputeTransmittanceToTopAtmosphereBoundaryTexture(params, uv);}"
    // );
}