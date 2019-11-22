#include "VertexLib.h"
#include "ShadingModel.h"

VSOutputP4 VS_NoIllumColor(VSInputP3 Input) 
{
	VSOutputP4 OutputP4;
	POSITION_ELEM(OutputP4) = mul(WorldViewProjectionMatrix, float4(POSITION_ELEM(Input), 1.0f));
	return OutputP4;
}

VSOutputP4TC2 VS_NoIllum(VSInputP3TC2 Input)
{
	VSOutputP4TC2 OutputP4TC2;
	POSITION_ELEM(OutputP4TC2) = mul(WorldViewProjectionMatrix, float4(POSITION_ELEM(Input), 1.0f));
	TEXCOORD_ELEM(OutputP4TC2, 0) = TEXCOORD_ELEM(Input, 0);
	return OutputP4TC2;
}

float3 Albedo;
float Alpha;
sampler2D AlbedoTexture;

float4 PS_NoIllumColor() : COLOR0
{
	return NoIllumOpaque(Albedo);
}

float4 PS_NoIllum(VSOutputP4TC2 Input) : COLOR0
{
	return NoIllumOpaque(tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0)).xyz);
}

float4 PS_NoIllumBlended(VSOutputP4TC2 Input) : COLOR0
{
	return NoIllumOpaque(tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0)).xyz * Albedo);
}

float4 PS_NoIllumColorTranslucent() : COLOR0
{
	return NoIllum(Albedo, Alpha);
}

float4 PS_NoIllumTranslucent(VSOutputP4TC2 Input) : COLOR0
{
	float4 c = tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0));
	return NoIllum(c.xyz, c.w * Alpha);
}

float4 PS_NoIllumBlendedTranslucent(VSOutputP4TC2 Input) : COLOR0
{
	float4 c = tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0));
	return NoIllum(c.xyz * Albedo, c.w * Alpha);
}