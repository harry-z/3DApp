#include "CustomGeometryNode.h"
#include "RenderItem.h"

CCustomGeometryNode::~CCustomGeometryNode()
{
	if (m_pRenderObj != nullptr)
		RenderObject::DestroyRenderObject(m_pRenderObj);
}

void CCustomGeometryNode::PreRender(CCamera *pCamera, EPreRenderMode mode)
{
	if (CheckFlag(RN_FLAG_VISIBLE))
	{
		if (m_pVertexLayout == nullptr)
			return;
		if (m_arrVertexBuffer.Num() == 0)
			return;
		for (auto &VertexBuffer : m_arrVertexBuffer)
		{
			if (VertexBuffer == nullptr)
				return;
		}

		if (m_pRenderObj == nullptr)
			m_pRenderObj = RenderObject::CreateRenderObject();

		m_pRenderObj->m_pWorldTransform = m_pTransform;

		if (CheckInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY))
		{
			m_pRenderObj->m_arrHwBuffer = m_arrVertexBuffer;
			m_pRenderObj->m_pIB = m_pIndexBuffer;
			m_pRenderObj->m_pVertexLayout = m_pVertexLayout;

			m_pRenderObj->m_nVertexCount = m_arrVertexBuffer[0]->Count();
			dword nPrimCount = 0;
			switch (m_PrimitiveType) 
			{
			case EPrimitiveType::EPrimType_PointList:
				nPrimCount = m_pRenderObj->m_nVertexCount;
			case EPrimitiveType::EPrimType_LineList:
				nPrimCount = (m_pIndexBuffer ? m_pIndexBuffer->Count() : m_pRenderObj->m_nVertexCount) / 2;
			case EPrimitiveType::EPrimType_LineStrip:
				nPrimCount = m_pRenderObj->m_nVertexCount - 1;
			case EPrimitiveType::EPrimType_TriangleList:
				nPrimCount = (m_pIndexBuffer ? m_pIndexBuffer->Count() : m_pRenderObj->m_nVertexCount) / 3;
			case EPrimitiveType::EPrimType_TriangleStrip:
				nPrimCount = m_pRenderObj->m_nVertexCount - 2;
			}
			m_pRenderObj->m_nPrimitiveCount = nPrimCount;
			m_pRenderObj->m_PrimType = m_PrimitiveType;
			
			RemoveInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
		}

		if (m_MtlPtr.IsValid() && m_MtlPtr->IsCreatedOrLoaded())
		{
			if (m_MtlPtr->Compile())
			{
				PreRenderInternal(pCamera, mode, m_pRenderObj, m_MtlPtr.Get());
				return;
			}
		}

		CMaterial *pDefaultMtl = Global::m_pMaterialManager->GetDefaultMaterial().Get();
		pDefaultMtl->Compile();
		PreRenderInternal(pCamera, mode, m_pRenderObj, pDefaultMtl);
	}
}

void CCustomGeometryNode::SetPredefinedVertexLayout(EPredefinedVertexLayout PredefinedLayout)
{
	m_pVertexLayout = Global::m_pHwBufferManager->GetPredefinedVertexLayout(PredefinedLayout);
	AddInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
}

void CCustomGeometryNode::SetVertexLayout(IVertexLayout *pVertexLayout)
{
	m_pVertexLayout = pVertexLayout;
	AddInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
}

void CCustomGeometryNode::SetPrimitiveType(EPrimitiveType PrimType)
{
	m_PrimitiveType = PrimType;
	AddInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
}

void CCustomGeometryNode::AddVertexBuffer(IHardwareBuffer *pVertexBuffer)
{
	m_arrVertexBuffer.Emplace(pVertexBuffer);
	AddInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
}

void CCustomGeometryNode::SetIndexBuffer(IHardwareBuffer *pIndexBuffer)
{
	m_pIndexBuffer = pIndexBuffer;
	AddInternalFlag(RN_FLAG_INTERNAL_GEOMETRY_DIRTY);
}

void CCustomGeometryNode::SetMaterial(CMaterial *pMaterial)
{
	m_MtlPtr = pMaterial;
}