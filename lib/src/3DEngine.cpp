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
    SAFE_DELETE(m_pMainCamera);
#ifdef RENDERAPI_DX9
    SAFE_DELETE(Global::m_pRenderBackend);
#endif
#ifdef INPUTAPI_DINPUT
    SAFE_DELETE(Global::m_pInputListener);
#endif
    SAFE_DELETE(Global::m_pDisplay);
    SAFE_DELETE(Global::m_pJobSystem);
    SAFE_DELETE(Global::m_pLog);

    Global::m_p3DEngine = nullptr;
}

bool C3DEngine::Initialize(CInputListener *pExternalInputListener /* = nullptr */, 
	ICameraController *pExternalCameraController /* = nullptr */) 
{
    CLog *pLog = new CLog;
    pLog->SetLogToDebugger(true);
    pLog->SetLogToFile(true, "3DApp.log");
    Global::m_pLog = pLog;

    IDisplay *pDisplay = new CDisplayWindows;
    if (!pDisplay->Initialize())
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Display Windows");
    Global::m_pDisplay = pDisplay;

#ifdef RENDERAPI_DX9
    CRenderBackendDX9 *pRenderBackendDX9 = new CRenderBackendDX9;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create RenderBackend D3D9");
    Global::m_pRenderBackend = pRenderBackendDX9;
#endif

    if (!Global::m_pRenderBackend->Initialize(pDisplay))
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize RenderBackend");

    m_pMainCamera = new CCamera;

#ifdef INPUTAPI_DINPUT
    CInputListenerDInput *pInputDInput = new CInputListenerDInput;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create InputListener DInput8");
    Global::m_pInputListener = pInputDInput;
#endif
    if (!Global::m_pInputListener->Initialize(pDisplay))
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize InputListener");

    CJobSystem *pJobSystem = new CJobSystem;
    pJobSystem->Initialize();

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