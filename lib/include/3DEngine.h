#pragma once

#include "Prereq.h"

#include "Display.h"
#include "RenderBackend.h"
#include "SceneClipping.h"
#include "CameraController.h"

//struct TerrainInfo;
class CInputListener;



class DLL_EXPORT C3DEngine 
{
public:
	enum ENodeListType {
		ENodeListType_Unknown = -1,
		ENodeListType_Geometry,
		ENodeListType_Count
	};

	C3DEngine();
	~C3DEngine();

	bool Initialize();
	void ClearScene();

	void SetSceneClipping(const char *pszName);
	void SetExternalSceneClipping(ISceneClippingStrategy *pSceneClipping);

	void SetCameraController(const char *pszName);
	void SetExternalCameraController(ICameraController *pCameraController);

	template <class NodeType>
	NodeType* CreateRenderNode()
	{
		IRenderNode *pNode = CreateRenderNodeInternal(NodeType::S_GetType());
		if (pNode != nullptr)
			return static_cast<NodeType*>(pNode);
		else
			return nullptr;
	}
	void DestroyRenderNode(IRenderNode *pNode);

	void RegisterNode(IRenderNode *pNode);
	void UnregisterNode(IRenderNode *pNode);

	//void CreateSky();
	//void DestroySky();
	// void SetSkyTurbidity(float turbidity);
	// void SetSkyRayleighScattering(float rayleigh);

	//bool CreateTerrain(const TerrainInfo &info);
	//void DestroyTerrain(bool bOnlyCache = false);

	

	//void SetSunlightDirection(const Vec3 &dir);
	//void SetSunlightColor(const Vec3 &lightColor);
	//void SetGlobalAmbientColor(const Vec3 &ambientColor);

	// void SetTime(float fTime);
	// float GetTime() const;
	// void SetTimeElapseRatio(float fRatio);
	// float GetTimeElapseRatio() const;

	void Run();
	void RunOneFrame(dword nFrameId);
	

	inline CCamera* GetMainCamera() { return m_pMainCamera; }
	ERenderAPI GetRenderAPIType() const;

private:
	void Frame(dword nFrameId);

	ISceneClippingStrategy* CreateSceneClippingStrategy(const char *pszName);
	void DestroySceneClippingStrategy(ISceneClippingStrategy *pClippingStrategy);

	ICameraController* CreateCameraController(const char *pszName);
	void DestroyCameraController(ICameraController *pCameraController);

	IRenderNode* CreateRenderNodeInternal(ERNType NodeType);
	void DestroyRenderNodeInternal(IRenderNode *pNode);
	void OnCreateRenderNode(IRenderNode *pNode);
	void OnDestroyRenderNode(IRenderNode *pNode);

	
	ENodeListType GetListTypeByNodeType(ERNType type) const;

private:

	CCamera *m_pMainCamera;
	CCamera *m_pSunShadowCamera;
	ICameraController *m_pCameraController;
	ISceneClippingStrategy *m_pSceneClipping;

	// class CMainCameraDisplayObserver : public IDisplay::IObserver {
	// public:
	// 	CCamera *m_pCamera;
	// 	virtual void OnDisplayResized(dword w, dword h);
	// } m_DisplayObserver;

	Linklist<IRenderNode> m_SceneNodelist[ENodeListType_Count];

	CPool m_MatrixPool;
	CPool m_AABBPool;
	//CPool m_NodePool;
	//CPool m_MeshDataPool;
	//CPool m_GeomDataPool;
};