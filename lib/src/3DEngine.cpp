#include "3DEngine.h"

#ifdef RENDERAPI_DX9
#include "Backend/D3D9/RenderBackendDX9.h"
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
    
}

C3DEngine::~C3DEngine()
{
#if defined(RENDERAPI_DX9)
    SAFE_DELETE(Global::m_pRenderBackend);
#endif
    SAFE_DELETE(Global::m_pDisplay);
    SAFE_DELETE(Global::m_pLog);
}

bool C3DEngine::Initialize(CInputListener *pExternalInputListener /* = nullptr */, 
	ICameraController *pExternalCameraController /* = nullptr */) 
{
    CLog *pLog = new CLog;
    if (pLog == nullptr)
        return false;
    pLog->SetLogToDebugger(true);
    pLog->SetLogToFile(true, "3DApp.log");
    Global::m_pLog = pLog;

    IDisplay *pDisplay = new CDisplayWindows;
    if (pDisplay == nullptr)
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create Display Windows");
    Global::m_pDisplay = pDisplay;

#if defined(RENDERAPI_DX9)
    CRenderBackendDX9 *pRenderBackendDX9 = new CRenderBackendDX9;
    if (pRenderBackendDX9 == nullptr)
        return false;
    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Create RenderBackend D3D9");
    Global::m_pRenderBackend = pRenderBackendDX9;
#endif

    if (!Global::m_pRenderBackend->Initialize(pDisplay))
        return false;

    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize 3DEngine");
    return true;
}

void C3DEngine::Run()
{
    IDisplay * __restrict pDisplay = Global::m_pDisplay;
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;

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

        Frame(nFrameId);

        ++nFrameId;
    }
}

void C3DEngine::RunOneFrame(dword nFrameId)
{
    EDeviceState devState = Global::m_pRenderBackend->CheckDeviceState();
    if (devState == EDeviceState::EDevState_Lost)
        return;
    else if (devState == EDeviceState::EDevState_Notreset) {
        if (!Global::m_pRenderBackend->HandleDeviceLost())
            return;
    }

    Frame(nFrameId);
}

void C3DEngine::Frame(dword nFrameId)
{
    IRenderBackend * __restrict pRenderBackend = Global::m_pRenderBackend;
    pRenderBackend->BeginRendering();
    pRenderBackend->EndRendering();
}