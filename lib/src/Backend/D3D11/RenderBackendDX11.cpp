#include "RenderBackendDX11.h"
#include "..\..\Windows\IDisplay_Windows.h"

CRenderBackendDX11::CRenderBackendDX11()
{

}

CRenderBackendDX11::~CRenderBackendDX11()
{
    Shutdown();
}

bool CRenderBackendDX11::Initialize(IDisplay *pDisplay)
{
    CLog *pLog = Global::m_pLog;
	IDisplayWindows *pDisplayWindows = (IDisplayWindows *)(pDisplay);
    dword nBackBufferWidth, nBackBufferHeight;
	pDisplayWindows->GetClientDimension(nBackBufferWidth, nBackBufferHeight);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = nBackBufferWidth;
    sd.BufferDesc.Height = nBackBufferHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = pDisplayWindows->GetHWnd();
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = !pDisplayWindows->IsFullScreen();

    D3D_FEATURE_LEVEL FeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, 
        D3D_DRIVER_TYPE_HARDWARE, 
        NULL, // Software Rasterizer
        D3D11_CREATE_DEVICE_SINGLETHREADED, 
        FeatureLevels, 3, 
        D3D11_SDK_VERSION,
        &sd,
        &m_pSwapChain,
        &m_pD3DDevice11, 
        NULL, 
        &m_pD3DContext11);
    if (FAILED(hr))
        return false;

    ID3D11Texture2D *pBackBuffer;
    if(FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
        return false;
    hr = m_pD3DDevice11->CreateRenderTargetView(pBackBuffer, NULL, &m_pMainRT);
    pBackBuffer->Release();
    if (FAILED(hr))
        return false;

    pDisplayWindows->AddDisplayObserver(this);

    pLog->Log(ELogType::eLogType_Info, ELogFlag::eLogFlag_Critical, "Initialize RenderBackend D3D11");

    g_pDevice11 = m_pD3DDevice11;
    g_pDeviceContext11 = m_pD3DContext11;
    return true;
}

void CRenderBackendDX11::Shutdown()
{
    SAFE_RELEASE(m_pMainRT);
    SAFE_RELEASE(m_pD3DContext11);
    SAFE_RELEASE(m_pD3DDevice11);
    SAFE_RELEASE(m_pSwapChain);
}

void CRenderBackendDX11::ClearTarget(dword nFlag, dword color, float fDepth, byte stencil)
{
    ID3D11RenderTargetView *pCurrentRTView = m_ContextState.GetCurrentRTView();
    if (pCurrentRTView != nullptr)
    {
        float RGBA[] = {
            ((color >> 16) & 0x000000FF) / 255.0f,
            ((color >> 8) & 0x000000FF) / 255.0f,
            (color & 0x000000FF) / 255.0f,
            ((color >> 24) & 0x000000FF) / 255.0f
        };
        m_pD3DContext11->ClearRenderTargetView(pCurrentRTView, RGBA);
    }
}

void CRenderBackendDX11::RestoreTarget()
{
    m_pD3DContext11->OMSetRenderTargets(1, &m_pMainRT, nullptr);
}

void CRenderBackendDX11::Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nPrimitiveCount)
{
    m_pD3DContext11->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)ePrimitiveType);
    m_pD3DContext11->Draw()
}