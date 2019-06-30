#pragma once

#include "..\..\RendererStableHeader.h"
//#include "RendererDX9CVars.h"
//#include "Array.h"
#include "Display.h"
//#include "Pool.h"
//#include "Cfg.h"
#include "RenderBackend.h"

//struct GpuResourceWrapper;

struct IGpuResource;
class IHardwareBuffer;
class CShader;
class CTexture;
class CVertexLayoutDX9;

class CRenderBackendDX9 : public IRenderBackend, public IDisplay::IObserver {
public:
	CRenderBackendDX9();
	virtual ~CRenderBackendDX9();

	// Methods from IRenderer
	virtual bool Initialize(IDisplay *pDisplay) override;
	virtual void Shutdown() override;

	//virtual IVertexLayout* CreateVertexLayout(const VertexElement *pElements, dword nElementCount) override;
	//virtual void DestroyVertexLayout(IVertexLayout *pLayout) override;
	// virtual IVertexLayout* GetPredefinedVertexLayout(EPredefinedVertexLayout eLayout) override;

	//virtual BlendStateBlock* CreateBlendStateBlock() override;
	//virtual DepthStencilStateBlock* CreateDepthStencilStateBlock() override;
	//virtual RasterStateBlock* CreateRasterStateBlock() override;
	//virtual SamplerStateBlock* CreateSamplerStateBlock() override;

	//virtual void DestroyBlendStateBlock(BlendStateBlock *pBS) override;
	//virtual void DestroyDepthStencilStateBlock(DepthStencilStateBlock *pDSS) override;
	//virtual void DestroyRasterStateBlock(RasterStateBlock *pRS) override;
	//virtual void DestroySamplerStateBlock(SamplerStateBlock *pSS) override;

	//virtual BlendStateBlock* GetPredefinedBlendState(EPredefinedBlendState eBlendState) override;
	//virtual DepthStencilStateBlock* GetPredefinedDepthStencilState(EPredefinedDepthStencilState eDepthStencilState) override;
	//virtual RasterStateBlock* GetPredefinedRasterState(EPredefinedRasterState eRasterState) override;
	//virtual SamplerStateBlock* GetPredefinedSamplerState(EPredefinedSamplerState eSamplerState) override;

	virtual void BeginRendering() override;
	virtual void EndRendering() override;

	virtual EDeviceState CheckDeviceState() override;
	virtual bool HandleDeviceLost() override;

	virtual ERenderAPI GetRenderAPI() const override { return ERenderAPI::ERenderAPI_DX9; }

	// Method from IDisplay::IObserver
	virtual void OnDisplayResized(dword w, dword h) override;

	void ClearTarget(dword nFlag, dword color, float fDepth, byte stencil);
	void RestoreTarget();
	// void SetTarget(CTexture *pRenderTarget, CTexture *pDepthStencil);

	//virtual void SetBlendStateBlock(BlendStateBlock *pBlendBlock) override;
	//virtual void SetDepthStencilStateBlock(DepthStencilStateBlock *pDepthStencilBlock) override;
	//virtual void SetRasterStateBlock(RasterStateBlock *pRasterBlock) override;

	//virtual void SetSamplerStateBlock(dword nSlot, SamplerStateBlock *pSamplerBlock) override;
	// void SetTexture1D(dword nSlot, CTexture *pTex1D);
	// void SetTexture2D(dword nSlot, CTexture *pTex2D);
	// void SetTexture3D(dword nSlot, CTexture *pTex3D);
	// void SetTextureCube(dword nSlot, CTexture *pTexCube);

	// void SetShader(CShader *pShader);
	// void SetVertexLayout(IVertexLayout *pVertexLayout);
	// void SetVBIB(IHardwareBuffer *pVB, dword nStride, IHardwareBuffer *pSkinningVB, IHardwareBuffer *pIB);

	void Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nPrimitiveCount);
	void Draw(EPrimitiveType ePrimitiveType,
		dword nVertexOffset, dword nVertexCount,
		dword nIndexOffset, dword nPrimitiveCount);

	

	
	Matrix4 ConvertProjectionMatrix(const Matrix4 &proj) { return proj; }


public:
	//void UpdateCVars(const Config &cfg);

	bool Reset(dword w, dword h);

	void DefaultPoolRcRelease();
	void DefaultPoolRcReconstruct(float w_changed_ratio, float h_changed_ratio);

	bool IsDepthFormatOk(D3DFORMAT TargetFormat, D3DFORMAT DepthFormat);
	bool IsMultiSampleTypeOk(D3DMULTISAMPLE_TYPE Type, D3DFORMAT TargetFormat, BOOL bFullScreen, DWORD &dwMaxQuality);

	// void InitializePredefinedVertexLayouts();
	//void InitializePredefinedStates();
	//void InitializePredefinedTextures();

	// void UninitializePredefinedVertexLayouts();
	//void UninitializePredefinedStates();
	//void UninitializePredefinedTextures();

public:
	D3DPRESENT_PARAMETERS m_dpparams;
	//RendererDX9CVars m_cvars;
	CRendererStateCache m_Cache;

	IDirect3D9 *m_pD3D9;
	IDirect3DDevice9 *m_pD3DDevice9;
	D3DCAPS9 *m_pDeviceCaps9;
	IDirect3DSurface9 *m_pBackbuffer;
	IDirect3DSurface9 *m_pDepthStencil;

	//static CRenderBackendDX9 *s_me;
	
//public:
//	enum EGpuObjType {
//#if TARGET_ARCHITECTURE == ARCHITECTURE_X86
//		ESize16 = 0,
//		ESize32,
//#elif TARGET_ARCHITECTURE == ARCHITECTURE_X64
//		ESize32 = 0,
//		ESize64,
//#endif
//		EGpuResource
//	};
//	CPool m_GpuObjsPool[3];

public:
	Linklist<IGpuResource> m_lstGpuResource;

	//CArray<IVertexLayout*> m_arrPredefinedLayout;

	//CArray<BlendStateBlock*> m_arrPredefinedBS;
	//CArray<DepthStencilStateBlock*> m_arrPredefinedDSS;
	//CArray<RasterStateBlock*> m_arrPredefinedRS;
	//CArray<SamplerStateBlock*> m_arrPredefinedSS;

	//CArray<ShaderUniform*> m_arrPredefinedShaderUniform;

	CVertexLayoutDX9 *m_arrPredefinedLayout;



//public:
//	ITexture2D *m_pShadowDepth;
//	ITexture2D *m_pShadowDepthDepth;
//	ITexture2D *m_pShadowMask;
};