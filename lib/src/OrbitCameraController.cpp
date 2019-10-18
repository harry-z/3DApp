#include "OrbitCameraController.h"
#include "Camera.h"

COrbitCameraController::COrbitCameraController(CCamera *pCamera)
: ICameraController(pCamera)
{
    m_Distance = (pCamera->GetEye() - pCamera->GetLookat()).Length();
}

void COrbitCameraController::OnMouseMove(dword x, dword y)
{
    static bool bFirstTime = true;
    if (!bFirstTime)
    {

        bFirstTime = false;
    }

    m_LastX = x;
    m_LastY = y;
}

void COrbitCameraController::OnKeyDown(EKeyCode KeyCode)
{

}

void COrbitCameraController::OnKeyUp(EKeyCode KeyCode)
{

}

void COrbitCameraController::Update()
{
    
}