#pragma once
#include "PixelFormatUtil.h"

enum class EHardwareBufferUsage {
	EUsage_Static,
	EUsage_Dynamic
};

enum class EHardwareBufferType {
	EHardwareBuffer_Vertex,
	EHardwareBuffer_Index
};

enum class EVertexUsage : word {
	EVertexUsage_PerVertex = 0,
	EVertexUsage_PerVertex_Skinning,
	EVertexUsage_Count
};

enum class EVertexType : byte {
	EVertexType_Float = 0,
	EVertexType_Float2,
	EVertexType_Float3,
	EVertexType_Float4,
	EVertexType_Color,
	EVertexType_UByte4,
	EVertexType_Short2,
	EVertexType_Short4
};

enum class EVertexSemantic : byte {
	EVertexSemantic_Position,
	EVertexSemantic_BlendWeight,
	EVertexSemantic_BlendIndices,
	EVertexSemantic_Normal,
	EVertexSemantic_Texcoord,
	EVertexSemantic_Tangent,
	EVertexSemantic_Binormal,
	EVertexSemantic_PositionT,
	EVertexSemantic_Color
};

enum class EPredefinedVertexLayout : byte {
	EPredefinedLayout_P = 0, // position only
	EPredefinedLayout_PT, // position and texture coordinate
	EPredefinedLayout_PTC, // position, texture coordinate and diffuse color
	EPredefinedLayout_PN, // position and normal
	EPredefinedLayout_PNC, // position, normal and diffuse color
	EPredefinedLayout_PNT, // position, normal and texture coordinate
	EPredefinedLayout_PNTANT, // position, normal, tangent and texture coordinate
	EPredefinedLayout_PNT_IDX_WEIGHT, // position, normal, texture coordinate, blend index and blend weight
	EPredefinedLayout_PNTANT_IDX_WEIGHT, // position, normal, tangent, texture coordinate, blend index and blend weight
	EPredefinedLayout_PTransformT, // transformed position, texture coordinate
	EPredefinedLayout_PNTANT1T2, // position, normal, tangent, texcoord0 and texcoord1
	EPredefinedLayout_PSHORT2T,
	EPredefinedLayout_Count
};

//////////////////////////////////////////////////////////////////////////
// Texture definition

enum class ETextureUsage : byte {
	ETextureUsage_Unknown = 0,
	ETextureUsage_Default,
	ETextureUsage_Dynamic,
	ETextureUsage_DynamicFixedSize,
	ETextureUsage_RenderTarget,
	ETextureUsage_RenderTargetFixedSize,
	ETextureUsage_DepthStencil,
	ETextureUsage_DepthStencilFixedSize
};

enum ETextureType : byte {
	ETextureType_Unknown = 0,
	ETextureType_1D = 0x01,
	ETextureType_2D = 0x02,
	ETextureType_3D = 0x04,
	ETextureType_Cube = 0x08,
	ETextureType_SRGB = 0x10
};

enum ECubeFace {
	ECubeFace_PosX = 0x01,
	ECubeFace_NegX = 0x02,
	ECubeFace_PosY = 0x04,
	ECubeFace_NegY = 0x08,
	ECubeFace_PosZ = 0x10,
	ECubeFace_NegZ = 0x20,
	ECubeFace_All = ECubeFace_PosX | ECubeFace_NegX | ECubeFace_PosY | ECubeFace_NegY | ECubeFace_PosZ | ECubeFace_NegZ
};

enum class EShaderType : byte {
	EShaderType_Vertex = 0,
	EShaderType_Pixel = 1,
	EShaderType_Unknown = 0xFF
};

enum class EShaderConstantType : byte {
	EShaderConstantType_Float = 0,
	EShaderConstantType_Int,
	EShaderConstantType_Unknown = 0xFF
};

enum EAutoUpdatedConstant : byte {
    EAutoUpdatedConstant_View = 0,
    EAutoUpdatedConstant_ViewProj,
    EAutoUpdatedConstant_InvView,
    EAutoUpdatedConstant_InvViewProj,
    EAutoUpdatedConstant_Proj,
    EAutoUpdatedConstant_InvProj,
    EAutoUpdatedConstant_CamPos,
    EAutoUpdatedConstant_CamDir,
	EAutoUpdatedConstant_NearFar,
	EAutoUpdatedConstant_Num,
	EAutoUpdatedConstant_None = 0xFF
};

enum EClearFlag {
	EClearFlag_Color = 0x01,
	EClearFlag_Depth = 0x02,
	EClearFlag_Stencil = 0x04
};

enum class EPrimitiveType : byte {
	EPrimType_PointList = 1,
	EPrimType_LineList,
	EPrimType_LineStrip,
	EPrimType_TriangleList,
	EPrimType_TriangleStrip
};

enum class EDeviceState {
	EDevState_Ok,
	EDevState_Lost,
	EDevState_Notreset,
	EDevState_InternalError
};

enum EShadingModel : char {
	ESM_Unknown = -1,
	ESM_NoIllum = 0, 
	ESM_NoIllumTranslucent,
	ESM_Illum,
	ESM_IllumTranslucent
};

#define MTL_MAX_TEXLAYER_COUNT 8

#define RENDER_PATH_FORWARD_SHADING 1
#define CURRENT_RENDER_PATH RENDER_PATH_FORWARD_SHADING

#include "Vector.h"

struct P3
{
	Vec3 m_Position;
};

struct P3T2
{
	Vec3 m_Position;
	Vec2 m_Texcoord;
};

struct P3T2C
{
	Vec3 m_Position;
	Vec2 m_Texcoord;
	dword m_Color;
};

struct P3N3
{
	Vec3 m_Position;
	Vec3 m_Normal;
};

struct P3N3C
{
	Vec3 m_Position;
	Vec3 m_Normal;
	dword m_Color;
};

struct P3N3T2
{
	Vec3 m_Position;
	Vec3 m_Normal;
	Vec2 m_Texcoord;
};

struct P3N3TAN3T2
{
	Vec3 m_Position;
	Vec3 m_Normal;
	Vec3 m_Tangent;
	Vec2 m_Texcoord;
};

struct BlendIdxWeight
{
	byte m_BlendIndex[4];
	float m_BlendWeight[4];
};

struct P3N3TAN3T2T2
{
	Vec3 m_Position;
	Vec3 m_Normal;
	Vec3 m_Tangent;
	Vec2 m_Texcoord0;
	Vec2 m_Texcoord1;
};

struct PSHORT2T2
{
	short m_Position[2];
	Vec2 m_Texcoord;
};