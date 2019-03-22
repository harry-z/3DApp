#pragma once

#include "Vector.h"

struct Quat {
	float x, y, z, w;
	Quat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quat(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
	Quat(const Vec3 &axis, float angle) {
		FromAngleAxis(axis, angle);
	}
	Quat(const Quat &other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
	Quat& operator= (const Quat &other) {
		x = other.x; y = other.y; z = other.z; w = other.w;
		return *this;
	}
	Quat& operator+= (const Quat &other) {
		x += other.x; y += other.y; z += other.z; w += other.w;
		return *this;
	}
	Quat operator+ (const Quat &other) const {
		Quat ret(*this);
		ret += other;
		return ret;
	}
	Quat& operator-= (const Quat &other) {
		x -= other.x; y -= other.y; z -= other.z; w -= other.w;
		return *this;
	}
	Quat operator- (const Quat &other) const {
		Quat ret(*this);
		ret -= other;
		return ret;
	}
	Quat& operator*= (const Quat &other) {
		float tx = w * other.x + x * other.w + y * other.z - z * other.y;
		float ty = w * other.y + y * other.w + z * other.x - x * other.z;
		float tz = w * other.z + z * other.w + x * other.y - y * other.x;
		float tw = w * other.w - x * other.x - y * other.y - z * other.z;
		x = tx; y = ty; z = tz; w = tw;
		return *this;
	}
	Quat operator* (const Quat &other) const {
		Quat ret(*this);
		ret *= other;
		return ret;
	}
	Quat& operator*= (float scalar) {
		x *= scalar; y *= scalar; z *= scalar; w *= scalar;
		return *this;
	}
	Quat operator* (float scalar) const {
		Quat ret(*this);
		ret *= scalar;
		return ret;
	}
	Vec3 operator* (const Vec3 &v) const {
		Vec3 axis(x,y,z), uv, uuv;
		uv = axis.Cross(v);
		uuv = axis.Cross(uv);
		uv *= (2.0f*w);
		uuv *= 2.0f;
		return v + uv + uuv;
	}
	Quat operator- () const { return Quat(-x, -y, -z, -w); }
	void Set(float x_, float y_, float z_, float w_) {
		x = x_; y = y_; z = z_; w = w_;
	}
	void FromAngleAxis(const Vec3 &axis, float angle) {
		float hSin = sinf(angle*0.5f);
		float hCos = cosf(angle*0.5f);
		x = axis.x * hSin;
		y = axis.y * hSin;
		z = axis.z * hSin;
		w = hCos;
	}
	float GetAngle() const {
		return acosf(w) * 2.0f;
	}
	Vec3 GetAxis() const {
		float sinh = sqrtf(1.0f - w*w);
		if (sinh < EPSILON) // 180 degree
			return Vec3();
		sinh = 1.0f / sinh;
		return Vec3(x*sinh, y*sinh, z*sinh);
	}
	void Normalize() {
		float l = sqrtf(x*x + y*y + z*z + w*w);
		assert(l > EPSILON);
		l = 1.0f / l;
		x *= l; y *= l; z *= l; w *= l;
	}
	float Dot(const Quat &other) const {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
	void Nlerp(const Quat &from, const Quat &to, float mix) {
		Quat tto = to;
		if (from.Dot(tto) < 0.0f) { tto = -tto; }
		*this = (tto - from) * mix + from;
		Normalize();
	}
	void Slerp(const Quat &from, const Quat &to, float mix) {
		Quat tto = to;
		float cosine = from.Dot(tto);
		if (cosine < 0.0f) { cosine = -cosine; tto = -tto; }
		if (fabs(cosine) < 1.0f - EPSILON) {
			// A standard slerp
			float sine = sqrtf(1.0f - cosine * cosine);
			float angle = atan2f(sine, cosine);
			sine = 1.0f / sine;
			float fCoeff0 = sinf((1.0f - mix) * angle) * sine;
			float fCoeff1 = sinf(mix * angle) * sine;
			*this = from * fCoeff0 + to * fCoeff1;
		}
		else {
			*this = (tto - from) * mix + from;
			Normalize();
		}
	}
};