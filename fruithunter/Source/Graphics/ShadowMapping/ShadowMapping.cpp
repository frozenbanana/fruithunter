#include "ShadowMapping.h"
#include "Renderer.h"
#include "ErrorLogger.h"

Matrix g_textureMatrix = { 0.5f, 0.f, 0.f, 0.f, 0.f, -0.5f, 0.f, 0.f, 0.f, 0.f, 1.0f, 0.f, 0.5f,
	0.5, 0.f, 1.0f };

void ShadowMapper::createCameraBuffer() {
	auto device = Renderer::getDevice();
	// Create constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_vpMatrix_t);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_vpMatrix_t;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_matrixBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logWarning(res, "(ShadowMapping) Failed creating CameraVPT constant buffer!");

	/*m_viewMatrix = XMMatrixLookAtLH(
		float3(21.43f, 36.567f, 183.6f), float3(22.06f, 36.2f, 182.9f), float3(0.f, 1.f, 0.f));
	m_projMatrix = XMMatrixOrthographicLH(283.f, 283.f, 100.f, 500.f);*/
	m_viewMatrix = XMMatrixLookAtLH(
		float3(-0.f, 110.f, 100.f), float3(100.0f, 0.0f, 100.f), float3(0.f, 1.f, 0.f));
	m_projMatrix = XMMatrixOrthographicLH(200.f, 180.f, 1.f, 500.f);
	/*m_projMatrix = XMMatrixPerspectiveFovLH(
		(XM_PI / 2.f), (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, NEAR_PLANE, FAR_PLANE);*/
	Matrix vp_matrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

	m_vpMatrix_t = vp_matrix.Transpose();

	// auto deviceContext = Renderer::getDeviceContext();
	// deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &m_vpMatrix_t, 0, 0);
}

void ShadowMapper::createVPTBuffer() {
	auto device = Renderer::getDevice();
	// Create constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_VPT);

	HRESULT res = device->CreateBuffer(&bufferDesc, nullptr, m_matrixVPTBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logWarning(res, "(ShadowMapping) Failed creating VPTMatrix constant buffer!");
}

void ShadowMapper::createInfoBuffer() {

	createShadowInfo();

	auto device = Renderer::getDevice();
	// Create constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_shadowInfo);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_shadowInfo;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_ShadowInfoBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logWarning(res, "(ShadowMapping) Failed creating ShadowInfo constant buffer!");
}

void ShadowMapper::updateViewMatrix() {
	m_viewMatrix =
		XMMatrixLookAtLH(m_position, m_position + m_lightDirection, float3(0.f, 1.f, 0.f));
}

void ShadowMapper::updateProjMatrix() {
	m_projMatrix = XMMatrixOrthographicLH(m_size.x, m_size.y, m_nearPlane, m_farPlane);
}

void ShadowMapper::bindVPTBuffer() {
	auto deviceContext = Renderer::getDeviceContext();

	// update
	m_VPT = m_viewMatrix * m_projMatrix * g_textureMatrix;
	Matrix mVPT = m_VPT.Transpose();

	// fill
	deviceContext->UpdateSubresource(m_matrixVPTBuffer.Get(), 0, 0, &mVPT, 0, 0);

	// bind
	deviceContext->VSSetConstantBuffers(MATRIX_VPT_SLOT, 1, m_matrixVPTBuffer.GetAddressOf());
}

void ShadowMapper::bindCameraBuffer() {
	auto deviceContext = Renderer::getDeviceContext();

	// update
	Matrix vp_matrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);
	m_vpMatrix_t = vp_matrix.Transpose();

	// fill
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, 0, &m_vpMatrix_t, 0, 0);

	// bind
	deviceContext->VSSetConstantBuffers(MATRIX_CAMERA_SLOT, 1, m_matrixBuffer.GetAddressOf());
}

