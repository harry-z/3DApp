#include "3DEngine.h"
#include "JobSystem.h"

#ifdef RENDERAPI_DX9
#include "Backend/D3D9/RenderBackendDX9.h"
#include "Backend/D3D9/ShaderDX9.h"
#include "Backend/D3D9/TextureDX9.h"
#include "Backend/D3D9/HardwareBufferDX9.h"
#endif

#ifdef INPUTAPI_DINPUT
#include "Backend/DInput/InputListenerDInput.h"
#endif

#if TARGET_PLATFORM == PLATFORM_WINDOWS
#include "Windows/Display_Windows.h"
#endif

std::thread::id g_MainThreadId;

C3DEngine::C3DEngine()
: m_pMainCamera(nullptr)
, m_pSunShadowCamera(nullptr)
, m_pCameraController(nullptr)
, m_pSceneClipping(nullptr)
{
    g_MainThreadId = std::this_thread::get_id();
    Global::m_p3DEngine = this;
}

C3DEngine::~C3DEngine()
{
    DestroySceneClippingStrategy(m_pSceneClipping);

    CJobSystem *pJobSystem = Global::m_pJobSystem;
    if (pJobSystem)
    {
        DELETE_TYPE(pJobSystem, CJobSystem);
    }
    Global::m_pJobSystem = nullptr;

    if (m_pMainCamera)
    {
        DELETE_TYPE(m_pMainCamera, CCamera);
    }
    m_pMainCamera = nullptr;

    if (Global::m_pMaterialManager)
    {
        CMaterialManager *pMtlMgr = Global::m_pMaterialManager;
        DELETE_TYPE(pMtlMgr, CMaterialManager);
    }
    Global::m_pMaterialManager = nullptr;

#ifdef INPUTAPI_DINPUT
    if (Global::m_pInputListener)
    {
        CInputListenerDInput *pInputListenerDInput = static_cast<CInputListenerDInput*>(Global::m_pInputListener);
        DELETE_TYPE(pInputListenerDInput, CInputListenerDInput);
    }
#endif
    Global::m_pInputListener = nullptr;

#ifdef RENDERAPI_DX9
    if (Global::m_pHwBufferManager)
    {
        CHardwareBufferManagerDX9 *pHwBufferManagerDX9 = static_cast<CHardwareBufferManagerDX9*>(Global::m_pHwBufferManager);
        DELETE_TYPE(pHwBufferManagerDX9, CHardwareBufferManagerDX9);
    }
    if (Global::m_pTextureManager)
    {
        CTextureManagerDX9 *pTextureManagerDX9 = static_cast<CTextureManagerDX9*>(Global::m_pTextureManager);
        DELETE_TYPE(pTextureManagerDX9, CTextureManagerDX9);
    }
    if (Global::m_pShaderManager)
    {
        CShaderManagerDX9 *pShaderManagerDX9 = static_cast<CShaderManagerDX9*>(Global::m_pShaderManager);
        DELETE_TYPE(pShaderManagerDX9, CShaderManagerDX9);
    }
    if (Global::m_pRenderBackend)
    {
        CRenderBackendDX9 *pRenderBackendDX9 = static_cast<CRenderBackendDX9*>(Global::m_pRenderBackend);
        DELETE_TYPE(pRenderBackendDX9, CRenderBackendDX9);
    }
#endif
    Global::m_pHwBufferManager = nullptr;
    Global::m_pTextureManager = nullptr;
    Global::m_pShaderManager = nullptr;
    Global::m_pRenderBackend = nullptr;

#if TARGET_PLATFORM == PLATFORM_WINDOWS
    if (Global::m_pDisplay)
    {
        CDisplayWindows *pDisplayWindows = static_cast<CDisplayWindows*>(Global::m_pDisplay);
        DELETE_TYPE(pDisplayWindows, CDisplayWindows);
    }
#endif
    Global::m_pDisplay = nullptr;

    CLog *pLog = Global::m_pLog;
    if (pLog)
    {
        DELETE_TYPE(pLog, CLog);
    }
    Global::m_pLog = nullptr;

    Global::m_p3DEngine = nullptr;
}

