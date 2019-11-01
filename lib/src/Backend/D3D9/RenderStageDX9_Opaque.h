#pragma once
#include "RenderStage.h"

class CRenderStageDX9_Opaque final : public IRenderStage
{
public:
    CRenderStageDX9_Opaque(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};