#pragma once

#include "Prereq.h"

#include "Display.h"
#include "RenderBackend.h"
// #include "RenderNode.h"

//struct TerrainInfo;
class ICameraController;
class CCamera;
class CInputListener;


class DLL_EXPORT C3DEngine {
public:
	C3DEngine();
	~C3DEngine();

	bool Initialize(CInputListener *pExternalInputListener = nullptr, 
		ICameraController *pExternalCameraController = nullptr);

	// IRenderNode* CreateRenderNode(ERNType eNodeType);
	// void DestroyRenderNode(IRenderNode *pNode);

	// void RegisterNode(IRenderNode *pNode);
	// void UnregisterNode(IRenderNode *pNode);

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
	

	FORCE_INLINE CCamera* GetMainCamera() { return m_pMainCamera; }
	ERenderAPI GetRenderAPIType() const;

private:
	void Frame(dword nFrameId);
	// void OnCreateRenderNode(IRenderNode *pNode);

	// enum ENodeListType {
	// 	ENodeListType_Mesh,
	// 	ENodeListType_Other,
	// 	ENodeListType_Count
	// };
	// ENodeListType GetListTypeByNodeType(ERNType type);

private:

	CCamera *m_pMainCamera;
	CCamera *m_pSunShadowCamera;
	ICameraController *m_pCameraController;

	// class CMainCameraDisplayObserver : public IDisplay::IObserver {
	// public:
	// 	CCamera *m_pCamera;
	// 	virtual void OnDisplayResized(dword w, dword h);
	// } m_DisplayObserver;

	// Linklist<IRenderNode> m_SceneNodelist[ENodeListType_Count];

	// CPool m_MatrixPool;
	// CPool m_AABBPool;
	//CPool m_NodePool;
	//CPool m_MeshDataPool;
	//CPool m_GeomDataPool;

	bool m_bUseExternalInputListener;
	bool m_bUseExternalCameraController;
};