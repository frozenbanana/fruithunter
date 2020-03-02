#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_SLOT 1
#define MATRIX_VP_SLOT 5

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

	float m_fov;
	float3 m_camEye;
	float3 m_camTarget;
	float3 m_camUp;



	void updateBuffer();
};