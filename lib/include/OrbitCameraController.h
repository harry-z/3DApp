#pragma once
#include "CameraController.h"

#define ORBIT_CAMERA_CONTROLLER "Orbit"

class COrbitCameraController final : public ICameraController
{
public:
    COrbitCameraController(CCamera *pCamera);

    virtual void OnMouseMove(dword x, dword y) override;
	virtual void OnKeyDown(EKeyCode KeyCode) override;
	virtual void OnKeyUp(EKeyCode KeyCode) override;

    virtual const char* Name() const override { return ORBIT_CAMERA_CONTROLLER; }
    virtual void Update() override;

    void SetLookat(const Vec3 &lookat);

private:
    float m_Distance;
    dword m_LastX, m_LastY;
};