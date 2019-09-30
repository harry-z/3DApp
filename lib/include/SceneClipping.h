#pragma once
#include "Camera.h"
#include "RenderNode.h"

#define SCENE_CLIPPING_DEFAULT "Default"
#define SCENE_CLIPPING_OCTREE "Octree"

class DLL_EXPORT ISceneClippingStrategy
{
public:
	virtual ~ISceneClippingStrategy() {}
    virtual const char* Name() const { return nullptr; }
	virtual void SceneClipping(CCamera *pCamera, Linklist<IRenderNode> &lstSceneNode) = 0;

protected:
	void RenderNode(CCamera *pCamera, IRenderNode *pNode);
};

class CDefaultSceneClippingStrategy final : public ISceneClippingStrategy
{
public:
    virtual const char* Name() const { return SCENE_CLIPPING_DEFAULT; }
    virtual void SceneClipping(CCamera *pCamera, Linklist<IRenderNode> &lstSceneNode) override;
};