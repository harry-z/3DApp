#pragma once
#include "RenderStage.h"

class CShader;
class IRenderBackend;



void RenderStageDX9_SetVertexShaderAndConstants(RenderItem *pRI, IRenderBackend *pRenderBackend, CShader *pVertexShader);
void RenderStageDX9_SetPixelShaderAndConstants(RenderItem *pRI, IRenderBackend *pRenderBackend, CShader *pPixelShader);
void RenderStageDX9_SetShaderResources(ShaderResources *pSR, IRenderBackend *pRenderBackend);
void RenderStageDX9_SetGeometryData(RenderItem *pRI, IRenderBackend *pRenderBackend);