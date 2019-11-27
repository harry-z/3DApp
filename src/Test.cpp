#include "3DApp.h"

class CTestScene final : public ISample
{
public:
	virtual void InitSample() override;
	virtual void UninitSample() override {};
};

void CTestScene::InitSample()
{
	C3DEngine * __restrict p3DEngine = Global::m_p3DEngine;
	CHardwareBufferManager * __restrict pHwBufferMgr = Global::m_pHwBufferManager;
	CMaterialManager * __restrict pMtlMgr = Global::m_pMaterialManager;

	p3DEngine->SetSceneClipping(SCENE_CLIPPING_DEFAULT);
	p3DEngine->SetCameraController(FIRST_PERSON_CAMERA_CONTROLLER);

	CCamera *pMainCamera = p3DEngine->GetMainCamera();
    pMainCamera->SetEye(Vec3(0.0f, 0.0f, -100.0f));
    pMainCamera->SetLookat(Vec3::s_ZeroVector);

	CMaterial *pMtl = pMtlMgr->LoadMaterial("Mtl/NoIllum.mtl");

	P3T2 vb[4];
	vb[0].m_Position.Set(-50.0f, -50.0f, 0.0f);
	vb[0].m_Texcoord.Set(0.0f, 1.0f);
	vb[1].m_Position.Set(-50.0f, 50.0f, 0.0f);
	vb[1].m_Texcoord.Set(0.0f, 0.0f);
	vb[2].m_Position.Set(50.0f, -50.0f, 0.0f);
	vb[2].m_Texcoord.Set(1.0f, 1.0f);
	vb[3].m_Position.Set(50.0f, 50.0f, 0.0f);
	vb[3].m_Texcoord.Set(1.0f, 0.0f);

	IHardwareBuffer *pVertexBuffer = pHwBufferMgr->CreateVertexBuffer(false, sizeof(P3T2), sizeof(P3T2) * 4, (const byte *)vb);

	CCustomGeometryNode *pGeomNode = p3DEngine->CreateRenderNode<CCustomGeometryNode>();
	pGeomNode->AddVertexBuffer(pVertexBuffer);
	pGeomNode->SetPrimitiveType(EPrimitiveType::EPrimType_TriangleStrip);
	pGeomNode->SetPredefinedVertexLayout(EPredefinedVertexLayout::EPredefinedLayout_PT);
	pGeomNode->SetBoundingBox(AxisAlignedBox(Vec3(-50.0f, -50.0f, 0.0f), Vec3(50.0f, 50.0f, 0.0f)));
	if (pMtl != nullptr)
		pGeomNode->SetMaterial(pMtl);
	p3DEngine->RegisterNode(pGeomNode);

	CCustomGeometryNode *pGeomNode1 = p3DEngine->CreateRenderNode<CCustomGeometryNode>();
	pGeomNode1->AddVertexBuffer(pVertexBuffer);
	pGeomNode1->SetPrimitiveType(EPrimitiveType::EPrimType_TriangleStrip);
	pGeomNode1->SetPredefinedVertexLayout(EPredefinedVertexLayout::EPredefinedLayout_PT);
	pGeomNode1->SetBoundingBox(AxisAlignedBox(Vec3(-50.0f, -50.0f, 0.0f), Vec3(50.0f, 50.0f, 0.0f)));
	if (pMtl != nullptr)
		pGeomNode1->SetMaterial(pMtl);
	Matrix4 mt;
	mt.MakeIdentity();
	mt.SetTranslation(Vec3(0.0f, 0.0f, -10.0f));
	pGeomNode1->SetTransform(mt);
	p3DEngine->RegisterNode(pGeomNode1);
}


class CTestSceneRegister
{
public:
	CTestSceneRegister()
	{
		m_pTestScene = NEW_TYPE(CTestScene);
		g_SampleMap.Insert("Test Scene", m_pTestScene);
	}
	~CTestSceneRegister()
	{
		DELETE_TYPE(m_pTestScene, CTestScene);
	}

	CTestScene *m_pTestScene;
} g_TestSceneRegister;