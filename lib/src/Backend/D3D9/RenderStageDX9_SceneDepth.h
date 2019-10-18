#pragma once
#include "RenderStage.h"

class CCamera;
class CRenderStageDX9_SceneDepth final : public IRenderStage
{
public:
    CRenderStageDX9_SceneDepth(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};