#include "Camera.h"
#include "Renderer.h"
#include "ErrorLogger.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.f

Camera::Camera() {
	// Set initial values
	m_camEye = float3(0.0, 0.0, -4.0);
	m_camTarget = float3(0.0, 0.0, 0.0);
	m_camUp = float3(0.0, 1.0, 0.0);

	m_fov = DEFAULT_FOV;
	m_projMatrix = XMMatrixPerspectiveFovLH(
		m_fov, (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, NEAR_PLANE, FAR_PLANE);
	m_viewMatrix = XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
	m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

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

Camera::Camera(float3 camEye, float3 camTarget, float3 camUp) {
	// Set initial values
	m_camEye = camEye;
	m_camTarget = camTarget;
	m_camUp = camUp;

	m_fov = DEFAULT_FOV;
	m_projMatrix = XMMatrixPerspectiveFovLH(
		m_fov, (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, NEAR_PLANE, FAR_PLANE);
	m_viewMatrix = XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
	m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

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

void Camera::setEye(float3 camEye) {
	m_camEye = camEye;
	m_viewChanged = true;
}

void Camera::setTarget(float3 camTarget) {
	m_camTarget = camTarget;
	m_viewChanged = true;
}

void Camera::setUp(float3 camUp) {
	m_camUp = camUp;
	m_viewChanged = true;
}

void Camera::setView(float3 camEye, float3 camTarget, float3 camUp) {
	m_camEye = camEye;
	m_camTarget = camTarget;
	m_camUp = camUp;
	m_viewChanged = true;
}

void Camera::setFov(float fov) {
	m_fov = fov;
	m_projChanged = true;
}

float Camera::getDefaultFov() const { return DEFAULT_FOV; }

void Camera::updateBuffer() {
	if (m_viewChanged || m_projChanged) {

		m_viewMatrix = XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);

		m_projMatrix = XMMatrixPerspectiveFovLH(
			m_fov, (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, NEAR_PLANE, FAR_PLANE);

		m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

		auto deviceContext = Renderer::getDeviceContext();
		Matrix vpMatrixTransposed = m_vpMatrix.Transpose();
		deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &vpMatrixTransposed, 0, 0);
		m_projChanged = false;
		m_viewChanged = false;
	}
}

void Camera::bindMatrix() {
	updateBuffer();
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
}

float4x4 Camera::getViewMatrix() const { return m_viewMatrix; }

float4x4 Camera::getViewProjMatrix() const { return m_vpMatrix; }

float3 Camera::getPosition() const { return m_camEye; }
