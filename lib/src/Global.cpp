#include "Global.h"
// #include "AnimationManager.h"
// #include "GUI.h"
// #include "InputListener.h"
// #include "JobSystem.h"
#include "3DEngine.h"
#include "Display.h"
// #include "HardwareBuffer.h"
// #include "Material.h"
// #include "Mesh.h"
// #include "Shader.h"
// //#include "Terrain.h"
// #include "Texture.h"
// #include "RenderBackend.h"
// #include "TimeOfDay.h"


C3DEngine* Global::m_p3DEngine = nullptr;
// CAnimationManager* Global::m_pAnimationManager = nullptr;
// CGUIManager* Global::m_pGUIManager = nullptr;
// CGUIRenderer* Global::m_pGUIRenderer = nullptr;
// CHardwareBufferManager* Global::m_pHwBufferManager = nullptr;
// CInputListener* Global::m_pInputListener = nullptr;
// CJobSystem* Global::m_pJobSystem = nullptr;
// CLog* Global::m_pLog = nullptr;
// CMaterialManager* Global::m_pMaterialManager = nullptr;
// CMeshManager* Global::m_pMeshManager = nullptr;
// CShaderManager* Global::m_pShaderManager = nullptr;
// //CTerrain* Global::m_pTerrain = nullptr;
// CTextureManager* Global::m_pTextureManager = nullptr;
// CTimeOfDay* Global::m_pTimeOfDay = nullptr;
// CTimerManager* Global::m_pTimerManager = nullptr;
IDisplay* Global::m_pDisplay = nullptr;
IRenderBackend* Global::m_pRenderBackend = nullptr;
// //ShaderGlobalParams* Global::m_pShaderGlobalParams = nullptr;
// ShaderResourcesManager* Global::m_pShaderResourceManager = nullptr;


// extern ThreadId g_MainThreadId;
// bool Global::IsMainThread() {
    // return g_MainThreadId == GetCurrThread();
// }

// String Global::s_WorkingDirectory;