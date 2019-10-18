#pragma once
#include "RenderItem.h"

#if CURRENT_RENDER_PATH == RENDER_PATH_FORWARD_SHADING
enum ERenderStage
{
    ERenderStage_ShadowDepth,
    ERenderStage_TranslucentShadowDepth,
    ERenderStage_ShadowMask,
    ERenderStage_TranslucentShadowMask,
    ERenderStage_SceneDepth,
    ERenderStage_Sky,
    //ERenderStage_Terrain,
    ERenderStage_Opaque,
    ERenderStage_Translucent,
    ERenderStage_3DCount,
    ERenderStage_PostProcess,
    ERenderStage_GUI,
    ERenderStage_Count
};
#endif

class IRenderStage
{
public:
    IRenderStage() : m_pShaderBatch(nullptr) {}
    IRenderStage(RenderItem::RenderItems *pShaderBatch) : m_pShaderBatch(pShaderBatch) {}
    virtual ~IRenderStage() {}
    virtual void Prepare(CCamera *pCamera) {}
    virtual void Render(CCamera *pCamera) = 0;

    static IRenderStage *m_ppRenderStage[ERenderStage_Count];

protected:
    RenderItem::RenderItems *m_pShaderBatch;
};