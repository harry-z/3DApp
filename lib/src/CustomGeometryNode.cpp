#include "CustomGeometryNode.h"
#include "RenderItem.h"

CCustomGeometryNode::~CCustomGeometryNode()
{

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

		if (CheckInternalFlag(RN_FLAG_INTERNAL_TRANSFORM_DIRTY))
		{
			m_pRenderObj->m_pWorldTransform = m_pTransform;
			RemoveInternalFlag(RN_FLAG_INTERNAL_TRANSFORM_DIRTY);
		}

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

		CMaterial *pUsedMtl = nullptr;
		if (m_MtlPtr.IsValid() && m_MtlPtr->IsCreatedOrLoaded())
			pUsedMtl = m_MtlPtr.Get();
		else
			pUsedMtl = Global::m_pMaterialManager->GetDefaultMaterial().Get();

		if (!pUsedMtl->IsCompiled())
			pUsedMtl->Compile();

		PreRenderInternal(pCamera, mode, m_pRenderObj, pUsedMtl);
	}
}

void CCustomGeometryNode::SetMaterial(CMaterial *pMaterial)
{
	m_MtlPtr = pMaterial;
}