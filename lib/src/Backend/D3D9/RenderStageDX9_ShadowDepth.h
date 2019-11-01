#pragma once
#include "RenderStage.h"

class CRenderStageDX9_ShadowDepth final : public IRenderStage
{
public:
    CRenderStageDX9_ShadowDepth(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};

class CRenderStageDX9_TranslucentShadowDepth final : public IRenderStage
{
public:
    CRenderStageDX9_TranslucentShadowDepth(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};