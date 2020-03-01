#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_SLOT 1

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.f

class Camera {
public:
	Camera();
	Camera(float3 camEye, float3 camTarget, float3 camUp);
	~Camera();

	void setEye(float3 camEye);
	void setTarget(float3 camTarget);
	void setUp(float3 camUp);
	void setView(float3 camEye, float3 camTarget, float3 camUp);

	void setFov(float fov);
	float getDefaultFov() const;

	void bindMatrix();
	float4x4 getViewMatrix() const;
	float4x4 getViewProjMatrix() const;
	float3 getPosition() const;

	vector<FrustumPlane> getFrustumPlanes() const;

private:
	const float DEFAULT_FOV = XM_PI / 2.5f;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	bool m_viewChanged = false;
	bool m_projChanged = false;
	float4x4 m_vpMatrix;
	float4x4 m_viewMatrix;
	float4x4 m_projMatrix;

	float m_fov;
	float3 m_camEye;
	float3 m_camTarget;
	float3 m_camUp;



	void updateBuffer();
};