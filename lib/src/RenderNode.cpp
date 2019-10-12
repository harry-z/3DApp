#include "RenderNode.h"

void IRenderNode::UpdateWSBoundingBox()
{
    if (BIT_CHECK(m_nInternalFlag, RN_FLAG_INTERNAL_BBOX_DIRTY))
    {
        assert(m_pBoundingBox != nullptr);
        m_pBoundingBox->Transform(*m_pTransform);
        RemoveInternalFlag(RN_FLAG_INTERNAL_TRANSFORM_DIRTY);
    }
}