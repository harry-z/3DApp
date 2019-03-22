#include "Matrix.h"

#define MINOR(r1, r2, r3, c1, c2, c3) \
		(m##r1##c1 * (m##r2##c2 * m##r3##c3 - m##r3##c2 * m##r2##c3) - \
		m##r1##c2 * (m##r2##c1 * m##r3##c3 - m##r3##c1 * m##r2##c3) + \
		m##r1##c3 * (m##r2##c1 * m##r3##c2 - m##r3##c1 * m##r2##c2))

Matrix4& Matrix4::operator*= (const Matrix4 &other) {
	float tm11 = m11 * other.m11 + m12 * other.m21 + m13 * other.m31 + m14 * other.m41;
	float tm12 = m11 * other.m12 + m12 * other.m22 + m13 * other.m32 + m14 * other.m42;
	float tm13 = m11 * other.m13 + m12 * other.m23 + m13 * other.m33 + m14 * other.m43;
	float tm14 = m11 * other.m14 + m12 * other.m24 + m13 * other.m34 + m14 * other.m44;

	float tm21 = m21 * other.m11 + m22 * other.m21 + m23 * other.m31 + m24 * other.m41;
	float tm22 = m21 * other.m12 + m22 * other.m22 + m23 * other.m32 + m24 * other.m42;
	float tm23 = m21 * other.m13 + m22 * other.m23 + m23 * other.m33 + m24 * other.m43;
	float tm24 = m21 * other.m14 + m22 * other.m24 + m23 * other.m34 + m24 * other.m44;

	float tm31 = m31 * other.m11 + m32 * other.m21 + m33 * other.m31 + m34 * other.m41;
	float tm32 = m31 * other.m12 + m32 * other.m22 + m33 * other.m32 + m34 * other.m42;
	float tm33 = m31 * other.m13 + m32 * other.m23 + m33 * other.m33 + m34 * other.m43;
	float tm34 = m31 * other.m14 + m32 * other.m24 + m33 * other.m34 + m34 * other.m44;

	float tm41 = m41 * other.m11 + m42 * other.m21 + m43 * other.m31 + m44 * other.m41;
	float tm42 = m41 * other.m12 + m42 * other.m22 + m43 * other.m32 + m44 * other.m42;
	float tm43 = m41 * other.m13 + m42 * other.m23 + m43 * other.m33 + m44 * other.m43;
	float tm44 = m41 * other.m14 + m42 * other.m24 + m43 * other.m34 + m44 * other.m44;

	m11 = tm11; m12 = tm12; m13 = tm13; m14 = tm14;
	m21 = tm21; m22 = tm22; m23 = tm23; m24 = tm24;
	m31 = tm31; m32 = tm32; m33 = tm33; m34 = tm34;
	m41 = tm41; m42 = tm42; m43 = tm43; m44 = tm44;
	return *this;
}

Matrix4 Matrix4::operator* (const Matrix4 &other) const {
	Matrix4 ret;
	ret.m11 = m11 * other.m11 + m12 * other.m21 + m13 * other.m31 + m14 * other.m41;
	ret.m12 = m11 * other.m12 + m12 * other.m22 + m13 * other.m32 + m14 * other.m42;
	ret.m13 = m11 * other.m13 + m12 * other.m23 + m13 * other.m33 + m14 * other.m43;
	ret.m14 = m11 * other.m14 + m12 * other.m24 + m13 * other.m34 + m14 * other.m44;

	ret.m21 = m21 * other.m11 + m22 * other.m21 + m23 * other.m31 + m24 * other.m41;
	ret.m22 = m21 * other.m12 + m22 * other.m22 + m23 * other.m32 + m24 * other.m42;
	ret.m23 = m21 * other.m13 + m22 * other.m23 + m23 * other.m33 + m24 * other.m43;
	ret.m24 = m21 * other.m14 + m22 * other.m24 + m23 * other.m34 + m24 * other.m44;

	ret.m31 = m31 * other.m11 + m32 * other.m21 + m33 * other.m31 + m34 * other.m41;
	ret.m32 = m31 * other.m12 + m32 * other.m22 + m33 * other.m32 + m34 * other.m42;
	ret.m33 = m31 * other.m13 + m32 * other.m23 + m33 * other.m33 + m34 * other.m43;
	ret.m34 = m31 * other.m14 + m32 * other.m24 + m33 * other.m34 + m34 * other.m44;

	ret.m41 = m41 * other.m11 + m42 * other.m21 + m43 * other.m31 + m44 * other.m41;
	ret.m42 = m41 * other.m12 + m42 * other.m22 + m43 * other.m32 + m44 * other.m42;
	ret.m43 = m41 * other.m13 + m42 * other.m23 + m43 * other.m33 + m44 * other.m43;
	ret.m44 = m41 * other.m14 + m42 * other.m24 + m43 * other.m34 + m44 * other.m44;

	return ret;
}

void Matrix4::FromQuat(const Quat &q) {
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q.x + q.x;
	y2 = q.y + q.y;
	z2 = q.z + q.z;
	xx = q.x * x2;
	xy = q.x * y2;
	xz = q.x * z2;
	yy = q.y * y2;
	yz = q.y * z2;
	zz = q.z * z2;
	wx = q.w * x2;
	wy = q.w * y2;
	wz = q.w * z2;

	m11 = 1.0f - (yy + zz);
	m21 = xy - wz;
	m31 = xz + wy;
	m41 = 0.0f;

	m12 = xy + wz;
	m22 = 1.0f - (xx + zz);
	m32 = yz - wx;
	m42 = 0.0f;

	m13 = xz - wy;
	m23 = yz + wx;
	m33 = 1.0f - (xx + yy);
	m43 = 0.0f;

	m14 = 0.0f;
	m24 = 0.0f;
	m34 = 0.0f;
	m44 = 1.0f;
}

void Matrix4::MakeTransform(const Vec3 &translation, const Quat &rotation, const Vec3 &scaling) {
	Matrix4 rot;
	rot.FromQuat(rotation);

	m11 = scaling.x * rot.m11;
	m12 = scaling.x * rot.m12;
	m13 = scaling.x * rot.m13;

	m21 = scaling.y * rot.m21;
	m22 = scaling.y * rot.m22;
	m23 = scaling.y * rot.m23;

	m31 = scaling.z * rot.m31;
	m32 = scaling.z * rot.m32;
	m33 = scaling.z * rot.m33;

	m14 = m24 = m34 = 0.0f;

	m41 = translation.x;
	m42 = translation.y;
	m43 = translation.z;
	m44 = 1.0f;
}

float Matrix4::Determinate() const {
	return m11 * MINOR(2, 3, 4, 2, 3, 4) -
		m12 * MINOR(2, 3, 4, 1, 3, 4) +
		m13 * MINOR(2, 3, 4, 1, 2, 4) -
		m14 * MINOR(2, 3, 4, 1, 2, 3);
}

void Matrix4::Inverse() {
	Matrix4 adjoint(
		MINOR(2, 3, 4, 2, 3, 4),
		-MINOR(1, 3, 4, 2, 3, 4),
		MINOR(1, 2, 4, 2, 3, 4),
		-MINOR(1, 2, 3, 2, 3, 4),

		-MINOR(2, 3, 4, 1, 3, 4),
		MINOR(1, 3, 4, 1, 3, 4),
		-MINOR(1, 2, 4, 1, 3, 4),
		MINOR(1, 2, 3, 1, 3, 4),

		MINOR(2, 3, 4, 1, 2, 4),
		-MINOR(1, 3, 4, 1, 2, 4),
		MINOR(1, 2, 4, 1, 2, 4),
		-MINOR(1, 2, 3, 1, 2, 4),

		-MINOR(2, 3, 4, 1, 2, 3),
		MINOR(1, 3, 4, 1, 2, 3),
		-MINOR(1, 2, 4, 1, 2, 3),
		MINOR(1, 2, 3, 1, 2, 3)
		);
	adjoint *= (1.0f / Determinate());
	operator= (adjoint);
}

void Matrix4::InverseFast() {
	Vec3 xaxis(m11, m12, m13);
	Vec3 yaxis(m21, m22, m23);
	Vec3 zaxis(m31, m32, m33);
	Vec3 translate(m41, m42, m43);
	Transpose();
	m14 = m24 = m34 = 0.0f;
	m41 = -xaxis.Dot(translate);
	m42 = -yaxis.Dot(translate);
	m43 = -zaxis.Dot(translate);
}