#pragma once

#include "ReferencedObject.h"

class CBaseResource : public CReferencedObject {
public:
	CBaseResource() { m_CreatedOrLoaded = false; }
	bool IsCreatedOrLoaded() { return m_CreatedOrLoaded; }
protected:
	std::atomic_bool m_CreatedOrLoaded;
};