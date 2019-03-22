#pragma once
#include "Quat.h"



struct DLL_EXPORT Matrix4 {
	union {
		struct {
			float row1[4];
			float row2[4];
			float row3[4];
			float row4[4];
		};
		struct {
			float m11, m12, m13, m14;
			float m21, m22, m23, m24;
			float m31, m32, m33, m34;
			float m41, m42, m43, m44;
		};
		float m[16];
	};

	Matrix4() {}
	Matrix4(float m11_, float m12_, float m13_, float m14_,
		float m21_, float m22_, float m23_, float m24_,
		float m31_, float m32_, float m33_, float m34_,
		float m41_, float m42_, float m43_, float m44_) {
		m11 = m11_; m12 = m12_; m13 = m13_; m14 = m14_;
		m21 = m21_; m22 = m22_; m23 = m23_; m24 = m24_;
		m31 = m31_; m32 = m32_; m33 = m33_; m34 = m34_;
		m41 = m41_; m42 = m42_; m43 = m43_; m44 = m44_;
	}
	Matrix4(const Quat &q) {
		FromQuat(q);
	}
	Matrix4(const Matrix4 &other) 
	: m11(other.m11), m12(other.m12), m13(other.m13), m14(other.m14), 
	m21(other.m21), m22(other.m22), m23(other.m23), m24(other.m24), 
	m31(other.m31), m32(other.m32), m33(other.m33), m34(other.m34),
	m41(other.m41), m42(other.m42), m43(other.m43), m44(other.m44) {}

