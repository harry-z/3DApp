#pragma once
#include "Prereq.h"
#include "RendererTypes.h"
#include "ReferencedObject.h"

class IHardwareBuffer;
class IVertexLayout;
class CTexture;

#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING

enum EForwardShading_ShaderBatch {
	EForwardShading_ShaderBatch_ShadowDepth,
	EForwardShading_ShaderBatch_TransluentShadowDepth,
	EForwardShading_ShaderBatch_ShadowMask,
	EForwardShading_ShaderBatch_TransluentShadowMask,
	EForwardShading_ShaderBatch_SceneDepth,
	EForwardShading_ShaderBatch_Sky,
	//EShaderBatch_Terrain,
	EForwardShading_ShaderBatch_Opaque,
	EForwardShading_ShaderBatch_Transluent,
	EForwardShading_ShaderBatch_3DCount,
	EForwardShading_ShaderBatch_GUI = EForwardShading_ShaderBatch_3DCount,
	EForwardShading_ShaderBatch_Count
};

#endif

struct RenderObject
{
    CArray<IHardwareBuffer*> m_arrHwBuffer;
    IHardwareBuffer *m_pIB = nullptr;
    IVertexLayout *m_pVertexLayout = nullptr;
    Matrix4 *m_pWorldTransform = nullptr;

	dword m_nVertexCount;
	dword m_nPrimitiveCount;
	EPrimitiveType m_PrimType;

	dword m_nFlag = 0;

	static CPool m_RenderObjectPool;

	static void Initialize() 
	{ 
		m_RenderObjectPool.Initialize(sizeof(RenderObject)); 
	}
	static void Uninitialize() 
	{ 
		m_RenderObjectPool.Uninitialize(); 
	}
	static RenderObject* CreateRenderObject() 
	{
		return new (m_RenderObjectPool.Allocate()) RenderObject;
	}
	static void DestroyRenderObject(RenderObject *pRenderObject) 
	{
		pRenderObject->~RenderObject();
		m_RenderObjectPool.Free(pRenderObject);
	}
};

struct ShaderVariable
{
	EShaderConstantType m_Type;
	dword m_nRegisterIndex;
    dword m_nLengthInBytes;
    dword m_nOffsetInBytes;
	void *m_pData = nullptr;
};

struct ShaderObject
{
	CArray<ShaderVariable> m_arrShaderVar;
	CArray<IdString> m_arrAutoShaderVar;

	static CPool m_ShaderObjectPool;
	static void Initialize() 
	{ 
		m_ShaderObjectPool.Initialize(sizeof(ShaderObject)); 
	}
	static void Uninitialize() 
	{ 
		m_ShaderObjectPool.Uninitialize(); 
	}
	static ShaderObject* CreateShaderObject() 
	{
		return new (m_ShaderObjectPool.Allocate()) ShaderObject;
	}
	static void DestroyShaderObject(ShaderObject *pShaderObject) 
	{
		pShaderObject->~ShaderObject();
		m_ShaderObjectPool.Free(pShaderObject);
	}
};

struct ShaderResources;
struct RenderItem
{
	float m_Distance = 0.0f;
	ldword m_nSortVal;

	RenderObject *m_pRenderObj = nullptr;
	ShaderObject *m_pVSShaderObj = nullptr;
	ShaderObject *m_pPSShaderObj = nullptr;
	ShaderResources *m_pShaderResources = nullptr;

	using RenderItems = CArray<RenderItem>;
#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
	static RenderItems m_RenderItems[EForwardShading_ShaderBatch_Count];
#endif

	static void Initialize() 
	{
#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
		for (dword i = 0; i < EForwardShading_ShaderBatch_Count; ++i) {
			if (i == EForwardShading_ShaderBatch_Sky)
				m_RenderItems[EForwardShading_ShaderBatch_Sky].Reserve(1);
			//else if (i == EShaderBatch_Terrain)
			//	m_RenderItems[EShaderBatch_Terrain].Reserve(9);
			else
				m_RenderItems[i].Reserve(1024);
		}
#endif
	}

	static void Uninitialize()
	{
#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
		for (dword i = 0; i < EForwardShading_ShaderBatch_Count; ++i)
			m_RenderItems[i].Free();
#endif
	}

	static RenderItem* AddItem(dword nList) 
	{
		RenderItem *pItem = m_RenderItems[nList].AddIndex(1);
		new (pItem) RenderItem();
		return pItem;
	}

	static void ResetItems() 
	{
#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
		for (dword i = 0; i < EForwardShading_ShaderBatch_Count; ++i)
			m_RenderItems[i].SetNum(0);
#endif
	}
};

inline ldword VertexShaderId(ldword nId) { return nId << 52; }
inline ldword GetVertexShaderId(ldword nId) { return (nId >> 52) & 0x0000000000000FFF; }

inline ldword PixelShaderId(ldword nId) { return nId << 40; }
inline ldword GetPixelShaderId(ldword nId) { return (nId >> 40) & 0x0000000000000FFF; }

inline ldword ShaderResourceId(ldword nId) { return nId << 24; }
inline ldword GetShaderResourceId(ldword nId) { return (nId >> 24) & 0x000000000000FFFF; }

inline ldword SortVal(ldword nVSId, ldword nPSId, ldword nSRId) { return VertexShaderId(nVSId) | PixelShaderId(nPSId) | ShaderResourceId(nSRId); }