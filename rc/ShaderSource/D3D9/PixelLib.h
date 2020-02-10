struct PSSRTOutput 
{
    float4 Color0 : COLOR0;
};

struct PSMRTOutput2
{
    float4 Color0 : COLOR0;
    float4 Color1 : COLOR1;
};

struct PSMRTOutput3
{
    float4 Color0 : COLOR0;
    float4 Color1 : COLOR1;
    float4 Color2 : COLOR2;
};

struct PSMRTOutput4
{
    float4 Color0 : COLOR0;
    float4 Color1 : COLOR1;
    float4 Color2 : COLOR2;
    float4 Color3 : COLOR3;
};

#define OUTPUT_CHANNEL(Struct, Index) Struct.Color##Index