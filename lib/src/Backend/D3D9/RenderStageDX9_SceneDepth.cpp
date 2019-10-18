#include "RenderStageDX9_SceneDepth.h"
#include "Camera.h"
#include "Shader.h"

CRenderStageDX9_SceneDepth::CRenderStageDX9_SceneDepth(RenderItem::RenderItems *pShaderBatch)
: IRenderStage(pShaderBatch)
{}

void CRenderStageDX9_SceneDepth::Prepare(CCamera *pCamera)
{}

void CRenderStageDX9_SceneDepth::Render(CCamera *pCamera)
{
    RenderItem::RenderItems &items = *m_pShaderBatch;
	if (!items.Empty()) 
    {
        CShaderManager * __restrict pShaderManager = Global::m_pShaderManager;
        CShader *pVertexShader = pShaderManager->FindShaderByName("VS_SceneDepth");
        CShader *pPixelShader = pShaderManager->FindShaderByName("PS_SceneDepth");
        if (pVertexShader && pPixelShader)
        {
            
        }
    }
}