#include "Camera.h"

CCamera::CCamera()
	: m_nearclip(0.1f),
	m_farclip(1000.0f),
	m_dirty(CAMERA_VIEW_DIRTY | CAMERA_PROJ_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY | CAMERA_FRUSTUM_SIZE_DIRTY | CAMERA_NEED_VIEW_MATRIX_SHADER_CONST_UPDATE | CAMERA_NEED_PROJ_MATRIX_SHADER_CONST_UPDATE), 
	m_type(EProjectType::EProject_Perspective) {
	m_ProjectionData.perspective.m_fov = PI * 0.25f;
	m_ProjectionData.perspective.m_aspect = 1.3333f;
	m_lookat = -Vec3::s_UnitZ;
	m_up = Vec3::s_UnitY;
}

void CCamera::SetupViewMatrix() const {
	assert(BIT_CHECK(m_dirty, CAMERA_VIEW_DIRTY));
	// Left handed view matrix
	Vec3 zaxis = m_lookat - m_eye;
	zaxis.Normalize();
	Vec3 xaxis = m_up.Cross(zaxis);
	xaxis.Normalize();
	Vec3 yaxis = zaxis.Cross(xaxis);
	m_view.Set(xaxis.x, yaxis.x, zaxis.x, 0.0f,
		xaxis.y, yaxis.y, zaxis.y, 0.0f,
		xaxis.z, yaxis.z, zaxis.z, 0.0f,
		-xaxis.Dot(m_eye), -yaxis.Dot(m_eye), -zaxis.Dot(m_eye), 1.0f);
	BIT_REMOVE(m_dirty, CAMERA_VIEW_DIRTY);
}

const Matrix4& CCamera::GetViewMatrix() const {
	if (BIT_CHECK(m_dirty, CAMERA_VIEW_DIRTY))
		SetupViewMatrix();
	return m_view;
}

void CCamera::SetupProjMatrix() const {
	assert(BIT_CHECK(m_dirty, CAMERA_PROJ_DIRTY));
	if (m_type == EProjectType::EProject_Perspective) {
		// Left handed with z range 0.0 ~ 1.0
		float yScale = 1.0f / tanf(m_ProjectionData.perspective.m_fov * 0.5f);
		float xScale = yScale / m_ProjectionData.perspective.m_aspect;
		float zdiff = m_farclip - m_nearclip;
		m_projection.Set(xScale, 0.0f, 0.0f, 0.0f,
			0.0f, yScale, 0.0f, 0.0f,
			0.0f, 0.0f, m_farclip / zdiff, 1.0f,
			0.0f, 0.0f, -m_nearclip * m_farclip / zdiff, 0.0f);
	}
	else if (m_type == EProjectType::EProject_Ortho) {
		float f1 = m_ProjectionData.ortho.m_right - m_ProjectionData.ortho.m_left;
		float f2 = m_ProjectionData.ortho.m_top - m_ProjectionData.ortho.m_bottom;
		float zdiff = m_farclip - m_nearclip;
		m_projection.Set(2.0f / f1, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / f2, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f / zdiff, 0.0f,
			-(m_ProjectionData.ortho.m_right + m_ProjectionData.ortho.m_left) / f1,
			-(m_ProjectionData.ortho.m_top + m_ProjectionData.ortho.m_bottom) / f2,
			-m_nearclip / zdiff, 1.0f);
	}
	BIT_REMOVE(m_dirty, CAMERA_PROJ_DIRTY);
}

const Matrix4& CCamera::GetProjMatrix() const {
	if (BIT_CHECK(m_dirty, CAMERA_PROJ_DIRTY))
		SetupProjMatrix();
	return m_projection;
}