void ShadowMapper::bindShadowInfoBuffer() {

	auto deviceContext = Renderer::getDeviceContext();

	// fill
	deviceContext->UpdateSubresource(m_ShadowInfoBuffer.Get(), 0, NULL, &m_shadowInfo, 0, 0);

	// bind
	deviceContext->PSSetConstantBuffers(
		MATRIX_SHADOWINFO_SLOT, 1, m_ShadowInfoBuffer.GetAddressOf());
}

ShadowMapper::ShadowMapper() {}

ShadowMapper::~ShadowMapper() { delete m_nullRenderTargets[0]; }

void ShadowMapper::setDirection(float3 direction) {
	m_lightDirection = direction;
	m_lightDirection.Normalize();
	m_shadowInfo.toLight = -float4(m_lightDirection.x, m_lightDirection.y, m_lightDirection.z,
		0.0f); // the toLight direction is the opposite of the direction of the light
}

void ShadowMapper::mapShadowToFrustum(vector<float3> frustum) {
	// view matrix
	float3 lookAt = frustum[0];
	float3 offset = -m_lightDirection * 100;
	m_position = lookAt + offset;
	updateViewMatrix();

	// proj matrix
	for (size_t i = 0; i < frustum.size(); i++)
		frustum[i] = float3::Transform(frustum[i], m_viewMatrix);
	CubeBoundingBox lightToFrustumBB(frustum);

	m_position = float3::Transform(lightToFrustumBB.getCenter(), m_viewMatrix.Invert()) + offset;
	m_size = float2(lightToFrustumBB.m_size.x, lightToFrustumBB.m_size.y);
	updateViewMatrix();
	updateProjMatrix();
}

void ShadowMapper::initiate() {
	auto device = Renderer::getDevice();

	m_shadowPort.TopLeftX = 0.0f;
	m_shadowPort.TopLeftY = 0.0f;
	m_shadowPort.Width = (float)m_smapSize.x;
	m_shadowPort.Height = (float)m_smapSize.y;
	m_shadowPort.MinDepth = 0.0f;
	m_shadowPort.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC shadowTexDesc;
	shadowTexDesc.Width = m_smapSize.x;
	shadowTexDesc.Height = m_smapSize.y;
	shadowTexDesc.MipLevels = 1;
	shadowTexDesc.ArraySize = 1;
	shadowTexDesc.Format = DXGI_FORMAT_R32_TYPELESS; // DXGI_FORMAT_R24G8_TYPELESS;
	shadowTexDesc.SampleDesc.Count = 1;
	shadowTexDesc.SampleDesc.Quality = 0;
	shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowTexDesc.CPUAccessFlags = 0;
	shadowTexDesc.MiscFlags = 0;


	HRESULT hr_0 = device->CreateTexture2D(&shadowTexDesc, 0, m_depthMap.GetAddressOf());
	HRESULT hr_01 = device->CreateTexture2D(&shadowTexDesc, 0, m_depthMapStatic.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc;
	shadowDSVDesc.Flags = 0;
	shadowDSVDesc.Format = DXGI_FORMAT_D32_FLOAT; // DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSVDesc.Texture2D.MipSlice = 0;

	HRESULT hr_1 = device->CreateDepthStencilView(m_depthMap.Get(), &shadowDSVDesc, &m_shadowDSV);
	HRESULT hr_11 =
		device->CreateDepthStencilView(m_depthMapStatic.Get(), &shadowDSVDesc, &m_staticShadowDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT; // DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = shadowTexDesc.MipLevels;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr_2 = device->CreateShaderResourceView(m_depthMap.Get(), &shadowSRVDesc, &m_shadowSRV);
	HRESULT hr_21 = device->CreateShaderResourceView(
		m_depthMapStatic.Get(), &shadowSRVDesc, &m_staticShadowSRV);

	createCameraBuffer();
	createVPTBuffer();
	createInfoBuffer();

	setDirection(float3(100.f, -100.f, 0));
}

void ShadowMapper::bindShadowMap() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->PSSetShaderResources(4, 1, m_shadowSRV.GetAddressOf());
}

void ShadowMapper::update(float3 playerPos) {
	// Moves the shadowmap camera to above the player with an offset.
	float3 offSet = { -1.f, 100.f, 0 }; // Essentially cameraPos
	m_position = playerPos + offSet;
	setDirection(-offSet);
	updateViewMatrix();
	updateProjMatrix();
}

void ShadowMapper::copyStaticToDynamic() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->CopyResource(m_depthMap.Get(), m_depthMapStatic.Get());
}

