#pragma once

#include "Matrix.h"

struct AxisAlignedBox {
	Vec3 lb, rt;

	AxisAlignedBox() {}
	AxisAlignedBox(const Vec3 &lb_, const Vec3 &rt_) : lb(lb_), rt(rt_) {}
	AxisAlignedBox(const AxisAlignedBox &other) : lb(other.lb), rt(other.rt) {}
	AxisAlignedBox& operator= (const AxisAlignedBox &other) {
		lb = other.lb; rt = other.rt;
		return *this;
	}

	bool IsValid() const { return rt.x >= lb.x && rt.y >= lb.y && rt.z >= lb.z; }
	Vec3 Center() const { return (lb + rt) * 0.5f; }
	Vec3 HalfSize() const { return (rt - lb) * 0.5f; }
	float Radius() const { return (rt - lb).Length() * 0.5f; }
	void Move(const Vec3 &v) { lb += v; rt += v; }
	void Scale(float scalar) { lb *= scalar; rt *= scalar; }
	void ExpandBy(const Vec3 &v) {
		lb.x = min(lb.x, v.x);
		lb.y = min(lb.y, v.y);
		lb.z = min(lb.z, v.z);
		rt.x = max(rt.x, v.x);
		rt.y = max(rt.y, v.y);
		rt.z = max(rt.z, v.z);
	}
	void ExpandBy(const AxisAlignedBox &aab) {
		assert(aab.IsValid());
		lb.x = min(lb.x, aab.lb.x);
		lb.y = min(lb.y, aab.lb.y);
		lb.z = min(lb.z, aab.lb.z);
		rt.x = max(rt.x, aab.rt.x);
		rt.y = max(rt.y, aab.rt.y);
		rt.z = max(rt.z, aab.rt.z);
	}
	bool Contains(const Vec3 &pos) const {
		if (pos.x < lb.x) return false;
		if (pos.y < lb.y) return false;
		if (pos.z < lb.z) return false;
		if (pos.x > rt.x) return false;
		if (pos.y > rt.y) return false;
		if (pos.z > rt.z) return false;
		return true;
	}
	bool Contains(const AxisAlignedBox &aab) {
		assert(aab.IsValid());
		return lb.x <= aab.lb.x && lb.y <= aab.lb.y && lb.z <= aab.lb.z &&
			rt.x >= aab.rt.x && rt.y >= aab.rt.y && rt.z >= aab.rt.z;
	}
	bool Intersects(const AxisAlignedBox &aab) {
		assert(aab.IsValid());
		if (lb.x > aab.rt.x || rt.x < aab.lb.x) return false;
		if (lb.y > aab.rt.y || rt.y < aab.lb.y) return false;
		if (lb.z > aab.rt.z || rt.z < aab.lb.z) return false;
		return true;
	}
	void Transform(const Matrix4 &matrix) {
		Vec3 tlb = lb;
		Vec3 trt = rt;
		Vec3 current = lb;
		lb = rt = matrix * current;
		current.z = trt.z;
		ExpandBy(matrix * current);
		current.y = trt.y;
		ExpandBy(matrix * current);
		current.z = tlb.z;
		ExpandBy(matrix * current);
		current.x = trt.x;
		ExpandBy(matrix * current);
		current.z = trt.z;
		ExpandBy(matrix * current);
		current.y = tlb.y;
		ExpandBy(matrix * current);
		current.z = tlb.z;
		ExpandBy(matrix * current);
	}
};