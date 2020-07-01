#include "RenderBackendDX11.h"
#include "..\..\Windows\IDisplay_Windows.h"

ID3D11Device *g_pDevice11;
ID3D11DeviceContext *g_pDeviceContext11;

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

    D3D11_TEXTURE2D_DESC Desc;
    Desc.Width = nBackBufferWidth;
    Desc.Height = nBackBufferHeight;
    Desc.MipLevels = 1;
    Desc.ArraySize = 1;
    Desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    Desc.MiscFlags = 0;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Desc.CPUAccessFlags = 0;
    Desc.SampleDesc.Count = 1;
    Desc.SampleDesc.Quality = 0;
    hr = m_pD3DDevice11->CreateTexture2D(&Desc, nullptr, &m_pMainDSTexture);
    if (FAILED(hr))
        return false;
    D3D11_DEPTH_STENCIL_VIEW_DESC DSViewDesc;
    DSViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DSViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DSViewDesc.Flags = 0;
    DSViewDesc.Texture2D.MipSlice = 0;
    hr = m_pD3DDevice11->CreateDepthStencilView(m_pMainDSTexture, &DSViewDesc, &m_pMainDS);
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
    SAFE_RELEASE(m_pMainDSTexture);
    SAFE_RELEASE(m_pMainDS);
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
    m_pD3DContext11->OMSetRenderTargets(1, &m_pMainRT, m_pMainDS);
}

void CRenderBackendDX11::Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nVertexCount, dword nPrimitiveCount)
{
    m_pD3DContext11->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)ePrimitiveType);
    m_pD3DContext11->Draw(nVertexCount, nVertexOffset);
}

void CRenderBackendDX11::Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nVertexCount, dword nIndexOffset, dword nIndexCount, dword nPrimitiveCount)
{
    m_pD3DContext11->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)ePrimitiveType);
    m_pD3DContext11->DrawIndexed(nIndexCount, nIndexOffset, nVertexOffset);
}