#pragma once
#include "CameraController.h"

class COrbitCameraController final : public ICameraController
{
public:
    COrbitCameraController(CCamera *pCamera);

    virtual void OnMouseMove(dword x, dword y) override;
	virtual void OnKeyDown(EKeyCode KeyCode) override;
	virtual void OnKeyUp(EKeyCode KeyCode) override;

    virtual void Update() override;

    void SetLookat(const Vec3 &lookat);

private:
    float m_Distance;
    dword m_LastX, m_LastY;
};