void ShadowMapper::createShadowInfo() {
	m_shadowInfo.nearFarPlane = float2(m_nearPlane, m_farPlane);
	m_shadowInfo.ShadowMapRes = float2((float)m_smapSize.x, (float)m_smapSize.y);
	m_lightDirection = float3(0, -1.f, 0);
	m_lightDirection.Normalize();
	m_shadowInfo.toLight = float4(m_lightDirection.x, m_lightDirection.y, m_lightDirection.z, 1.0f);
}

void ShadowMapper::bindInfoBuffer() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_ShadowInfoBuffer.Get(), 0, NULL, &m_shadowInfo, 0, 0);
	deviceContext->PSSetConstantBuffers(6, 1, m_ShadowInfoBuffer.GetAddressOf());
}

vector<FrustumPlane> ShadowMapper::getFrustumPlanes() const {
	vector<FrustumPlane> planes;
	planes.reserve(6);

	float3 forward = m_lightDirection;
	forward.Normalize();
	float3 right = forward.Cross(float3(0, 1.f, 0));
	right.Normalize();
	float3 up = right.Cross(forward);
	up.Normalize();

	float3 center = m_position;
	float height = m_size.y / 2.f;
	float width = m_size.x / 2.f;
	float nearP = m_nearPlane;
	float farP = m_farPlane;
	float betweenP = (nearP + farP) / 2.f;

	float3 p_left = center + forward * betweenP + right * width * -1.f;
	float3 p_right = center + forward * betweenP + right * width * 1.f;
	float3 p_up = center + forward * betweenP + up * height * 1.f;
	float3 p_down = center + forward * betweenP + up * height * -1.f;
	float3 p_forward = center + forward * farP;
	float3 p_backward = center + forward * nearP;

	planes.push_back(FrustumPlane(p_left, -right));
	planes.push_back(FrustumPlane(p_right, right));
	planes.push_back(FrustumPlane(p_up, up));
	planes.push_back(FrustumPlane(p_down, -up));
	planes.push_back(FrustumPlane(p_forward, forward));
	planes.push_back(FrustumPlane(p_backward, -forward));

	planes.push_back(FrustumPlane(p_backward + forward, -forward));
	planes.push_back(FrustumPlane(p_backward + right, -forward));
	planes.push_back(FrustumPlane(p_backward + up, -forward));

	return planes;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowMapper::getDepthMapSRV() {
	return m_shadowSRV;
}

void ShadowMapper::bindDSVAndSetNullRenderTarget() {
	auto deviceContext = Renderer::getDeviceContext();

	deviceContext->RSSetViewports(1, &m_shadowPort);

	deviceContext->OMSetRenderTargets(1, m_nullRenderTargets, m_shadowDSV.Get());

	deviceContext->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMapper::bindDSVAndSetNullRenderTargetAndCopyStatic() {
	auto deviceContext = Renderer::getDeviceContext();

	deviceContext->RSSetViewports(1, &m_shadowPort);

	deviceContext->OMSetRenderTargets(1, m_nullRenderTargets, m_shadowDSV.Get());

	deviceContext->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	copyStaticToDynamic();
}

void ShadowMapper::bindDSVAndSetNullRenderTargetStatic() {
	auto deviceContext = Renderer::getDeviceContext();

	deviceContext->RSSetViewports(1, &m_shadowPort);

	deviceContext->OMSetRenderTargets(1, m_nullRenderTargets, m_staticShadowDSV.Get());

	deviceContext->ClearDepthStencilView(m_staticShadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
