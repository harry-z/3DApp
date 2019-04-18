#pragma once

#include "Prereq.h"

#define CAMERA_VIEW_DIRTY										0x01
#define CAMERA_PROJ_DIRTY										0x02
#define CAMERA_FRUSTUM_PLANE_DIRTY			0x04
#define CAMERA_FRUSTUM_SIZE_DIRTY				0x08

enum class EProjectType : char {
	EProject_Perspective,
	EProject_Ortho
};

enum class EFrustumPlane {
	EFrustumPlane_Left,
	EFrustumPlane_Right,
	EFrustumPlane_Top,
	EFrustumPlane_Bottom,
	EFrustumPlane_Near,
	EFrustumPlane_Far,
	EFrustumPlane_Num
};

// 左手坐标系视图投影矩阵，深度范围0 ~ 1
class DLL_EXPORT CCamera {
public:
	friend class C3DEngine;

	inline void SetEye(const Vec3 &eye) {
		m_eye = eye;
		BIT_ADD(m_dirty, (CAMERA_VIEW_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY));
	}
	inline const Vec3& GetEye() const { return m_eye; }
	inline void SetLookat(const Vec3 &lookat) {
		m_lookat = lookat;
		BIT_ADD(m_dirty, (CAMERA_VIEW_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY));
	}
	inline const Vec3& GetLookat() const { return m_lookat; }
	inline void SetUp(const Vec3 &up) {
		m_up = up;
		BIT_ADD(m_dirty, (CAMERA_VIEW_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY));
	}
	inline const Vec3& GetUp() const { return m_up; }
	void SetupViewMatrix() const;
	const Matrix4& GetViewMatrix() const;

	inline void SetPerspectiveData(float fov, float aspect) {
		m_ProjectionData.perspective.m_fov = fov;
		m_ProjectionData.perspective.m_aspect = aspect;
		BIT_ADD(m_dirty, (CAMERA_PROJ_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY | CAMERA_FRUSTUM_SIZE_DIRTY));
		m_type = EProjectType::EProject_Perspective;
	}
	inline float GetFov() const { return m_ProjectionData.perspective.m_fov; }
	inline float GetAspect() const { return m_ProjectionData.perspective.m_aspect; }
	inline void SetOrthoData(float left, float right, float top, float bottom) {
		m_ProjectionData.ortho.m_left = left;
		m_ProjectionData.ortho.m_right = right;
		m_ProjectionData.ortho.m_top = top;
		m_ProjectionData.ortho.m_bottom = bottom;
		BIT_ADD(m_dirty, (CAMERA_PROJ_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY | CAMERA_FRUSTUM_SIZE_DIRTY));
		m_type = EProjectType::EProject_Ortho;
	}
	inline void GetOrtho(float &left, float &right, float &top, float &bottom) {
		left = m_ProjectionData.ortho.m_left;
		right = m_ProjectionData.ortho.m_right;
		top = m_ProjectionData.ortho.m_top;
		bottom = m_ProjectionData.ortho.m_bottom;
	}
	inline void SetNearClip(float nearClip) {
		m_nearclip = nearClip;
		BIT_ADD(m_dirty, (CAMERA_PROJ_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY));
	}
	inline float GetNearClip() const { return m_nearclip; }
	inline void SetFarClip(float farClip) {
		m_farclip = farClip;
		BIT_ADD(m_dirty, (CAMERA_PROJ_DIRTY | CAMERA_FRUSTUM_PLANE_DIRTY));
	}
	inline float GetFarClip() const { return m_farclip; }
	void SetupProjMatrix() const;
	const Matrix4& GetProjMatrix() const;

	inline EProjectType GetProjType() const { return m_type; }

	void SetupFrustumPlanes() const;
	const Plane& GetFrustumPlane(EFrustumPlane ePlane) const;
	const Plane* GetFrustumPlanes() const;

	inline void SetCustomClipPlane(const Plane &plane) {
		m_CustomClipPlane = plane;
		m_bCustomClipPlane = true;
	}
	inline void ClearCustomClipPlane() {
		m_bCustomClipPlane = false;
	}
	inline bool HasCustomClipPlane() const { return m_bCustomClipPlane; }
	inline const Plane& GetCustomClipPlane() const { return m_CustomClipPlane; }

private:
	CCamera();
	~CCamera() {}

private:
	Vec3 m_eye;
	Vec3 m_lookat;
	Vec3 m_up;
	mutable Matrix4 m_view;

	float m_nearclip, m_farclip;
	union {
		struct {
			float m_fov;
			float m_aspect;
			float m_reserved[2];
		} perspective;
		struct {
			float m_left, m_right, m_top, m_bottom;
		} ortho;
	} m_ProjectionData;
	mutable Matrix4 m_projection;

	mutable Plane m_FrustumPlane[6];
	Plane m_CustomClipPlane;
	bool m_bCustomClipPlane;

	mutable char m_dirty;
	EProjectType m_type;

	
};