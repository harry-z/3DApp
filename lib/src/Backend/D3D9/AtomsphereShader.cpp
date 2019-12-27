#include "AtomsphereRendererDX9.h"

String CAtomsphereRendererDX9::ShaderHeader() const
{
    // constexpr dword cLayerCount = 2;
    // const double lambdas[] = { 680.0, 550.0, 440.0 };
    // // const Vec3 lambdas(680.0f, 550.0f, 440.0f);

    // CArray<DensityProfileLayer> arrRayleighDensity = params.m_arrRayleighDensity;
    // if (arrRayleighDensity.Num() < cLayerCount)
    // {
    //     while (arrRayleighDensity.Num() < cLayerCount)
    //         arrRayleighDensity.Add(DensityProfileLayer());
    // }

    // CArray<DensityProfileLayer> arrMieDensity = params.m_arrMieDensity;
    // if (arrMieDensity.Num() < cLayerCount)
    // {
    //     while (arrMieDensity.Num() < cLayerCount)
    //         arrMieDensity.Add(DensityProfileLayer());
    // }

    // CArray<DensityProfileLayer> arrAbsorptionDensity = params.m_arrAbsorptionDensity;
    // if (arrAbsorptionDensity.Num() < cLayerCount)
    // {
    //     while (arrAbsorptionDensity.Num() < cLayerCount)
    //         arrAbsorptionDensity.Add(DensityProfileLayer());
    // }

    // auto ToFloat3String = [this](const AtmosphereParams &params, const CArray<double> &v, const double *lambdas, double scale) -> String {
    //     double r = Interpolate(params.m_arrWaveLength, v, lambdas[0]) * scale;
    //     double g = Interpolate(params.m_arrWaveLength, v, lambdas[1]) * scale;
    //     double b = Interpolate(params.m_arrWaveLength, v, lambdas[2]) * scale;
    //     return String("float3(") + ToString(r) + "," + ToString(g) + "," + ToString(b) + ")";
    // };

    // auto ToDensityProfileString = [](const DensityProfileLayer &layer) -> String {
    //     char szParam[CSTR_MAX];
    //     sprintf(szParam, "%f,%f,%f,%f,%f", 
    //         layer.m_Width, 
    //         layer.m_ExpTerm, 
    //         layer.m_ExpScale, 
    //         layer.m_LinearTerm, 
    //         layer.m_ConstantTerm);
    //     return String(szParam);
    // };

    return String(
        "const float2 Unit2 = { 1.0f, 1.0f };\n" \
        "const float3 Unit3 = { 1.0f, 1.0f, 1.0f };\n" \
        "const float3 Zero3 = { 0.0f, 0.0f, 0.0f };\n" \

        "float ClampDistance(float d) {\n" \
        "return max(d, 0.0f);}\n" \

        "float SafeSqrt(float a) {\n" \
        "return sqrt(max(a, 0.0f));}\n" \
    );
}

String CAtomsphereRendererDX9::VertexShader() const
{
    return String(
        "struct VSOutput {\n" \
        "float4 Position : POSITION;\n" \
        "float2 Texcoord : TEXCOORD0;};\n" \
        "VSOutput DrawQuad(float2 Position : POSITION){\n" \
        "VSOutput o;\n" \
        "o.Position = float4(Position, 0.0f, 1.0f);\n" \
        "o.Texcoord = (Position * float2(1.0f, -1.0f) + float2(1.0f, 1.0f)) * 0.5f;\n" \
        "return o;}"
    );
}

String CAtomsphereRendererDX9::TransmittanceShader() const
{
    return String(
        "const int SAMPLE_COUNT = 500;\n"\
        "float2 TextureDim;\n" \
        "float4 TopBottomRadius;\n" \
        "float3 RayleighScattering;\n" \
        "float3 MieExtinction;\n" \
        "float3 AbsorptionExtinction;\n" \
        "float3 RMAExpTerm[2];\n" \
        "float3 RMAExpScale[2];\n" \
        "float3 RMALinearTerm[2];\n" \
        "float3 RMAConstTerm[2];\n" \
        "float3 RMAWidth;\n" \
        "float3 DistanceToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float discriminant = MU_R_Sqr.w * (MU_R_Sqr.z - 1.0f) + TopBottomRadius.z;\n" \
        "return ClampDistance(- MU_R_Sqr.y * MU_R_Sqr.x + SafeSqrt(discriminant)).xxx;}\n" \
        "float3 GetLayerDensity(float3 altitude, int index){\n" \
        "float3 density = RMAExpTerm[index] * exp(RMAExpScale[index] * altitude) + RMALinearTerm[index] * altitude + RMAConstTerm[index];\n" \
        "return clamp(density, Zero3, Unit3);}\n" \
        "float3 GetProfileDensity(float3 altitude){\n" \
        "float3 ratio = clamp(sign(altitude - RMAWidth), Zero3, Unit3);\n" \
        "return lerp(GetLayerDensity(altitude, 0), GetLayerDensity(altitude, 1), ratio);}\n" \
        "float4 GetRMuFromTransmittanceTextureUv(float2 UV){\n" \
        "float4 MU_R_Sqr;\n" \
        "float2 X_MU_R = (UV - Unit2 / TextureDim * 0.5f) / (Unit2 - Unit2 / TextureDim);\n" \
        "float H = sqrt(TopBottomRadius.z - TopBottomRadius.w);\n" \
        "float rho = H * X_MU_R.y;\n" \
        "MU_R_Sqr.y = sqrt(rho * rho + TopBottomRadius.w);\n" \
        "float d_min = TopBottomRadius.x - MU_R_Sqr.y;\n" \
        "float d_max = rho + H;\n" \
        "float d = d_min + X_MU_R.x * (d_max - d_min);\n" \
        "MU_R_Sqr.x = d == 0.0f ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * MU_R_Sqr.y * d);\n" \
        "MU_R_Sqr.zw = MU_R_Sqr.xy * MU_R_Sqr.xy;\n" \
        "return MU_R_Sqr;}\n" \
        "float3 ComputeOpticalLengthToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float3 dx = DistanceToTopAtmosphereBoundary(MU_R_Sqr) / SAMPLE_COUNT;\n" \
        "float3 result = Zero3;\n" \
        "for (int i = 0; i <= SAMPLE_COUNT; ++i){\n" \
        "float3 d_i = dx * i;\n" \
        "float3 r_i = sqrt(d_i * d_i + d_i * MU_R_Sqr.x * MU_R_Sqr.y * 2.0f + MU_R_Sqr.www);\n" \
        "float3 y_i = GetProfileDensity(r_i - TopBottomRadius.yyy);\n" \
        "result += y_i * dx * (i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f);}\n" \
        "return result;}\n" \
        "float3 ComputeTransmittanceToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float3 OpticalLength = ComputeOpticalLengthToTopAtmosphereBoundary(MU_R_Sqr);\n" \
        "float3 Transmittance = RayleighScattering * OpticalLength.x + \n" \
        "MieExtinction * OpticalLength.y + \n" \
        "AbsorptionExtinction * OpticalLength.z;\n" \
        "return exp(-Transmittance);}\n" \
        "float4 ComputeTransmittanceToTopAtmosphereBoundaryTexture(float2 UV : TEXCOORD0) : COLOR0{\n" \
        "float4 MU_R_Sqr = GetRMuFromTransmittanceTextureUv(UV);\n" \
        "return float4(ComputeTransmittanceToTopAtmosphereBoundary(MU_R_Sqr), 1.0f);}\n"
    );
}