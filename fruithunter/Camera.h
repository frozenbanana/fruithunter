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

	void updateBuffer();
	void bindMatrix();
	DirectX::SimpleMath::Matrix getViewProjMatrix() const;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;

	bool m_viewChanged = false;
	float4x4 m_vpMatrix;
	float4x4 m_viewMatrix;
	float4x4 m_projMatrix;

	float3 m_camEye;
	float3 m_camTarget;
	float3 m_camUp;
};