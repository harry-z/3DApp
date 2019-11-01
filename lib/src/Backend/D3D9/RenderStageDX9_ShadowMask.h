#pragma once
#include "RenderStage.h"

class CRenderStageDX9_ShadowMask final : public IRenderStage
{
public:
    CRenderStageDX9_ShadowMask(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};

class CRenderStageDX9_TranslucentShadowMask final : public IRenderStage
{
public:
    CRenderStageDX9_TranslucentShadowMask(RenderItem::RenderItems *pShaderBatch);
    virtual void Prepare(CCamera *pCamera) override;
    virtual void Render(CCamera *pCamera) override;
};