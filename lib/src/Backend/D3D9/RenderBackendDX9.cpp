// #include "tbb_header.h"
#include "GPUResource.h"
#include "RenderBackendDX9.h"
#include "..\..\Windows\IDisplay_Windows.h"

//CRenderBackendDX9* CRenderBackendDX9::s_me = nullptr;

IDirect3DDevice9 *g_pDevice9;
D3DCAPS9 *g_pCaps9;

CRenderBackendDX9::CRenderBackendDX9() 
: m_pD3D9(nullptr), 
m_pD3DDevice9(nullptr) {
	memset(&m_dpparams, 0, sizeof(D3DPRESENT_PARAMETERS));
//#if TARGET_ARCHITECTURE == ARCHITECTURE_X86
//	m_GpuObjsPool[ESize16].Initialize(16, 100);
//	m_GpuObjsPool[ESize32].Initialize(32, 100);
//	m_GpuObjsPool[EGpuResource].Initialize(16, 100);
//#elif TARGET_ARCHITECTURE == ARCHITECTURE_X64
//	m_GpuObjsPool[ESize32].Initialize(32, 100);
//	m_GpuObjsPool[ESize64].Initialize(64, 100);
//	m_GpuObjsPool[EGpuResource].Initialize(32, 100);
//#endif
}

CRenderBackendDX9::~CRenderBackendDX9() {
	Shutdown();
}

bool CRenderBackendDX9::Initialize(IDisplay *pDisplay) {
	CLog *pLog = Global::m_pLog;
	IDisplayWindows *pDisplayWindows = (IDisplayWindows *)(pDisplay);

	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D9 == nullptr) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not create IDirect3D9 object");
		return false;
	}
	if (!IsDepthFormatOk(D3DFMT_X8R8G8B8, D3DFMT_D24S8)) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not use D24S8 depth format with the current back buffer");
		return false;
	}
	/*DWORD dwMaxQuality = 0;
	dword nMultiSampleQuality = m_cvars.m_nMSAAQuality;
	dword nMultiSampleCount = max(0, min(m_cvars.m_nMSAACount, 16));
	if (!IsMultiSampleTypeOk((D3DMULTISAMPLE_TYPE)nMultiSampleCount, D3DFMT_X8R8G8B8, 
		pDisplayWindows->IsFullScreen(), dwMaxQuality)) {
		pLog->Log(eLogType_Error, eLogFlag_Critical, "Can not use the desired multi sample type on this graphic card");
		return false;
	}*/

	dword nBackBufferWidth, nBackBufferHeight;
	pDisplayWindows->GetDimension(nBackBufferWidth, nBackBufferHeight);
	m_dpparams.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_dpparams.BackBufferCount = 1;
	m_dpparams.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_dpparams.BackBufferHeight = nBackBufferHeight;
	m_dpparams.BackBufferWidth = nBackBufferWidth;
	m_dpparams.EnableAutoDepthStencil = TRUE;
	m_dpparams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	m_dpparams.FullScreen_RefreshRateInHz = pDisplayWindows->IsFullScreen() ? 60 : 0;
	m_dpparams.hDeviceWindow = pDisplayWindows->GetHWnd();
	m_dpparams.MultiSampleQuality = 0; // nMultiSampleQuality > dwMaxQuality ? dwMaxQuality - 1 : nMultiSampleQuality;
	m_dpparams.MultiSampleType = D3DMULTISAMPLE_NONE;  //(D3DMULTISAMPLE_TYPE)nMultiSampleCount;
	m_dpparams.PresentationInterval = pDisplayWindows->IsFullScreen() ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_DEFAULT;
	m_dpparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_dpparams.Windowed = !pDisplayWindows->IsFullScreen();
	HRESULT hr = m_pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		pDisplayWindows->GetHWnd(), D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_dpparams, &m_pD3DDevice9);
	if (FAILED(hr)) {
		pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Can not create D3D9 device");
		return false;
	}
	g_pDevice9 = m_pD3DDevice9;

	m_pD3DDevice9->GetRenderTarget(0, &m_pBackbuffer);
	m_pD3DDevice9->GetDepthStencilSurface(&m_pDepthStencil);

	m_pDeviceCaps9 = NEW_TYPE(D3DCAPS9);
	memset(m_pDeviceCaps9, 0, sizeof(D3DCAPS9));
	m_pD3DDevice9->GetDeviceCaps(m_pDeviceCaps9);
	g_pCaps9 = m_pDeviceCaps9;

	pDisplayWindows->AddDisplayObserver(this);

	// InitializePredefinedVertexLayouts();
	//InitializePredefinedStates();
	//InitializePredefinedTextures();

	//m_Cache.Initialize(this);

	pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize RenderBackend D3D9");

	return true;
}

void CRenderBackendDX9::Shutdown() {
	// UninitializePredefinedVertexLayouts();
	//UninitializePredefinedStates();
	//UninitializePredefinedTextures();
	if (m_pDeviceCaps9)
	{
		DELETE_TYPE(m_pDeviceCaps9, D3DCAPS9);
		m_pDeviceCaps9 = nullptr;
	}
	SAFE_RELEASE(m_pBackbuffer);
	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_pD3DDevice9);
	SAFE_RELEASE(m_pD3D9);
}

