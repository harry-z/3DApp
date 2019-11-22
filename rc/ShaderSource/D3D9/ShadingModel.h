float4 NoIllum(float3 Albedo, float Alpha)
{
    return float4(Albedo, Alpha);
}

float4 NoIllumOpaque(float3 Albedo)
{
    return NoIllum(Albedo, 1.0f);
}