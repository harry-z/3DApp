#include "FirstPersonCameraController.h"
#include "Camera.h"

void CFirstPersonCameraController::OnMouseMove(dword x, dword y) 
{
    if (BIT_CHECK(m_State, EState_Rotating)) {
        int nDeltaX = (int)(m_LastX - x);
        int nDeltaY = (int)(m_LastY - y);
        const Vec3 &eye = m_pCamera->GetEye();
        const Vec3 &lookat = m_pCamera->GetLookat();
        Vec3 dir = lookat - eye;
        dir.Normalize();
        Vec3 up(0.0f, 1.0f, 0.0f);
        bool bHorizontal = abs(nDeltaX) >= abs(nDeltaY);
        if (bHorizontal) {
            Quat rot(up, nDeltaX * 0.005f);
            dir = rot * dir;
        }
        else {
            Vec3 right = up.Cross(dir);
            right.Normalize();
            Quat rot(right, nDeltaY * 0.005f);
            dir = rot * dir;
        }
        m_pCamera->SetLookat(eye + dir * 1.0f);
    }

    m_LastX = x; m_LastY = y;
}

void CFirstPersonCameraController::OnKeyDown(EKeyCode KeyCode)
{
    switch (KeyCode)
    {
    case KC_W:
        BIT_ADD(m_MovingState, EMoving_Forward);
        break;
    case KC_S:
        BIT_ADD(m_MovingState, EMoving_Backward);
        break;
    case KC_A:
        BIT_ADD(m_MovingState, EMoving_Left);
        break;
    case KC_D:
        BIT_ADD(m_MovingState, EMoving_Right);
        break;
    case KC_Q:
        BIT_ADD(m_MovingState, EMoving_Upward);
        break;
    case KC_E:
        BIT_ADD(m_MovingState, EMoving_Downward);
        break;
    }
}

void CFirstPersonCameraController::OnKeyUp(EKeyCode KeyCode)
{
    switch (KeyCode)
    {
    case KC_W:
        BIT_REMOVE(m_MovingState, EMoving_Forward);
        break;
    case KC_S:
        BIT_REMOVE(m_MovingState, EMoving_Backward);
        break;
    case KC_A:
        BIT_REMOVE(m_MovingState, EMoving_Left);
        break;
    case KC_D:
        BIT_REMOVE(m_MovingState, EMoving_Right);
        break;
    case KC_Q:
        BIT_REMOVE(m_MovingState, EMoving_Upward);
        break;
    case KC_E:
        BIT_REMOVE(m_MovingState, EMoving_Downward);
        break;
    }
}

void CFirstPersonCameraController::Update() 
{
    if (m_MovingState > 0) 
    {
        Vec3 eye = m_pCamera->GetEye();
        Vec3 lookat = m_pCamera->GetLookat();
        Vec3 dir = lookat - eye;
        dir.Normalize();
        Vec3 right = Vec3(0.0f, 1.0f, 0.0f).Cross(dir);
        right.Normalize();
        if (BIT_CHECK(m_MovingState, EMoving_Forward))
            eye += dir * 0.5f;
        if (BIT_CHECK(m_MovingState, EMoving_Backward))
            eye -= dir * 0.5f;
        if (BIT_CHECK(m_MovingState, EMoving_Left))
            eye -= right * 0.5f;
        if (BIT_CHECK(m_MovingState, EMoving_Right))
            eye += right * 0.5f;
        if (BIT_CHECK(m_MovingState, EMoving_Upward))
            eye.y += 0.5f;
        if (BIT_CHECK(m_MovingState, EMoving_Downward))
            eye.y -= 0.5f;
        m_pCamera->SetEye(eye);
        m_pCamera->SetLookat(eye + dir * 1.0f);
    }
}