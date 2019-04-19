#include "3DEngine.h"
#include "Camera.h"
#include "JobSystem.h"

#ifdef RENDERAPI_DX9
#include "Backend/D3D9/RenderBackendDX9.h"
#endif

#ifdef INPUTAPI_DINPUT
#include "Backend/DInput/InputListenerDInput.h"
#endif

#if TARGET_PLATFORM == PLATFORM_WINDOWS
#include "Windows/Display_Windows.h"
#endif

C3DEngine::C3DEngine()
: m_pMainCamera(nullptr)
, m_pSunShadowCamera(nullptr)
, m_pCameraController(nullptr)
, m_bUseExternalInputListener(false)
, m_bUseExternalCameraController(false)
{
    Global::m_p3DEngine = this;
}

C3DEngine::~C3DEngine()
{
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

#ifdef INPUTAPI_DINPUT
    if (Global::m_pInputListener)
    {
        CInputListenerDInput *pInputListenerDInput = static_cast<CInputListenerDInput*>(Global::m_pInputListener);
        DELETE_TYPE(pInputListenerDInput, CInputListenerDInput);
    }
#endif
    Global::m_pInputListener = nullptr;

#ifdef RENDERAPI_DX9
    if (Global::m_pRenderBackend)
    {
        CRenderBackendDX9 *pRenderBackendDX9 = static_cast<CRenderBackendDX9*>(Global::m_pRenderBackend);
        DELETE_TYPE(pRenderBackendDX9, CRenderBackendDX9);
    }
#endif
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

bool C3DEngine::Initialize(CInputListener *pExternalInputListener /* = nullptr */, 
	ICameraController *pExternalCameraController /* = nullptr */) 
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
#endif
    if (!pRenderBackend->Initialize(pDisplay))
        return false;
    Global::m_pRenderBackend = pRenderBackend;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize RenderBackend");

#ifdef INPUTAPI_DINPUT
    CInputListener *pInputListener = NEW_TYPE(CInputListenerDInput);
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create InputListener DInput8");
#endif
    if (!pInputListener->Initialize(pDisplay))
        return false;
    Global::m_pInputListener = pInputListener;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize InputListener");

    m_pMainCamera = NEW_TYPE(CCamera);

    CJobSystem *pJobSystem = NEW_TYPE(CJobSystem);
    pJobSystem->Initialize();
    Global::m_pJobSystem = pJobSystem;

    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize 3DEngine");
    return true;
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
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
    pRenderBackend->BeginRendering();
    pRenderBackend->EndRendering();
}