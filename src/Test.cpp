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

	p3DEngine->SetSceneClipping(SCENE_CLIPPING_DEFAULT);

	CCamera *pMainCamera = p3DEngine->GetMainCamera();
    pMainCamera->SetEye(Vec3(0.0f, 0.0f, -100.0f));
    pMainCamera->SetLookat(Vec3::s_ZeroVector);

    CCustomGeometryNode *pGeomNode = p3DEngine->CreateRenderNode<CCustomGeometryNode>();
	
	P3 vb[4];
	vb[0].m_Position.Set(-50.0f, -50.0f, 0.0f);
	vb[1].m_Position.Set(-50.0f, 50.0f, 0.0f);
	vb[2].m_Position.Set(50.0f, -50.0f, 0.0f);
	vb[3].m_Position.Set(50.0f, 50.0f, 0.0f);
	IHardwareBuffer *pVertexBuffer = pHwBufferMgr->CreateVertexBuffer(false, sizeof(P3), sizeof(P3) * 4, (const byte *)vb);
	pGeomNode->AddVertexBuffer(pVertexBuffer);
	pGeomNode->SetPrimitiveType(EPrimitiveType::EPrimType_TriangleStrip);
	pGeomNode->SetPredefinedVertexLayout(EPredefinedVertexLayout::EPredefinedLayout_P);

	p3DEngine->RegisterNode(pGeomNode);
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