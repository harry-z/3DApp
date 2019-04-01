#include "3DEngine.h"

#ifdef RENDERAPI_DX9
#include "Backend/D3D9/RenderBackendDX9.h"
#endif

#if TARGET_PLATFORM == PLATFORM_WINDOWS
#include "Windows\Display_Windows.h"
#endif

C3DEngine::C3DEngine()
: m_pMainCamera(nullptr)
, m_pSunShadowCamera(nullptr)
, m_pCameraController(nullptr)
, m_bUseExternalInputListener(false)
, m_bUseExternalCameraController(false)
{
#if defined(RENDERAPI_DX9)
    CRenderBackendDX9 *pRenderBackendDX9 = new CRenderBackendDX9;
    Global::m_pRenderBackend = pRenderBackendDX9;
#endif
}

C3DEngine::~C3DEngine()
{
#if defined(RENDERAPI_DX9)
    delete Global::m_pRenderBackend;
#endif
    delete Global::m_pDisplay;
}

bool C3DEngine::Initialize(CInputListener *pExternalInputListener /* = nullptr */, 
	ICameraController *pExternalCameraController /* = nullptr */) 
{
    IDisplay *pDisplay = new CDisplayWindows;
    Global::m_pDisplay = pDisplay;
    if (!Global::m_pRenderBackend->Initialize(pDisplay))
        return false;
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