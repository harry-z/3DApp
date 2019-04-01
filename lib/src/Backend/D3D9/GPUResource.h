#pragma once

#include "LinkList.h"

struct IGpuResource {
	IGpuResource() { m_node.m_pOwner = this; }
	virtual ~IGpuResource() {}
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceReset(float w_changed_ratio, float h_changed_ratio) = 0;
	LinklistNode<IGpuResource> m_node;
};