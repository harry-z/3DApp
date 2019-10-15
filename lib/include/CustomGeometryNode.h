#pragma once
#include "HardwareBuffer.h"
#include "RenderNode.h"

struct RenderObject;
class CCustomGeometryNode final : public IRenderNode
{
public:
	friend class C3DEngine;

	DECLARE_NODE_TYPE(ERNType::ERNType_CustomGeometry)
	
	virtual void PreRender(CCamera *pCamera, EPreRenderMode mode) override;

	void SetPredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout);
	void SetVertexLayout(IVertexLayout *pVertexLayout);
	void SetPrimitiveType(EPrimitiveType PrimType);
	void AddVertexBuffer(IHardwareBuffer *pVertexBuffer);
	void SetIndexBuffer(IHardwareBuffer *pIndexBuffer);

	void SetMaterial(CMaterial *pMaterial);

protected:
	virtual ~CCustomGeometryNode();

private:
	RenderObject *m_pRenderObj = nullptr;
	IVertexLayout *m_pVertexLayout = nullptr;
	IHardwareBuffer *m_pIndexBuffer = nullptr;
	CArray<IHardwareBuffer*> m_arrVertexBuffer;
	EPrimitiveType m_PrimitiveType = EPrimitiveType::EPrimType_PointList;
};