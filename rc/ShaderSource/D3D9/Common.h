const float2 Unit2 = { 1.0f, 1.0f };
const float3 Unit3 = { 1.0f, 1.0f, 1.0f };
const float3 Zero3 = { 0.0f, 0.0f, 0.0f };

float ClampDistance(float d)
{
    return max(d, 0.0f);
}

float SafeSqrt(float sqr)
{
    return sqrt(max(sqr, 0.0f));
}