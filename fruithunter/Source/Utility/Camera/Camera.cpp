#include "Camera.h"
#include "Renderer.h"
#include "ErrorLogger.h"

Camera::Camera() {
	// Set initial values
	m_positionEye = float3(0.0, 0.0, -4.0);
	m_positionTarget = float3(0.0, 0.0, 0.0);
	m_up = float3(0.0, 1.0, 0.0);

	m_fov = DEFAULT_FOV;
	m_projMatrix = XMMatrixPerspectiveFovLH(
		m_fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, m_nearPlane, m_farPlane);
	m_viewMatrix = XMMatrixLookAtLH(m_positionEye, m_positionTarget, m_up);
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
	m_positionEye = camEye;
	m_propertiesChanged = true;
}

void Camera::move(float3 movement) { setEye(m_positionEye + movement); }

void Camera::setTarget(float3 camTarget) {
	m_positionTarget = camTarget;
	m_propertiesChanged = true;
}

void Camera::setUp(float3 camUp) {
	m_up = camUp;
	m_propertiesChanged = true;
}

void Camera::setView(float3 camEye, float3 camTarget, float3 camUp) {
	m_positionEye = camEye;
	m_positionTarget = camTarget;
	m_up = camUp;
	m_propertiesChanged = true;
}

void Camera::setFov(float fov) {
	m_fov = fov;
	m_propertiesChanged = true;
}

void Camera::setNearPlane(float nearPlane) {
	m_nearPlane = nearPlane;
	m_propertiesChanged = true;
}

void Camera::setFarPlane(float farPlane) {
	m_farPlane = farPlane;
	m_propertiesChanged = true;
}

float Camera::getDefaultFov() const { return DEFAULT_FOV; }

void Camera::updateResources() {
	auto deviceContext = Renderer::getDeviceContext();
	Matrix vpMatrixTransposed = m_vpMatrix.Transpose();
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &vpMatrixTransposed, 0, 0);

	// Changes to make particle system work
	ViewPerspectiveBuffer viewNPerspective;
	viewNPerspective.mView = m_viewMatrix.Transpose();
	viewNPerspective.mPerspective = m_projMatrix.Transpose();
	deviceContext->UpdateSubresource(
		m_matrixBufferViewNPerspective.Get(), 0, NULL, &viewNPerspective, 0, 0);
}

void Camera::updateMatrices() {
	m_propertiesChanged = false;
	m_viewMatrix = XMMatrixLookAtLH(m_positionEye, m_positionTarget, m_up);
	m_projMatrix = XMMatrixPerspectiveFovLH(
		m_fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, m_nearPlane, m_farPlane);

	m_vpMatrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);
}

void Camera::bind() {
	if (m_propertiesChanged)
		updateMatrices();
	//update resource
	updateResources();
	//bind
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
	deviceContext->VSSetConstantBuffers(
		MATRIX_VP_SLOT, 1, m_matrixBufferViewNPerspective.GetAddressOf());

	deviceContext->GSSetConstantBuffers(MATRIX_SLOT, 1, m_matrixBuffer.GetAddressOf());
	deviceContext->GSSetConstantBuffers(
		MATRIX_VP_SLOT, 1, m_matrixBufferViewNPerspective.GetAddressOf());
}

float4x4 Camera::getViewMatrix() { 
	if (m_propertiesChanged)
		updateMatrices();
	return m_viewMatrix; 
}

float4x4 Camera::getViewProjMatrix() {
	if (m_propertiesChanged)
		updateMatrices();
	return m_vpMatrix;
}

float3 Camera::getPosition() const { return m_positionEye; }

vector<FrustumPlane> Camera::getFrustumPlanes() const {
	vector<FrustumPlane> planes;
	planes.reserve(6);
	float3 center = m_positionEye;
	float height = tan(m_fov / 2.f);
	float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float width = height * aspectRatio;

	float3 camForward = m_positionTarget - m_positionEye;
	camForward.Normalize();
	float3 camLeft = m_up.Cross(camForward);
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
	float3 center = m_positionEye;
	float height = tan(m_fov / 2.f);
	float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float width = height * aspectRatio;

	float3 camForward = m_positionTarget - m_positionEye;
	camForward.Normalize();
	float3 camLeft = m_up.Cross(camForward);
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

float3 Camera::getMousePickVector(float2 mousePos) const { 
	vector<float3> points = getFrustumPoints(1);
	float3 topLeft = points[1] - points[0];
	float3 topRight = points[2] - points[0];
	float3 bottomLeft = points[3] - points[0];
	float3 bottomRight = points[4] - points[0];
	float3 topLerp = lerp(topLeft, topRight, mousePos.x);
	float3 bottomLerp = lerp(bottomLeft, bottomRight, mousePos.x);
	float3 middleLerp = lerp(topLerp, bottomLerp, mousePos.y);
	middleLerp.Normalize();
	return middleLerp;
}

float3 Camera::getForward() const { 
	float3 forward = m_positionTarget - m_positionEye;
	forward.Normalize();
	return forward; 
}

float3 Camera::getUp() const { return m_up; }

float3 Camera::getRight() const {
	return getUp().Cross(getForward());
	//getForward().Cross(getUp());
}
