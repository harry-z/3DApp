#include "RenderStageDX9_Translucent.h"
#include "RenderStageDX9.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "../../RendererStableHeader.h"

CRenderStageDX9_Translucent::CRenderStageDX9_Translucent(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_Translucent::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_Translucent::Render(CCamera *pCamera)
{}