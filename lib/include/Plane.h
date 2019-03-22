#pragma once

#include "AABB.h"

struct Plane {
	Vec3 normal;
	float dis;
	Plane() : dis(0.0f) {}
	Plane(const Vec3 &normal_, float dis_) : normal(normal_), dis(dis_) {}
	Plane(float nx_, float ny_, float nz_, float dis_) : normal(Vec3(nx_, ny_, nz_)), dis(dis_) {}
	Plane(const Plane &other) : normal(other.normal), dis(other.dis) {}
	Plane& operator= (const Plane &other) {
		normal = other.normal; dis = other.dis;
		return *this;
	}

	void Set(float nx_, float ny_, float nz_, float dis_) {
		normal.x = nx_; normal.y = ny_; normal.z = nz_; dis = dis_;
	}
	void Set(const Vec3 &normal_, float dis_) {
		normal = normal_; dis = dis_;
	}
	void FromPoints(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3) {
		Vec3 v12 = v2 - v1;
		Vec3 v13 = v3 - v1;
		// Check if the two vectors are parallel
		assert(1.0f - fabs(v12.Dot(v13)) < EPSILON);
		normal = v12.Cross(v13);
		normal.Normalize();
		dis = -normal.Dot(v1);
	}
	void FromNormalAndPoint(const Vec3 &normal_, const Vec3 &pointOnPlane) {
		normal = normal_;
		dis = -normal.Dot(pointOnPlane);
	}
	float Distance(const Vec3 &pos) const {
		return normal.Dot(pos) + dis;
	}
	int Side(const Vec3 &pos) const {
		float d = Distance(pos);
		if (d > 0.0f) return 1;
		else if (d < 0.0f) return -1;
		else return 0;
	}
	int Side(const AxisAlignedBox &aab) const {
		Vec3 center = aab.Center();
		Vec3 halfSize = aab.HalfSize();
		float d = Distance(center);
		float maxAbsDis = fabs(normal.x*halfSize.x) + fabs(normal.y*halfSize.y) + fabs(normal.z*halfSize.z);
		if (d < -maxAbsDis) return -1;
		if (d > maxAbsDis) return 1;
		return 0;
	}
	void Normalize() {
		float l = normal.Normalize();
		assert(l > EPSILON);
		dis /= l;
	}
	Vec3 MirrorVector(const Vec3 &v) const {
		return normal * (2 * normal.Dot(v)) - v;
	}
	Vec3 MirrorPoint(const Vec3 &p) const {
		return p - normal * 2 * (normal.Dot(p) + dis);
	}
};