#pragma once

#include "PreHeader.h"

#ifdef RENDERAPI_DX9
	#ifdef _DEBUG
		#define D3D_DEBUG_INFO
	#endif
	#include <d3d9.h>
	#include <d3dcompiler.h>

	extern IDirect3DDevice9 *g_pDevice9;
	extern D3DCAPS9 *g_pCaps9;

// #elif defined(RENDERAPI_GLES20) || defined(RENDERAPI_GLES20_EMU)

// #ifdef RENDERAPI_GLES20
// 	#include <EGL/egl.h>
// 	#include <GLES2/gl2.h>
// 	#include <GLES2/gl2ext.h>
// #elif defined(RENDERAPI_GLES20_EMU)
// 	#include "EGL/egl.h"
// 	#include "GLES2/gl2.h"
// 	#include "GLES2/gl2ext.h"
// 	#define DIRECTINPUT_VERSION 0x0800
// 	#include <dinput.h>

// 	#pragma comment(lib, "libEGL.lib")
// 	#pragma comment(lib, "libGLESv2.lib")
// 	#pragma comment(lib, "dxguid.lib")
// 	#pragma comment(lib, "dinput8.lib")
// #endif
	
// 	struct GLuintWrapper {
// 		GLuintWrapper() : m_field(0), m_code(0) {}

// 		GLuintWrapper& operator= (GLuint u) { m_field = u; return *this; }
// 		operator GLuint() const { return m_field; }
		
// 		bool IsValid() const { return m_field != 0; }
// 		dword Code() const { return m_code; }

// 		GLuint m_field;
// 		dword m_code;
// 	};

#endif

#ifdef INPUTAPI_DINPUT
	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>
#endif
	
/*#define SHADER_VS_UNIFORM_CAMERA_SLOT 0
#define SHADER_VS_UNIFORM_TRANSFORM_SLOT 1
#define SHADER_VS_UNIFORM_SKELETON_TRANSFORM_SLOT 9
#define SHADER_VS_UNIFORM_CUSTOM_SLOT 202

#define SHADER_PS_UNIFORM_LIGHTING_SLOT 0
#define SHADER_PS_UNIFORM_SURFACE_PARAMS_SLOT 3
#define SHADER_PS_UNIFORM_TEX_MODIFIER_SLOT 6
#define SHADER_PS_UNIFORM_VIEWPORT_DIM_SLOT 8
#define SHADER_PS_UNIFORM_TIME_SLOT 9
#define SHADER_PS_UNIFORM_CUSTOM_SLOT 10

#endif

#define ENABLE_SHADER_DEBUGGING 1

// Transform matrix used in shader programs, including
// two matrices. Any matrices can be set to this two slots
#define SHADER_VS_UNIFORM_FLAG_TRANSFORM									0x01
// Camera transformation used in shader programs, including
// world space camera position and direction. 
#define SHADER_VS_UNIFORM_FLAG_CAMERA										0x02
// Skeleton transform info, including transform info of every bone
// in skeleton
#define SHADER_VS_UNIFORM_FLAG_SKELETON_TRANSFORM							0x04

// Lighting params used in shader programs, including
// world space light direction, light color and global ambient. 
#define SHADER_PS_UNIFORM_FLAG_GLOBAL_LIGHTING								0x01
// Object surface params used in shader programs, including
// surface albedo color, specular color and glossiness
#define SHADER_PS_UNIFORM_FLAG_SURFACE_PARAMS								0x02
// Texture coordinate modifier, including UV offset, UV delta offset, 
// UV rotation and UV delta rotation
#define SHADER_PS_UNIFORM_FLAG_TEX_MODIFIER									0x04
// Viewport dimension, including viewport width and height,
// inversed viewport width and height
#define SHADER_PS_UNIFORM_FLAG_VIEWPORT_DIM									0x08
// Time information, including elapsed time and frame time 
// interval
#define SHADER_PS_UNIFORM_FLAG_TIME											0x10

enum EPredefinedShaderUniform {
	EPredefinedUniform_Transform = 0,
	EPredefinedUniform_Skeleton,
	EPredefinedUniform_Camera,
	EPredefinedUniform_Lighting,
	EPredefinedUniform_Surface,
	EPredefinedUniform_UV,
	EPredefinedUniform_Viewport,
	EPredefinedUniform_Time,
	EPredefinedUniform_Count
};*/