	Matrix4& operator= (const Matrix4 &other) {
		Set(other.m11, other.m12, other.m13, other.m14,
			other.m21, other.m22, other.m23, other.m24,
			other.m31, other.m32, other.m33, other.m34,
			other.m41, other.m42, other.m43, other.m44);
		return *this;
	}
	Matrix4& operator+= (const Matrix4 &other) {
		m11 += other.m11; m12 += other.m12; m13 += other.m13; m14 += other.m14;
		m21 += other.m21; m22 += other.m22; m23 += other.m23; m24 += other.m24;
		m31 += other.m31; m32 += other.m32; m33 += other.m33; m34 += other.m34;
		m41 += other.m41; m42 += other.m42; m43 += other.m43; m44 += other.m44;
		return *this;
	}
	Matrix4 operator+ (const Matrix4 &other) const {
		Matrix4 ret(*this);
		ret += other;
		return ret;
	}
	Matrix4& operator-= (const Matrix4 &other) {
		m11 -= other.m11; m12 -= other.m12; m13 -= other.m13; m14 -= other.m14;
		m21 -= other.m21; m22 -= other.m22; m23 -= other.m23; m24 -= other.m24;
		m31 -= other.m31; m32 -= other.m32; m33 -= other.m33; m34 -= other.m34;
		m41 -= other.m41; m42 -= other.m42; m43 -= other.m43; m44 -= other.m44;
		return *this;
	}
	Matrix4 operator- (const Matrix4 &other) const {
		Matrix4 ret(*this);
		ret -= other;
		return ret;
	}
	Matrix4& operator*= (float scalar) {
		m11 *= scalar; m12 *= scalar; m13 *= scalar; m14 *= scalar;
		m21 *= scalar; m22 *= scalar; m23 *= scalar; m24 *= scalar;
		m31 *= scalar; m32 *= scalar; m33 *= scalar; m34 *= scalar;
		m41 *= scalar; m42 *= scalar; m43 *= scalar; m44 *= scalar;
		return *this;
	}
	Matrix4 operator* (float scalar) const {
		Matrix4 ret(*this);
		ret *= scalar;
		return ret;
	}
	Matrix4& operator*= (const Matrix4 &other);
	Matrix4 operator* (const Matrix4 &other) const;
	Vec3 operator* (const Vec3 &vector) const {
		Vec3 ret;
		// vector * matrix
		float iw = 1.0f / (vector.x * m14 + vector.y * m24 + vector.z * m34 + m44);
		ret.x = (vector.x * m11 + vector.y * m21 + vector.z * m31 + m41) * iw;
		ret.y = (vector.x * m12 + vector.y * m22 + vector.z * m32 + m42) * iw;
		ret.z = (vector.x * m13 + vector.y * m23 + vector.z * m33 + m43) * iw;
		return ret;
	}
	Vec4 operator* (const Vec4 &vector) const {
		Vec4 ret;
		// vector * matrix
		ret.x = vector.x * m11 + vector.y * m21 + vector.z * m31 + vector.w * m41;
		ret.y = vector.x * m12 + vector.y * m22 + vector.z * m32 + vector.w * m42;
		ret.z = vector.x * m13 + vector.y * m23 + vector.z * m33 + vector.w * m43;
		ret.w = vector.x * m14 + vector.y * m24 + vector.z * m34 + vector.w * m44;
		return ret;
	}
	void Set(float m11_, float m12_, float m13_, float m14_,
		float m21_, float m22_, float m23_, float m24_,
		float m31_, float m32_, float m33_, float m34_,
		float m41_, float m42_, float m43_, float m44_) {
		m11 = m11_; m12 = m12_; m13 = m13_; m14 = m14_;
		m21 = m21_; m22 = m22_; m23 = m23_; m24 = m24_;
		m31 = m31_; m32 = m32_; m33 = m33_; m34 = m34_;
		m41 = m41_; m42 = m42_; m43 = m43_; m44 = m44_;
	}
	void MakeIdentity() {
		m11 = 1.0f; m12 = 0.0f; m13 = 0.0f; m14 = 0.0f;
		m21 = 0.0f; m22 = 1.0f; m23 = 0.0f; m24 = 0.0f;
		m31 = 0.0f; m32 = 0.0f; m33 = 1.0f; m34 = 0.0f;
		m41 = 0.0f; m42 = 0.0f; m43 = 0.0f; m44 = 1.0f;
	}
	void SetTranslation(const Vec3 &trans) {
		m41 = trans.x; m42 = trans.y; m43 = trans.z;
	}
	void MakeTranslation(const Vec3 &trans) {
		m11 = 1.0f; m12 = 0.0f; m13 = 0.0f; m14 = 0.0f;
		m21 = 0.0f; m22 = 1.0f; m23 = 0.0f; m24 = 0.0f;
		m31 = 0.0f; m32 = 0.0f; m33 = 1.0f; m34 = 0.0f;
		m41 = trans.x; m42 = trans.y; m43 = trans.z; m44 = 1.0f;
	}
	Vec3 GetTranslation() const { return Vec3(m41, m42, m43); }
	static Matrix4 GetTranslationMatrix(const Vec3 &trans) {
		return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			trans.x, trans.y, trans.z, 1.0f);
	}
	void SetScale(const Vec3 &scale) {
		m11 = scale.x; m22 = scale.y; m33 = scale.z;
	}
	void MakeScale(const Vec3 &scale) {
		m11 = scale.x; m12 = 0.0f; m13 = 0.0f; m14 = 0.0f;
		m21 = 0.0f; m22 = scale.y; m23 = 0.0f; m24 = 0.0f;
		m31 = 0.0f; m32 = 0.0f; m33 = scale.z; m34 = 0.0f;
		m41 = 0.0f; m42 = 0.0f; m43 = 0.0f; m44 = 1.0f;
	}
	static Matrix4 GetScaleMatrix(const Vec3 &scale) {
		return Matrix4(scale.x, 0.0f, 0.0f, 0.0f,
			0.0f, scale.y, 0.0f, 0.0f,
			0.0f, 0.0f, scale.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	void RotX(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
		m21 = 0.0f; m22 = fCos; m23 = fSin;
		m31 = 0.0f; m32 = -fSin; m33 = fCos;
	}
	void MakeRotX(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = 1.0f; m12 = 0.0f; m13 = 0.0f; m14 = 0.0f;
		m21 = 0.0f; m22 = fCos; m23 = fSin; m24 = 0.0f;
		m31 = 0.0f; m32 = -fSin; m33 = fCos; m34 = 0.0f;
		m41 = 0.0f; m42 = 0.0f; m43 = 0.0f; m44 = 1.0f;
	}
	static Matrix4 GetRotXMatrix(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		return Matrix4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, fCos, fSin, 0.0f,
			0.0f, -fSin, fCos, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	void RotY(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = fCos; m12 = 0.0f; m13 = -fSin;
		m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
		m31 = fSin; m32 = 0.0f; m33 = fCos;
	}
	void MakeRotY(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = fCos; m12 = 0.0f; m13 = -fSin; m14 = 0.0f;
		m21 = 0.0f; m22 = 1.0f; m23 = 0.0f; m24 = 0.0f;
		m31 = fSin; m32 = 0.0f; m33 = fCos; m34 = 0.0f;
		m41 = 0.0f; m42 = 0.0f; m43 = 0.0f; m44 = 1.0f;
	}
	static Matrix4 GetRotYMatrix(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		return Matrix4(fCos, 0.0f, -fSin, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			fSin, 0.0f, fCos, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	void RotZ(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = fCos; m12 = fSin; m13 = 0.0f;
		m21 = -fSin; m22 = fCos; m23 = 0.0f;
		m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
	}
	void MakeRotZ(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		m11 = fCos; m12 = fSin; m13 = 0.0f; m14 = 0.0f;
		m21 = -fSin; m22 = fCos; m23 = 0.0f; m24 = 0.0f;
		m31 = 0.0f; m32 = 0.0f; m33 = 1.0f; m34 = 0.0f;
		m41 = 0.0f; m42 = 0.0f; m43 = 0.0f; m44 = 1.0f;
	}
	static Matrix4 GetRotZMatrix(float angle) {
		float fSin = sinf(angle);
		float fCos = cosf(angle);
		return Matrix4(fCos, fSin, 0.0f, 0.0f,
			-fSin, fCos, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	void FromQuat(const Quat &q);
	void MakeTransform(const Vec3 &translation, const Quat &rotation, const Vec3 &scaling);
	static Matrix4 GetTransformMatrix(const Vec3 &translation, const Quat &rotation, const Vec3 &scaling) {
		Matrix4 matrix;
		matrix.MakeTransform(translation, rotation, scaling);
		return matrix;
	}
	float Determinate() const;
	void Inverse();
	Matrix4 GetInverse() const {
		Matrix4 ret(*this);
		ret.Inverse();
		return ret;
	}
	void InverseFast();
	Matrix4 GetInverseFast() const {
		Matrix4 ret(*this);
		ret.InverseFast();
		return ret;
	}
	void Transpose() {
		float t = m12; m12 = m21; m21 = t;
		t = m13; m13 = m31; m31 = t;
		t = m14; m14 = m41; m41 = t;
		t = m23; m23 = m32; m32 = t;
		t = m24; m24 = m42; m42 = t;
		t = m34; m34 = m43; m43 = t;
	}
	Matrix4 GetTranspose() const {
		Matrix4 ret(*this);
		ret.Transpose();
		return ret;
	}
};

inline Vec3 operator* (const Vec3 &v, const Matrix4 &m) {
	return m * v;
}

inline Vec4 operator* (const Vec4 &v, const Matrix4 &m) {
	return m * v;
}