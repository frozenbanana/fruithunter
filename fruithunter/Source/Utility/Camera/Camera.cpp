#include "Camera.h"
#include "Renderer.h"
#include "ErrorLogger.h"

Camera::Camera() {
	// Set initial values
	m_camEye = float3(0.0, 0.0, -4.0);
	m_camTarget = float3(0.0, 0.0, 0.0);
	m_camUp = float3(0.0, 1.0, 0.0);

	m_fov = DEFAULT_FOV;
	m_projMatrix = XMMatrixPerspectiveFovLH(
		m_fov, (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, m_nearPlane, m_farPlane);
	m_viewMatrix = XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
	m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);
	{
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
	{
		// Create constant buffer for struct
		auto device = Renderer::getInstance()->getDevice();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(ViewPerspectiveBuffer);
		HRESULT res = device->CreateBuffer(
			&bufferDesc, nullptr, m_matrixBufferViewNPerspective.GetAddressOf());

		if (FAILED(res)) {
			ErrorLogger::messageBox(res, "Camera failed to create buffer struct.");
		}
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

void Camera::setNearPlane(float nearPlane) {
	m_nearPlane = nearPlane;
	m_projChanged = true;
}

void Camera::setFarPlane(float farPlane) {
	m_farPlane = farPlane;
	m_projChanged = true;
}

float Camera::getDefaultFov() const { return DEFAULT_FOV; }

void Camera::updateBuffer() {
	if (m_viewChanged || m_projChanged) {

		m_viewMatrix = XMMatrixLookAtLH(m_camEye, m_camTarget, m_camUp);
		/*ErrorLogger::logFloat3("CameraPos: ", m_camEye);
		ErrorLogger::logFloat3("CameraTarget: ", m_camTarget);
		ErrorLogger::logFloat3("CameraUp: ", m_camUp);*/

		m_projMatrix = XMMatrixPerspectiveFovLH(
			m_fov, (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, m_nearPlane, m_farPlane);

		m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

		auto deviceContext = Renderer::getDeviceContext();
		Matrix vpMatrixTransposed = m_vpMatrix.Transpose();
		deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &vpMatrixTransposed, 0, 0);

		// Changes to make particle system work
		ViewPerspectiveBuffer viewNPerspective;
		viewNPerspective.mView = m_viewMatrix.Transpose();
		viewNPerspective.mPerspective = m_projMatrix.Transpose();
		deviceContext->UpdateSubresource(
			m_matrixBufferViewNPerspective.Get(), 0, NULL, &viewNPerspective, 0, 0);

		m_projChanged = false;
		m_viewChanged = false;
	}
}

void Camera::bindMatrix() {
	updateBuffer();
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
	deviceContext->VSSetConstantBuffers(
		MATRIX_VP_SLOT, 1, m_matrixBufferViewNPerspective.GetAddressOf());

	deviceContext->GSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
	deviceContext->GSSetConstantBuffers(
		MATRIX_VP_SLOT, 1, m_matrixBufferViewNPerspective.GetAddressOf());
}

float4x4 Camera::getViewMatrix() const { return m_viewMatrix; }

float4x4 Camera::getViewProjMatrix() const { return m_vpMatrix; }

float3 Camera::getPosition() const { return m_camEye; }

vector<FrustumPlane> Camera::getFrustumPlanes() const {
	vector<FrustumPlane> planes;
	planes.reserve(6);
	float3 center = m_camEye;
	float height = tan(m_fov / 2.f);
	float aspectRatio = (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT;
	float width = height * aspectRatio;

	float3 camForward = m_camTarget - m_camEye;
	camForward.Normalize();
	float3 camLeft = m_camUp.Cross(camForward);
	camLeft.Normalize();
	float3 camUp = camForward.Cross(camLeft);
	camUp.Normalize();

	float depth = 1.f; // NEAR_PLANE
	float3 topLeft = center + (camForward + camLeft * width * 1.f + camUp * height * 1.f) * depth;
	float3 topRight = center + (camForward + camLeft * width * 1.f + camUp * height * -1.f) * depth;
	float3 bottomLeft =
		center + (camForward + camLeft * width * -1.f + camUp * height * 1.f) * depth;
	float3 bottomRight =
		center + (camForward + camLeft * width * -1.f + camUp * height * -1.f) * depth;

	planes.push_back(FrustumPlane(center, (bottomLeft - center).Cross(topLeft - center)));
	planes.push_back(FrustumPlane(bottomRight, (topRight - center).Cross(bottomRight - center)));
	planes.push_back(FrustumPlane(topRight, (topLeft - center).Cross(topRight - center)));
	planes.push_back(FrustumPlane(bottomLeft, (bottomRight - center).Cross(bottomLeft - center)));
	planes.push_back(FrustumPlane(center + camForward * m_nearPlane, -camForward));
	planes.push_back(FrustumPlane(center + camForward * m_farPlane, camForward));

	return planes;
}

CubeBoundingBox Camera::getFrustumBoundingBox() const {
	return CubeBoundingBox(getFrustumPoints());
}

vector<float3> Camera::getFrustumPoints(float scaleBetweenNearAndFarPlane) const {
	float3 center = m_camEye;
	float height = tan(m_fov / 2.f);
	float aspectRatio = (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT;
	float width = height * aspectRatio;

	float3 camForward = m_camTarget - m_camEye;
	camForward.Normalize();
	float3 camLeft = m_camUp.Cross(camForward);
	camLeft.Normalize();
	float3 camUp = camForward.Cross(camLeft);
	camUp.Normalize();

	float depth =
		m_nearPlane * (1 - scaleBetweenNearAndFarPlane) + m_farPlane * scaleBetweenNearAndFarPlane;
	float3 topLeft = center + (camForward + camLeft * width * 1.f + camUp * height * 1.f) * depth;
	float3 topRight = center + (camForward + camLeft * width * -1.f + camUp * height * 1.f) * depth;
	float3 bottomLeft =
		center + (camForward + camLeft * width * 1.f + camUp * height * -1.f) * depth;
	float3 bottomRight =
		center + (camForward + camLeft * width * -1.f + camUp * height * -1.f) * depth;

	vector<float3> points;
	points.reserve(5);
	points.push_back(center);
	points.push_back(topLeft);
	points.push_back(topRight);
	points.push_back(bottomLeft);
	points.push_back(bottomRight);

	return points;
}
