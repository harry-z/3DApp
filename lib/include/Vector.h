#pragma once

#include "PreHeader.h"

struct Vec2 {
	float x, y;

	Vec2() : x(0.0f), y(0.0f) {}
	Vec2(float x_, float y_) : x(x_), y(y_) {}
	Vec2(float u) : x(u), y(u) {}
	Vec2(const Vec2 &other) : x(other.x), y(other.y) {}

	Vec2& operator= (const Vec2 &other) {
		x = other.x; y = other.y;
		return *this;
	}
	Vec2 operator* (const Vec2 &other) const {
		return Vec2(x * other.x, y * other.y);
	}
	void Set(float x_, float y_) {
		x = x_; y = y_;
	}

	// (0, 0)
	static DLL_EXPORT const Vec2 s_ZeroVector;
	// (1, 1)
	static DLL_EXPORT const Vec2 s_UnitVector;
};

struct Vec2d
{
	double x, y;
	Vec2d() : x(0.0), y(0.0) {}
	Vec2d(double x_, double y_) : x(x_), y(y_) {}
	Vec2d(double u) : x(u), y(u) {}
	Vec2d(const Vec2d &other) : x(other.x), y(other.y) {}
	Vec2d& operator= (const Vec2d &other) {
		x = other.x; y = other.y;
		return *this;
	}
};

struct Vec3 {
	float x, y, z;
	
	Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	Vec3(float u) : x(u), y(u), z(u) {}
	Vec3(const Vec3 &other) : x(other.x), y(other.y), z(other.z) {}

	Vec3& operator= (const Vec3 &other) {
		x = other.x; y = other.y; z = other.z;
		return *this;
	}
	bool operator== (const Vec3 &other) {
		return x == other.x && y == other.y && z == other.z;
	}
	Vec3& operator+= (const Vec3 &other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}
	Vec3 operator+ (const Vec3 &other) const {
		Vec3 result(x, y, z);
		result += other;
		return result;
	}
	Vec3& operator-= (const Vec3 &other) {
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}
	Vec3 operator- (const Vec3 &other) const {
		Vec3 result(*this);
		result -= other;
		return result;
	}
	Vec3& operator*= (float scalar) {
		x *= scalar; y *= scalar; z *= scalar;
		return *this;
	}
	Vec3 operator* (float scalar) const {
		Vec3 result(*this);
		result *= scalar;
		return result;
	}
	Vec3 operator* (const Vec3 &other) const {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}
	Vec3 operator- () const {
		return Vec3(-x, -y, -z);
	}
	void Set(float x_, float y_, float z_) {
		x = x_; y = y_; z = z_;
	}
	void Set(float u) {
		x = y = z = u;
	}
	float Length() const {
		return sqrtf(x*x + y*y + z*z);
	}
	float SquareLength() const {
		return x*x + y*y + z*z;
	}
	float Normalize() {
		float l = Length();
		// Vector length can not be zero
		assert(l > EPSILON);
		float OneOverl = 1.0f / l;
		x *= OneOverl; y *= OneOverl; z *= OneOverl;
		return l;
	}
	float Dot(const Vec3 &other) const {
		return x*other.x + y*other.y + z*other.z;
	}
	Vec3 Cross(const Vec3 &other) const {
		return Vec3(
			y*other.z - z*other.y, 
			z*other.x - x*other.z, 
			x*other.y - y*other.x);
	}
	void SRGBToLinear() {
		x = powf(x, 2.2f);
		y = powf(y, 2.2f);
		z = powf(z, 2.2f);
	}
	Vec3 MakeSRGBToLinear() const {
		Vec3 ret(*this);
		ret.SRGBToLinear();
		return ret;
	}
	void LinearToSRGB() {
		float expo = 1.0f / 2.2f;
		x = powf(x, expo);
		y = powf(y, expo);
		z = powf(z, expo);
	}
	Vec3 MakeLinearToSRGB() const {
		Vec3 ret(*this);
		ret.LinearToSRGB();
		return ret;
	}

	// (0, 0, 0)
	static DLL_EXPORT const Vec3 s_ZeroVector;
	// (1, 0, 0)
	static DLL_EXPORT const Vec3 s_UnitX;
	// (0, 1, 0)
	static DLL_EXPORT const Vec3 s_UnitY;
	// (0, 0, 1)
	static DLL_EXPORT const Vec3 s_UnitZ;
	// (1, 1, 1)
	static DLL_EXPORT const Vec3 s_UnitVector;
};

struct Vec3d
{
	double x, y, z;

	Vec3d() : x(0.0), y(0.0), z(0.0) {}
	Vec3d(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
	Vec3d(double u) : x(u), y(u), z(u) {}
	Vec3d(const Vec3d &other) : x(other.x), y(other.y), z(other.z) {}

	Vec3d& operator= (const Vec3d &other) {
		x = other.x; y = other.y; z = other.z;
		return *this;
	}

	Vec3d& operator+= (const Vec3d &other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}
	Vec3d operator+ (const Vec3d &other) const {
		Vec3d result(x, y, z);
		result += other;
		return result;
	}

	Vec3d& operator*= (double scalar) {
		x *= scalar; y *= scalar; z *= scalar;
		return *this;
	}

	Vec3d operator* (double scalar) const {
		Vec3d result(*this);
		result *= scalar;
		return result;
	}
};

struct Vec4 {
	float x, y, z, w;

	Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
	Vec4(const Vec3 &v3, float w_) : x(v3.x), y(v3.y), z(v3.z), w(w_) {}
	Vec4(const Vec4 &other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

	Vec4& operator= (const Vec4 &other) {
		x = other.x; y = other.y; z = other.z; w = other.w;
		return *this;
	}
	Vec4 operator* (const Vec4 &other) const {
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
	void Set(float x_, float y_, float z_, float w_) {
		x = x_; y = y_; z = z_; w = w_;
	}

	// (0, 0, 0, 0)
	static DLL_EXPORT const Vec4 s_ZeroVector;
	// (1, 1, 1, 1)
	static DLL_EXPORT const Vec4 s_UnitVector;
};