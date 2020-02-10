struct VSInputP2TC2
{
	float2 Position : POSITION;
	float2 Texcoord0 : TEXCOORD0;
};

struct VSInputP3
{
	float3 Position : POSITION;
};

struct VSInputP3TC2
{
	float3 Position : POSITION;
	float2 Texcoord0 : TEXCOORD0;
};

struct VSOutputP4
{
	float4 Position : POSITION;
};

struct VSOutputP4TC2
{
	float4 Position : POSITION;
	float2 Texcoord0 : TEXCOORD0;
};

struct VSOutputP4TC3
{
	float4 Position : POSITION;
	float3 Texcoord0 : TEXCOORD0;	
};

#define POSITION_ELEM(Struct) Struct.Position
#define TEXCOORD_ELEM(Struct, Channel) Struct.Texcoord##Channel



float4x4 WorldViewProjectionMatrix;
float4x4 WorldViewMatrix;
float2 NearFarClip;