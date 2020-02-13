#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_SLOT 1

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

	void updateBuffer();
	void bindMatrix();
	float4x4 getViewProjMatrix() const;
	float3 getPosition() const;

private:
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
};