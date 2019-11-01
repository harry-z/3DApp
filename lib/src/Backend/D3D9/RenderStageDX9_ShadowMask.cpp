#include "RenderStageDX9_ShadowMask.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

CRenderStageDX9_ShadowMask::CRenderStageDX9_ShadowMask(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_ShadowMask::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_ShadowMask::Render(CCamera *pCamera)
{}

CRenderStageDX9_TranslucentShadowMask::CRenderStageDX9_TranslucentShadowMask(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_TranslucentShadowMask::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_TranslucentShadowMask::Render(CCamera *pCamera)
{}