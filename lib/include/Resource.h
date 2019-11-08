#pragma once

#include "ReferencedObject.h"

class CBaseResource : public CReferencedObject {
public:
	CBaseResource() { m_CreatedOrLoaded = false; }
	inline void CreatedOrLoaded() { m_CreatedOrLoaded = true; }
	inline bool IsCreatedOrLoaded() const { return m_CreatedOrLoaded; }
protected:
	std::atomic_bool m_CreatedOrLoaded;
};