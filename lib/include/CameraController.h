#pragma once

#include "InputHandler.h"

class CCamera;
class ICameraController : public IInputHandler {
public:
	ICameraController(CCamera *pCamera) : m_pCamera(pCamera) {}
	virtual ~ICameraController() {}
	virtual const char* Name() const { return nullptr; }
	virtual void Update() {};
protected:
    CCamera *m_pCamera;
};