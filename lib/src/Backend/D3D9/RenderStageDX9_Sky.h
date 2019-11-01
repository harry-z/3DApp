#pragma once
#include "RenderStage.h"

class CRenderStageDX9_Sky final : public IRenderStage
{
public:
    CRenderStageDX9_Sky(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};