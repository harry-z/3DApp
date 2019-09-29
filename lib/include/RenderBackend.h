#pragma once

#include "Prereq.h"
#include "RendererTypes.h"

enum class ERenderAPI {
	ERenderAPI_DX9,
	ERenderAPI_DX11
};

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
};
