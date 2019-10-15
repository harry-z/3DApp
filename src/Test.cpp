#include "3DApp.h"

class CTestScene final : public ISample
{
public:
	virtual void InitSample() override;
	virtual void UninitSample() override {};
};

void CTestScene::InitSample()
{
	CCamera *pMainCamera = Global::m_p3DEngine->GetMainCamera();
    pMainCamera->SetEye(Vec3(0.0f, 0.0f, -100.0f));
    pMainCamera->SetLookat(Vec3::s_ZeroVector);

    CCustomGeometryNode *pGeomNode = Global::m_p3DEngine->CreateRenderNode<CCustomGeometryNode>();
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