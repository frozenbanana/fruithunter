#include "Camera.h"
#include "DirectXMath.h"

using namespace DirectX::SimpleMath;

Camera::Camera() {
	// Set initial values
	m_camEye = Vector3(0.0, 0.0, -4.0);
	m_camTarget = Vector3(0.0, 0.0, 0.0);
	m_camUp = Vector3(0.0, 1.0, 0.0);

	m_projMatrix =
		DirectX::XMMatrixPerspectiveFovLH(3.14159265f / 4.0f, 800.0f / 600.0f, 0.1f, 100.0f);
	m_viewMatrix = DirectX::XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
	m_vpMatrix = DirectX::XMMatrixMultiply(m_viewMatrix, m_projMatrix);

	// Create constant buffer
	auto device = Renderer::getInstance()->getDevice();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_vpMatrix);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_vpMatrix;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_matrixBuffer.GetAddressOf());

	if (FAILED(res)) {
		ErrorLogger::messageBox(res, "Camera failed to create buffer.");
	}
}

Camera::Camera(Vector3 camEye, Vector3 camTarget, Vector3 camUp) {
	// Set initial values
	m_camEye = camEye;
	m_camTarget = camTarget;
	m_camUp = camUp;

	m_projMatrix =
		DirectX::XMMatrixPerspectiveFovLH(3.14159265f / 4.0f, 800.0f / 600.0f, 0.1f, 100.0f);
	m_viewMatrix = DirectX::XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
	m_vpMatrix = DirectX::XMMatrixMultiply(m_viewMatrix, m_projMatrix);

	// Create constant buffer
	auto device = Renderer::getInstance()->getDevice();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_vpMatrix);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_vpMatrix;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_matrixBuffer.GetAddressOf());

	if (FAILED(res)) {
		ErrorLogger::messageBox(res, "Camera failed to create buffer.");
	}
}

Camera::~Camera() {}

void Camera::setEye(Vector3 camEye) {
	m_camEye = camEye;
	m_viewChanged = true;
}

void Camera::setTarget(Vector3 camTarget) {
	m_camTarget = camTarget;
	m_viewChanged = true;
}

void Camera::setUp(Vector3 camUp) {
	m_camUp = camUp;
	m_viewChanged = true;
}

void Camera::setView(Vector3 camEye, Vector3 camTarget, Vector3 camUp) {
	m_camEye = camEye;
	m_camTarget = camTarget;
	m_camUp = camUp;
	m_viewChanged = true;
}

void Camera::updateBuffer() {
	if (m_viewChanged) {
		m_viewMatrix = DirectX::XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
		m_vpMatrix = DirectX::XMMatrixMultiply(m_viewMatrix, m_projMatrix);
		m_viewChanged = false;
	}

	auto deviceContext = Renderer::getDeviceContext();
	Matrix vpMatrixTransposed = m_vpMatrix.Transpose();
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &vpMatrixTransposed, 0, 0);
}

void Camera::bindMatrix() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
}

DirectX::SimpleMath::Matrix Camera::getViewProjMatrix() const { return m_vpMatrix; }