void CRenderBackendDX9::BeginRendering() {
	m_Cache.Reset();
	m_pD3DDevice9->BeginScene();

	for (dword i = 0; i < MTL_MAX_TEXLAYER_COUNT; ++i) {
		m_pD3DDevice9->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice9->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pD3DDevice9->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	}

	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	m_pD3DDevice9->SetSamplerState(MTL_MAX_TEXLAYER_COUNT, D3DSAMP_BORDERCOLOR, D3DCOLOR_XRGB(255, 255, 255));

	RestoreTarget();
	ClearTarget((int)EClearFlag::EClearFlag_Color | (int)EClearFlag::EClearFlag_Depth | (int)EClearFlag::EClearFlag_Stencil, 
		D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
}

void CRenderBackendDX9::EndRendering() {
	m_pD3DDevice9->EndScene();
	m_pD3DDevice9->Present(nullptr, nullptr, nullptr, nullptr);
}

EDeviceState CRenderBackendDX9::CheckDeviceState() {
	HRESULT hr = m_pD3DDevice9->TestCooperativeLevel();
	switch (hr)
	{
	case D3D_OK:
		return EDeviceState::EDevState_Ok;
	case D3DERR_DEVICELOST:
		return EDeviceState::EDevState_Lost;
	case D3DERR_DEVICENOTRESET:
		return EDeviceState::EDevState_Notreset;
	default:
		return EDeviceState::EDevState_InternalError;
	}
}

bool CRenderBackendDX9::HandleDeviceLost() {
	return Reset(m_dpparams.BackBufferWidth, m_dpparams.BackBufferHeight);
}

void CRenderBackendDX9::ClearTarget(dword nFlag, dword color, float fDepth, byte stencil) {
	m_pD3DDevice9->Clear(0, nullptr, nFlag, color, fDepth, stencil);
}

void CRenderBackendDX9::RestoreTarget() {
	m_pD3DDevice9->SetRenderTarget(0, m_pBackbuffer);
	m_pD3DDevice9->SetDepthStencilSurface(m_pDepthStencil);

	D3DVIEWPORT9 viewport;
	ZeroMemory(&viewport, sizeof(D3DVIEWPORT9));
	viewport.Width = m_dpparams.BackBufferWidth;
	viewport.Height = m_dpparams.BackBufferHeight;
	viewport.MaxZ = 1.0f;
	m_pD3DDevice9->SetViewport(&viewport);
}

void CRenderBackendDX9::Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nPrimitiveCount) {
	m_pD3DDevice9->DrawPrimitive((D3DPRIMITIVETYPE)ePrimitiveType, nVertexOffset, nPrimitiveCount);
}

void CRenderBackendDX9::Draw(EPrimitiveType ePrimitiveType,
	dword nVertexOffset, dword nVertexCount, dword nIndexOffset, dword nPrimitiveCount) {
	m_pD3DDevice9->DrawIndexedPrimitive((D3DPRIMITIVETYPE)ePrimitiveType, 0, nVertexOffset, nVertexCount,
		nIndexOffset, nPrimitiveCount);
}

void CRenderBackendDX9::OnDisplayResized(dword w, dword h) {
	Reset(w, h);
}

//void CRendererDX9::UpdateCVars(const Config &cfg) {
//	m_cvars.m_nMSAACount = cfg.m_nMSAACount;
//	m_cvars.m_nMSAAQuality = cfg.m_nMSAAQuality;
//	m_cvars.m_bGammaOutput = cfg.m_bGammaOutput;
//}

bool CRenderBackendDX9::Reset(dword w, dword h) {
	float w_changed_ratio = (float)w / (float)m_dpparams.BackBufferWidth;
	float h_changed_ratio = (float)h / (float)m_dpparams.BackBufferHeight;

	DefaultPoolRcRelease();
	SAFE_RELEASE(m_pBackbuffer);
	SAFE_RELEASE(m_pDepthStencil);
	m_dpparams.BackBufferWidth = w;
	m_dpparams.BackBufferHeight = h;
	HRESULT hr = m_pD3DDevice9->Reset(&m_dpparams);
	if (FAILED(hr)) {
		Global::m_pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, "Failed to reset D3D9 device");
		return false;
	}
	m_pD3DDevice9->GetRenderTarget(0, &m_pBackbuffer);
	m_pD3DDevice9->GetDepthStencilSurface(&m_pDepthStencil);
	DefaultPoolRcReconstruct(w_changed_ratio, h_changed_ratio);
	return true;
}

void CRenderBackendDX9::DefaultPoolRcRelease() {
	Linklist<IGpuResource>::_NodeType *pTemp = m_lstGpuResource.m_pRoot;
	while (pTemp) {
		pTemp->m_pOwner->OnDeviceLost();
		pTemp = pTemp->m_pNext;
	}
}

void CRenderBackendDX9::DefaultPoolRcReconstruct(float w_changed_ratio, float h_changed_ratio) {
	Linklist<IGpuResource>::_NodeType *pTemp = m_lstGpuResource.m_pRoot;
	while (pTemp) {
		pTemp->m_pOwner->OnDeviceReset(w_changed_ratio, h_changed_ratio);
		pTemp = pTemp->m_pNext;
	}
}

bool CRenderBackendDX9::IsDepthFormatOk(D3DFORMAT TargetFormat, D3DFORMAT DepthFormat) {
	HRESULT hr = m_pD3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE,
		DepthFormat);
	if (FAILED(hr)) return false;

	hr = m_pD3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		D3DUSAGE_RENDERTARGET,
		D3DRTYPE_SURFACE,
		TargetFormat);
	if (FAILED(hr)) return false;

	hr = m_pD3D9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		TargetFormat,
		DepthFormat);

	return SUCCEEDED(hr);
}

bool CRenderBackendDX9::IsMultiSampleTypeOk(D3DMULTISAMPLE_TYPE Type, 
	D3DFORMAT TargetFormat, BOOL bFullScreen, DWORD &dwMaxQuality) {
	HRESULT hr = m_pD3D9->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, TargetFormat, !bFullScreen, Type, &dwMaxQuality);
	return SUCCEEDED(hr);
}