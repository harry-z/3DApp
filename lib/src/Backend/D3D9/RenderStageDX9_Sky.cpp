#include "RenderStageDX9_Sky.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

CRenderStageDX9_Sky::CRenderStageDX9_Sky(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_Sky::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_Sky::Render(CCamera *pCamera)
{}