void CCamera::SetupFrustumPlanes() const
{
	assert(BIT_CHECK(m_dirty, CAMERA_FRUSTUM_PLANE_DIRTY));
	const Matrix4 &view = GetViewMatrix();
	const Matrix4 &projection = GetProjMatrix();
	Matrix4 viewproj = view * projection;

	dword nFrustumPlaneLeft = (dword)EFrustumPlane::EFrustumPlane_Left;
	dword nFrustumPlaneRight = (dword)EFrustumPlane::EFrustumPlane_Right;
	dword nFrustumPlaneTop = (dword)EFrustumPlane::EFrustumPlane_Top;
	dword nFrustumPlaneBottom = (dword)EFrustumPlane::EFrustumPlane_Bottom;
	dword nFrustumPlaneNear = (dword)EFrustumPlane::EFrustumPlane_Near;
	dword nFrustumPlaneFar = (dword)EFrustumPlane::EFrustumPlane_Far;

	// left
	m_FrustumPlane[nFrustumPlaneLeft].normal.x = viewproj.m11 + viewproj.m14;
	m_FrustumPlane[nFrustumPlaneLeft].normal.y = viewproj.m21 + viewproj.m24;
	m_FrustumPlane[nFrustumPlaneLeft].normal.z = viewproj.m31 + viewproj.m34;
	m_FrustumPlane[nFrustumPlaneLeft].dis = viewproj.m41 + viewproj.m44;
	m_FrustumPlane[nFrustumPlaneLeft].Normalize();

	// right
	m_FrustumPlane[nFrustumPlaneRight].normal.x = viewproj.m14 - viewproj.m11;
	m_FrustumPlane[nFrustumPlaneRight].normal.y = viewproj.m24 - viewproj.m21;
	m_FrustumPlane[nFrustumPlaneRight].normal.z = viewproj.m34 - viewproj.m31;
	m_FrustumPlane[nFrustumPlaneRight].dis = viewproj.m44 - viewproj.m41;
	m_FrustumPlane[nFrustumPlaneRight].Normalize();

	// top
	m_FrustumPlane[nFrustumPlaneTop].normal.x = viewproj.m14 - viewproj.m12;
	m_FrustumPlane[nFrustumPlaneTop].normal.y = viewproj.m24 - viewproj.m22;
	m_FrustumPlane[nFrustumPlaneTop].normal.z = viewproj.m34 - viewproj.m32;
	m_FrustumPlane[nFrustumPlaneTop].dis = viewproj.m44 - viewproj.m42;
	m_FrustumPlane[nFrustumPlaneTop].Normalize();

	// bottom
	m_FrustumPlane[nFrustumPlaneBottom].normal.x = viewproj.m12 + viewproj.m14;
	m_FrustumPlane[nFrustumPlaneBottom].normal.y = viewproj.m22 + viewproj.m24;
	m_FrustumPlane[nFrustumPlaneBottom].normal.z = viewproj.m32 + viewproj.m34;
	m_FrustumPlane[nFrustumPlaneBottom].dis = viewproj.m42 + viewproj.m44;
	m_FrustumPlane[nFrustumPlaneBottom].Normalize();

	// near
	m_FrustumPlane[nFrustumPlaneNear].normal.x = viewproj.m13;
	m_FrustumPlane[nFrustumPlaneNear].normal.y = viewproj.m23;
	m_FrustumPlane[nFrustumPlaneNear].normal.z = viewproj.m33;
	m_FrustumPlane[nFrustumPlaneNear].dis = viewproj.m43;
	m_FrustumPlane[nFrustumPlaneNear].Normalize();

	// far
	m_FrustumPlane[nFrustumPlaneFar].normal.x = viewproj.m14 - viewproj.m13;
	m_FrustumPlane[nFrustumPlaneFar].normal.y = viewproj.m24 - viewproj.m23;
	m_FrustumPlane[nFrustumPlaneFar].normal.z = viewproj.m34 - viewproj.m33;
	m_FrustumPlane[nFrustumPlaneFar].dis = viewproj.m44 - viewproj.m43;
	m_FrustumPlane[nFrustumPlaneFar].Normalize();

	BIT_REMOVE(m_dirty, CAMERA_FRUSTUM_PLANE_DIRTY);
}

const Plane& CCamera::GetFrustumPlane(EFrustumPlane ePlane) const
{
	if (BIT_CHECK(m_dirty, CAMERA_FRUSTUM_PLANE_DIRTY))
		SetupFrustumPlanes();
	return m_FrustumPlane[(dword)ePlane];
}

const Plane* CCamera::GetFrustumPlanes() const
{
	if (BIT_CHECK(m_dirty, CAMERA_FRUSTUM_PLANE_DIRTY))
		SetupFrustumPlanes();
	return &m_FrustumPlane[(dword)EFrustumPlane::EFrustumPlane_Left];
}