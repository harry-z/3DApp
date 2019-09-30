#pragma once
#include "Material.h"

#define RN_FLAG_VISIBLE							0x0001
#define RN_FLAG_SHADOWCASTER					0x0002
#define RN_FLAG_SHADOWRECEIVER					0x0004
#define RN_FLAG_ALWAYSVISIBLE					0x0008

#define RN_FLAG_INTERNAL_BBOX_DIRTY             0x0001

enum class ERNType : byte
{
    ERNType_Unknown = 0,
    ERNType_Mesh,
    ERNType_CustomGeometry
};

class CCamera;
class IRenderNode
{
public:
    friend class C3DEngine;

    virtual ERNType GetType() const { return ERNType::ERNType_Unknown; }

    inline void SetTransform(const Matrix4 &transform) {
		*m_pTransform = transform;
		NeedUpdateBoundingBox();
	}
	inline const Matrix4& GetTransform() const { return *m_pTransform; }
	inline const AxisAlignedBox& GetWSBox() const { return *m_pBoundingBox; }

    inline void SetRenderFlag(word nFlag) { m_nFlag = nFlag; }
	inline void AddRenderFlag(word nFlag) { BIT_ADD(m_nFlag, nFlag); }
	inline void RemoveRenderFlag(word nFlag) { BIT_REMOVE(m_nFlag, nFlag); }
	inline bool CheckRenderFlag(word nFlag) { return BIT_CHECK(m_nFlag, nFlag); }

    

	inline CMaterial* GetMaterial() { return m_MtlPtr; }

    enum EPreRenderMode
    {
        EPreRenderMode_ShadowCaster = 0,
		EPreRenderMode_ShadowReceiver,
		EPreRenderMode_General
    };
    virtual void PreRender(CCamera *pCamera, EPreRenderMode mode) = 0;

protected:
    IRenderNode()
    : m_nFlag(RN_FLAG_VISIBLE)
    , m_nInternalFlag(0)
    {
        m_node.m_pOwner = this;
    }
    virtual ~IRenderNode() {}

    inline void NeedUpdateBoundingBox() { BIT_ADD(m_nInternalFlag, RN_FLAG_INTERNAL_BBOX_DIRTY); }
    inline void BoundingBoxUpdated() { BIT_REMOVE(m_nInternalFlag, RN_FLAG_INTERNAL_BBOX_DIRTY); }
    void UpdateWSBoundingBox();

protected:
    word m_nFlag;
    word m_nInternalFlag;
    Matrix4 *m_pTransform = nullptr;
	AxisAlignedBox *m_pBoundingBox = nullptr;
	CReferencedPointer<CMaterial> m_MtlPtr;

    LinklistNode<IRenderNode> m_node;
};