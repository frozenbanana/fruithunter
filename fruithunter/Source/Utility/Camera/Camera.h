#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_SLOT 1
#define CBUFFER_POS_SLOT 9

class Camera {
public:
	Camera();
	~Camera();

	void setView(float3 camEye, float3 camTarget, float3 camUp);
	void setEye(float3 camEye);
	void setTarget(float3 camTarget);
	void setUp(float3 camUp);
	void setFov(float fov);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);

	void move(float3 movement);

	float3 getPosition() const;
	float4x4 getViewMatrix();
	float4x4 getViewProjMatrix();
	float getDefaultFov() const;
	float3 getForward() const;
	float3 getUp() const;
	float3 getRight() const;

	vector<FrustumPlane> getFrustumPlanes() const;
	CubeBoundingBox getFrustumBoundingBox() const;
	vector<float3> getFrustumPoints(float scaleBetweenNearAndFarPlane = 1.f) const;

	float3 getMousePickVector(float2 mousePos) const;

	void bind();

protected:
	const float DEFAULT_FOV = XM_PI / 2.5f;
	struct ViewPerspectiveBuffer {
		float4x4 mView;
		float4x4 mPerspective;
		float4x4 mViewPerspective;
	};

	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbufferPosition;

	bool m_propertiesChanged = false;
	float4x4 m_vpMatrix;
	float4x4 m_viewMatrix;
	float4x4 m_projMatrix;

	float m_fov = DEFAULT_FOV;
	float3 m_positionEye = float3(0, 0, 0);
	float3 m_positionTarget = float3(0, 0, -1.f);
	float3 m_up = float3(0, 1.f, 0);

	float m_nearPlane = 0.1f;
	float m_farPlane = 150.f;

	void updateResources();
	void updateMatrices();
};