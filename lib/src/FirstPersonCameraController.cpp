#include "FirstPersonCameraController.h"
#include "Camera.h"
#include "Platform.h"
#include "Display.h"

void CFirstPersonCameraController::OnMouseMove(dword x, dword y) 
{
    // static bool bFirstTime = true;

    // dword w, h;
    // Global::m_pDisplay->GetClientDimension(w, h);
    // w /= 2; h /= 2;

    // if (bFirstTime)
    // {
    //     BIT_ADD(m_State, EState_Rotating);
    //     bFirstTime = false;
    // }
    // else
    // {
    //     if (BIT_CHECK(m_State, EState_Rotating)) {
    //         int nDeltaX = (int)(x - w);
    //         int nDeltaY = (int)(y - h);
    //         float fDeltaRadX = nDeltaX * PI / 180.0f * 0.1f;
    //         float fDeltaRadY = nDeltaY * PI / 180.0f * 0.1f;
    //         m_RotX += fDeltaRadX;
    //         m_RotY += fDeltaRadY;
    //         float RotY = Clamp<float>(m_RotY, -1.5f, 1.5f);
    //         Quat qRotHorizontal(Vec3::s_UnitY, m_RotX);
    //         Vec3 vForward = qRotHorizontal * Vec3::s_UnitZ;
    //         Vec3 vRight = Vec3::s_UnitY.Cross(vForward);
    //         Quat qRotVertical(vRight, RotY);
    //         Vec3 vDir = qRotVertical * vForward;
    //         // const Vec3 &eye = m_pCamera->GetEye();
    //         const Vec3 &lookat = m_pCamera->GetLookat();
    //         // Vec3 dir = lookat - eye;
    //         // dir.Normalize();
    //         // Vec3 up(0.0f, 1.0f, 0.0f);
    //         // bool bHorizontal = abs(nDeltaX) >= abs(nDeltaY);
    //         // if (bHorizontal) {
    //         //     Quat rot(up, nDeltaX * 0.05f);
    //         //     dir = rot * dir;
    //         // }
    //         // else {
    //         //     Vec3 right = up.Cross(dir);
    //         //     right.Normalize();
    //         //     Quat rot(right, nDeltaY * 0.05f);
    //         //     dir = rot * dir;
    //         // }
    //         m_pCamera->SetLookat(lookat + vDir * 1.0f);
    //     }
    // }

    // int dx, dy;
    // Global::m_pDisplay->GetClientPosition(dx, dy);
    // Global::m_pPlatform->SetCursorPos(dx + (int)w, dy + (int)h);
    // m_LastX = x; m_LastY = y;
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
    RotateCamera();
    
    if (m_MovingState > 0) 
    {
        Vec3 eye = m_pCamera->GetEye();
        Vec3 lookat = m_pCamera->GetLookat();
        Vec3 dir = lookat - eye;
        float length = dir.Normalize();
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
        m_pCamera->SetLookat(eye + dir * length);
    }
}

void CFirstPersonCameraController::RotateCamera()
{
    static bool bFirstTime = true;

    IPlatform * __restrict pPlatform = Global::m_pPlatform;

    int w, h;
    pPlatform->GetDesktopDimension(w, h);
    w = w >> 1;
    h = h >> 1;

    if (bFirstTime)
    {
        bFirstTime = false;
    }
    else
    {
        int x, y;
        pPlatform->GetCursorPos(x, y);
        int nDeltaX = (int)(x - w);
        int nDeltaY = (int)(y - h);
        float fDeltaRadX = nDeltaX * PI / 180.0f * 0.1f;
        float fDeltaRadY = nDeltaY * PI / 180.0f * 0.1f;
        m_RotX += fDeltaRadX;
        m_RotY += fDeltaRadY;
        float RotY = Clamp<float>(m_RotY, -1.5f, 1.5f);
        Quat qRotHorizontal(Vec3::s_UnitY, m_RotX);
        Vec3 vForward = qRotHorizontal * Vec3::s_UnitZ;
        Vec3 vRight = Vec3::s_UnitY.Cross(vForward);
        Quat qRotVertical(vRight, RotY);
        Vec3 vDir = qRotVertical * vForward;
        const Vec3 &eye = m_pCamera->GetEye();
        m_pCamera->SetLookat(eye + vDir * 1.0f);
    }
    
    pPlatform->SetCursorPos(w, h);
}