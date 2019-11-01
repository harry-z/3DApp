#pragma once
#include "RenderStage.h"

class CRenderStageDX9_Translucent final : public IRenderStage
{
public:
    CRenderStageDX9_Translucent(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};