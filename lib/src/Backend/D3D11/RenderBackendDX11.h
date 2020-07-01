#pragma once
#include "..\..\RendererStableHeader.h"
#include "Display.h"
#include "RenderBackend.h"

class CRenderBackendDX11 : public IRenderBackend, public IDisplay::IObserver {
public:
    CRenderBackendDX11();
    virtual ~CRenderBackendDX11();

    // Methods from IRenderer
	virtual bool Initialize(IDisplay *pDisplay) override;
	virtual void Shutdown() override;

    virtual void BeginRendering() override {}
    virtual void EndRendering() override {}

    virtual EDeviceState CheckDeviceState() override { return EDeviceState::EDevState_Ok; }
    virtual bool HandleDeviceLost() override { return true; }

    virtual ERenderAPI GetRenderAPI() const override { return ERenderAPI::ERenderAPI_DX11; }

    virtual void ClearTarget(dword nFlag, dword color, float fDepth, byte stencil) override;
	virtual void RestoreTarget() override;
	virtual void SetTarget(dword nNumColorBuffers, CTexture **ppColorBuffer, CTexture *pDepthStencilBuffer) override;

    virtual void SetShader(CShader *pShader) override;

    virtual void SetTexture(dword nSlot, CTexture *pTexture) override;

    virtual void SetVertexLayout(IVertexLayout *pLayout) override;
    virtual void SetVertexBuffers(const CArray<IHardwareBuffer*> &arrVertexBuffer) override;
    virtual void SetIndexBuffer(IHardwareBuffer *pIndexBuffer) override;

    virtual void Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nVertexCount, dword nPrimitiveCount) override;
    virtual void Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nVertexCount, dword nIndexOffset, dword nIndexCount, dword nPrimitiveCount) override;

    virtual Matrix4 ConvertProjectionMatrixByAPI(const Matrix4 &proj) const override { return proj; }

    // Method from IDisplay::IObserver
	virtual void OnDisplayResized(dword w, dword h) override {}

private:
    IDXGISwapChain *m_pSwapChain = nullptr;
    ID3D11Device *m_pD3DDevice11 = nullptr;
    ID3D11DeviceContext *m_pD3DContext11 = nullptr;
    ID3D11RenderTargetView *m_pMainRT = nullptr;
    ID3D11Texture2D *m_pMainDSTexture = nullptr;
    ID3D11DepthStencilView *m_pMainDS = nullptr;

    struct ContextState {
        ID3D11RenderTargetView *m_pCurrentRTView = nullptr;
        ID3D11DepthStencilView *m_pCurrentDSView = nullptr;

        inline void SetCurrentRTView(ID3D11RenderTargetView *pRTView) { m_pCurrentRTView = pRTView; }
        inline ID3D11RenderTargetView* GetCurrentRTView() { return m_pCurrentRTView; }
        inline void SetCurrentDSView(ID3D11DepthStencilView *pDSView) { m_pCurrentDSView = pDSView; }
        inline ID3D11DepthStencilView* GetCurrentDSView() { return m_pCurrentDSView; }
        inline void Reset() {
            m_pCurrentRTView = nullptr;
            m_pCurrentDSView = nullptr;
        }

    } m_ContextState;
};