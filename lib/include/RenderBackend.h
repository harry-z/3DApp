#pragma once

#include "Prereq.h"
#include "RendererTypes.h"

enum class ERenderAPI {
	ERenderAPI_DX9,
	ERenderAPI_DX11
};

class IDisplay;
class CTexture;
class CShader;
class IVertexLayout;
class IHardwareBuffer;

// class CRendererStateCache {
// public:

// 	void Reset() {
// 		m_pCurrentVertexLayout = nullptr;
// 		m_nCurrentShader = 0xFF;
// 		m_nCurrentShaderResource = 0;
// 		m_nCurrentMaterial = 0;
// 		m_nCurrentRenderTarget = 0;
// 	}
// 	bool NeedUpdateVertexLayout(IVertexLayout *pVertexLayout) {
// 		bool b = m_pCurrentVertexLayout != pVertexLayout;
// 		if (b) m_pCurrentVertexLayout = pVertexLayout;
// 		return b;
// 	}
// 	bool NeedUpdateShader(byte shader) {
// 		bool b = m_nCurrentShader != shader;
// 		if (b) m_nCurrentShader = shader;
// 		return b;
// 	}
// 	bool NeedUpdateShaderResource(dword shaderResource) {
// 		bool b = m_nCurrentShaderResource != shaderResource;
// 		if (b) m_nCurrentShaderResource = shaderResource;
// 		return b;
// 	}
// 	bool NeedUpdateMaterial(dword material) {
// 		bool b = m_nCurrentMaterial != material;
// 		if (b) m_nCurrentMaterial = material;
// 		return b;
// 	}
// 	bool NeedUpdateRenderTarget(dword target) {
// 		bool b = m_nCurrentRenderTarget != target;
// 		if (b) m_nCurrentRenderTarget = target;
// 		return b;
// 	}

// 	IVertexLayout *m_pCurrentVertexLayout;
// 	byte m_nCurrentShader;
// 	dword m_nCurrentShaderResource;
// 	dword m_nCurrentMaterial;
// 	dword m_nCurrentRenderTarget;
// };

class IRenderBackend {
public:
	virtual ~IRenderBackend() {}

	virtual bool Initialize(IDisplay *pDisplay) = 0;
	virtual void Shutdown() = 0;
	
	// virtual IVertexLayout* GetPredefinedVertexLayout(EPredefinedVertexLayout eLayout) = 0;

	virtual void BeginRendering() = 0;
	virtual void EndRendering() = 0;

	virtual EDeviceState CheckDeviceState() = 0;
	virtual bool HandleDeviceLost() = 0;

	virtual ERenderAPI GetRenderAPI() const = 0;

	virtual void ClearTarget(dword nFlag, dword color, float fDepth, byte stencil) = 0;
	virtual void RestoreTarget() = 0;
	virtual void SetTarget(CTexture *pColorBuffer, CTexture *pDepthStencilBuffer) = 0;

	virtual void SetShader(CShader *pShader) = 0;

	virtual void SetVertexLayout(IVertexLayout *pLayout) = 0;
	virtual void SetVertexBuffers(const CArray<IHardwareBuffer*> &arrVertexBuffer) = 0;
	virtual void SetIndexBuffer(IHardwareBuffer *pIndexBuffer) = 0;

	virtual void Draw(EPrimitiveType ePrimitiveType, dword nVertexOffset, dword nPrimitiveCount) = 0;
	virtual void Draw(EPrimitiveType ePrimitiveType,
		dword nVertexOffset, dword nVertexCount,
		dword nIndexOffset, dword nPrimitiveCount) = 0;

	virtual Matrix4 ConvertProjectionMatrixByAPI(const Matrix4 &proj) const = 0;
};
