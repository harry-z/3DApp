#pragma once
#include "CameraController.h"

#define FIRST_PERSON_CAMERA_CONTROLLER "FirstPerson"

class CFirstPersonCameraController final : public ICameraController
{
public:
    CFirstPersonCameraController(CCamera *pCamera) 
    : ICameraController(pCamera) 
    , m_State(0)
    , m_MovingState(0)
    , m_LastX(0)
    , m_LastY(0)
    {
        
    }

	virtual void OnMouseMove(dword x, dword y) override;
	virtual void OnKeyDown(EKeyCode KeyCode) override;
	virtual void OnKeyUp(EKeyCode KeyCode) override;

    virtual const char* Name() const override { return FIRST_PERSON_CAMERA_CONTROLLER; }
    virtual void Update() override;

private:
    enum EState : byte
    {
        EState_Null = 0,
        EState_Moving,
        EState_Rotating
    }; 
    byte m_State;

    enum EMovingState : byte
    {
        EMoving_Null = 0x00,
        EMoving_Forward = 0x01,
        EMoving_Backward = 0x02,
        EMoving_Left = 0x04,
        EMoving_Right = 0x08,
        EMoving_Upward = 0x10,
        EMoving_Downward = 0x20
    };
    byte m_MovingState;

    dword m_LastX, m_LastY;
};