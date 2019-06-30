#pragma once

#include "Prereq.h"

class C3DEngine;
class CAnimationManager;
class CGUIManager;
class CGUIRenderer;
class CHardwareBufferManager;
class CInputListener;
class CJobSystem;
class CLog;
class CMaterialManager;
class CMeshManager;
class CShaderManager;
//class CTerrain;
class CTextureManager;
class CTimeOfDay;
class CTimerManager;
class IDisplay;
class IRenderBackend;
struct ShaderResourcesManager;

struct Global {
    DLL_EXPORT	static C3DEngine *m_p3DEngine;
	DLL_EXPORT	static CAnimationManager *m_pAnimationManager;
	DLL_EXPORT	static CGUIManager *m_pGUIManager;
	DLL_EXPORT	static CGUIRenderer *m_pGUIRenderer;
				static CHardwareBufferManager *m_pHwBufferManager;
	DLL_EXPORT	static CInputListener *m_pInputListener;
				static CJobSystem *m_pJobSystem;
	DLL_EXPORT	static CLog *m_pLog;
	DLL_EXPORT	static CMeshManager *m_pMeshManager;
	DLL_EXPORT	static CMaterialManager *m_pMaterialManager;
				static CShaderManager *m_pShaderManager;
	//DLL_EXPORT	static CTerrain *m_pTerrain;
	DLL_EXPORT	static CTextureManager *m_pTextureManager;
				static CTimeOfDay *m_pTimeOfDay;
				static CTimerManager *m_pTimerManager;
	DLL_EXPORT	static IDisplay *m_pDisplay;
				static IRenderBackend *m_pRenderBackend;
				static ShaderResourcesManager *m_pShaderResourceManager;

    DLL_EXPORT static bool IsMainThread();

	// DLL_EXPORT	static String s_WorkingDirectory;
};