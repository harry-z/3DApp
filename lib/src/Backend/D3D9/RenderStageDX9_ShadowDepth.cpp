#include "RenderStageDX9_ShadowDepth.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

CRenderStageDX9_ShadowDepth::CRenderStageDX9_ShadowDepth(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_ShadowDepth::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_ShadowDepth::Render(CCamera *pCamera)
{}

CRenderStageDX9_TranslucentShadowDepth::CRenderStageDX9_TranslucentShadowDepth(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_TranslucentShadowDepth::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_TranslucentShadowDepth::Render(CCamera *pCamera)
{}