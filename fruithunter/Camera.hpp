#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "SimpleMath.h"
#include "ErrorLogger.hpp"
#include "Renderer.hpp"

class Camera {
public:
	Camera();
	~Camera();

	void setEye(DirectX::SimpleMath::Vector3 camEye);
	void setTarget(DirectX::SimpleMath::Vector3 camTarget);
	void setUp(DirectX::SimpleMath::Vector3 camUp);
	void setView(DirectX::SimpleMath::Vector3 camEye, DirectX::SimpleMath::Vector3 camTarget,
		DirectX::SimpleMath::Vector3 camUp);

	void buildMatrices();

	void createBuffer();
	void updateBuffer();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	ErrorLogger m_errors;
	ID3D11Buffer* m_matrixBuffer = nullptr;

	bool m_viewChanged = false;

	DirectX::SimpleMath::Matrix m_viewMatrix;
	DirectX::SimpleMath::Matrix m_projMatrix;
	DirectX::SimpleMath::Matrix m_vpMatrix;

	DirectX::SimpleMath::Vector3 m_camEye;
	DirectX::SimpleMath::Vector3 m_camTarget;
	DirectX::SimpleMath::Vector3 m_camUp;
};