#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_SLOT 1
#define MATRIX_VP_SLOT 5

class Camera {
public:
	Camera();
	~Camera();

	void move(float3 movement);
	void setEye(float3 camEye);
	void setTarget(float3 camTarget);
	void setUp(float3 camUp);
	void setView(float3 camEye, float3 camTarget, float3 camUp);
	void setFov(float fov);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);

	float3 getPosition() const;
	float4x4 getViewMatrix() const;
	float4x4 getViewProjMatrix() const;
	float getDefaultFov() const;

	vector<FrustumPlane> getFrustumPlanes() const;
	CubeBoundingBox getFrustumBoundingBox() const;
	vector<float3> getFrustumPoints(float scaleBetweenNearAndFarPlane = 1.f) const;

	void bindMatrix();

private:
	const float DEFAULT_FOV = XM_PI / 2.5f;
	struct ViewPerspectiveBuffer {
		float4x4 mView;
		float4x4 mPerspective;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBufferViewNPerspective;


	bool m_viewChanged = false;
	bool m_projChanged = false;
	float4x4 m_vpMatrix;
	float4x4 m_viewMatrix;
	float4x4 m_projMatrix;

	float m_fov = DEFAULT_FOV;
	float3 m_positionEye = float3(0, 0, 0);
	float3 m_positionTarget = float3(0, 0, -1.f);
	float3 m_up = float3(0, 1.f, 0);

	float m_nearPlane = 0.1f;
	float m_farPlane = 150.f;

	void updateBuffer();
};