bool C3DEngine::Initialize() 
{
    CLog *pLog = NEW_TYPE(CLog);
    pLog->SetLogToDebugger(true);
    pLog->SetLogToFile(true, "3DApp.log");
    Global::m_pLog = pLog;

#if TARGET_PLATFORM == PLATFORM_WINDOWS
    IDisplay *pDisplay = NEW_TYPE(CDisplayWindows);
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Display Windows");
#endif
    if (!pDisplay->Initialize())
        return false;
    Global::m_pDisplay = pDisplay;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize Display");

#ifdef RENDERAPI_DX9
    IRenderBackend *pRenderBackend = NEW_TYPE(CRenderBackendDX9);
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create RenderBackend D3D9");
    CShaderManager *pShaderManager = NEW_TYPE(CShaderManagerDX9);
    CTextureManager *pTextureManager = NEW_TYPE(CTextureManagerDX9);
    CHardwareBufferManager * pHwBufferManager = NEW_TYPE(CHardwareBufferManagerDX9);
#endif
    Global::m_pRenderBackend = pRenderBackend;
    Global::m_pShaderManager = pShaderManager;
    Global::m_pTextureManager = pTextureManager;
    Global::m_pHwBufferManager = pHwBufferManager;

    if (!pRenderBackend->Initialize(pDisplay))
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize RenderBackend");

    if (!pShaderManager->LoadShaders())
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Load Shaders");

    if (!pTextureManager->Initialize())
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize Textures");

    if (!pHwBufferManager->Initialize())
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize Predefined Vertex Layout");

#ifdef INPUTAPI_DINPUT
    CInputListener *pInputListener = NEW_TYPE(CInputListenerDInput);
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create InputListener DInput8");
#endif
    if (!pInputListener->Initialize(pDisplay))
        return false;
    Global::m_pInputListener = pInputListener;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize InputListener");

    Global::m_pMaterialManager = NEW_TYPE(CMaterialManager);

    m_pMainCamera = NEW_TYPE(CCamera);

    CJobSystem *pJobSystem = NEW_TYPE(CJobSystem);
    pJobSystem->Initialize();
    Global::m_pJobSystem = pJobSystem;

    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize 3DEngine");
    return true;
}

void C3DEngine::SetSceneClipping(const char *pszName)
{
    DestroySceneClippingStrategy(m_pSceneClipping);
    m_pSceneClipping = CreateSceneClippingStrategy(pszName);
}

void C3DEngine::SetExternalSceneClipping(ISceneClippingStrategy *pSceneClipping)
{
    DestroySceneClippingStrategy(m_pSceneClipping);
    m_pSceneClipping = pSceneClipping;
}

void C3DEngine::Run()
{
    IDisplay * __restrict pDisplay = Global::m_pDisplay;
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
    CInputListener * __restrict pInputListener = Global::m_pInputListener;

    dword nFrameId = 0;
    while (pDisplay->MessagePump())
    {
        EDeviceState devState = pRenderBackend->CheckDeviceState();
		if (devState == EDeviceState::EDevState_Lost)
			continue;
		else if (devState == EDeviceState::EDevState_Notreset) {
			if (!pRenderBackend->HandleDeviceLost())
				return;
		}

        pInputListener->Capture();

        Frame(nFrameId);

        ++nFrameId;
    }
}

void C3DEngine::RunOneFrame(dword nFrameId)
{
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
    CInputListener * __restrict pInputListener = Global::m_pInputListener;

    EDeviceState devState = pRenderBackend->CheckDeviceState();
    if (devState == EDeviceState::EDevState_Lost)
        return;
    else if (devState == EDeviceState::EDevState_Notreset) {
        if (!pRenderBackend->HandleDeviceLost())
            return;
    }

    pInputListener->Capture();

    Frame(nFrameId);
}

void C3DEngine::Frame(dword nFrameId)
{
    for (dword i = 0; i < ENodeListType_Count; ++i)
    {
        Linklist<IRenderNode>::_NodeType *pTemp = m_SceneNodelist[i].m_pRoot;
        while (pTemp != nullptr)
        {
            IRenderNode *pNode = pTemp->m_pOwner;
            pNode->UpdateWSBoundingBox();
        }

        if (m_pSceneClipping != nullptr)
            m_pSceneClipping->SceneClipping(m_pMainCamera, m_SceneNodelist[i]);
    }
    
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
    pRenderBackend->BeginRendering();
    pRenderBackend->EndRendering();
}

ISceneClippingStrategy* C3DEngine::CreateSceneClippingStrategy(const char *pszName)
{
    if (strcmp(pszName, SCENE_CLIPPING_DEFAULT) == 0)
        return NEW_TYPE(CDefaultSceneClippingStrategy);
    // else if (strcmp(pszName, SCENE_CLIPPING_OCTREE) == 0)
        // return NEW_TYPE()
    else
        return nullptr;
}

void C3DEngine::DestroySceneClippingStrategy(ISceneClippingStrategy *pClippingStrategy)
{
    if (pClippingStrategy != nullptr)
    {
        if (strcmp(pClippingStrategy->Name(), SCENE_CLIPPING_DEFAULT))
        {
            CDefaultSceneClippingStrategy *pDefaultClippingStrategy = (CDefaultSceneClippingStrategy *)pClippingStrategy;
            DELETE_TYPE(pDefaultClippingStrategy, CDefaultSceneClippingStrategy);
        }
    }
}