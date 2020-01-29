#include "Camera.hpp"

using namespace DirectX::SimpleMath;

Camera::Camera() {
	m_projMatrix =
		Matrix::CreatePerspectiveFieldOfView(3.14159265f / 4.0f, 800.0f / 600.0f, 0.1f, 100.0f);
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

void Camera::buildMatrices() {
	if (m_viewChanged) {
		m_viewMatrix = Matrix::CreateLookAt(m_camEye, m_camTarget, m_camUp);
		m_vpMatrix = m_viewMatrix * m_projMatrix;
		m_viewChanged = false;
	}
}

void Camera::createBuffer() {
	auto device = Renderer::getInstance()->getDevice();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_vpMatrix);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_vpMatrix;
	device->CreateBuffer(&bufferDesc, &data, m_matrixBuffer.GetAddressOf());
}

void Camera::updateBuffer() {
	auto deviceContext = Renderer::getInstance()->getDeviceContext();
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &m_vpMatrix, 0, 0);
}
