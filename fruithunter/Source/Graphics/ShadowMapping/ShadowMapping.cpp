#include "ShadowMapping.h"
#include "Renderer.h"
 
void ShadowMapper::createBuffer() { 
	m_viewMatrix = XMMatrixLookAtLH(
		float3(21.43f, 36.567f, 183.6f), float3(22.06f, 36.2f, 182.9f), float3(0.f, 1.f, 0.f));
	m_projMatrix = XMMatrixOrthographicLH(285.f, 320.f, NEAR_PLANE, 250.f);
	/*m_projMatrix = XMMatrixPerspectiveFovLH(
		(XM_PI / 2.f), (float)STANDARD_WIDTH / (float)STANDARD_HEIGHT, NEAR_PLANE, FAR_PLANE);*/
	Matrix vp_matrix = XMMatrixMultiply(m_viewMatrix, m_projMatrix);

	m_vpMatrix_t = vp_matrix.Transpose();

	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_matrixBuffer.Get(), 0, NULL, &m_vpMatrix_t, 0, 0);
}

void ShadowMapper::createVPTBuffer() {
	createVPTMatrix();

	auto device = Renderer::getDevice();
	// Create constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_VPT);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_VPT;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_matrixVPTBuffer.GetAddressOf());

	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_matrixVPTBuffer.Get(), 0, NULL, &m_VPT, 0, 0);
}

void ShadowMapper::createVPTMatrix() { 
	m_VPT = XMMatrixMultiply(m_viewMatrix, m_projMatrix);
	Matrix textureMatrix = {
		0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.0f, 0.f,
		0.5f, 0.5, 0.f, 1.0f
	};
	m_VPT = XMMatrixMultiply(m_VPT, textureMatrix);
	//m_VPT.Transpose();
}

ShadowMapper::ShadowMapper() { initiate(); }

ShadowMapper::~ShadowMapper() { 
	delete m_nullRenderTargets[0];
}

//void ShadowMapper::initiate(UINT width, UINT height) {
//	auto device = Renderer::getDevice();
//
//	m_shadowPort.TopLeftX = 0.0f;
//	m_shadowPort.TopLeftY = 0.0f;
//	m_shadowPort.Width = static_cast<float>(width);
//	m_shadowPort.Height = static_cast<float>(height);
//	m_shadowPort.MinDepth = 0.0f;
//	m_shadowPort.MaxDepth = 1.0f;
//
//	D3D11_TEXTURE2D_DESC shadowTexDesc;
//	shadowTexDesc.Width = m_shadowPortSize.x;
//	shadowTexDesc.Height = m_shadowPortSize.y;
//	shadowTexDesc.MipLevels = 1;
//	shadowTexDesc.ArraySize = 1;
//	shadowTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
//	shadowTexDesc.SampleDesc.Count = 1;
//	shadowTexDesc.SampleDesc.Quality = 0;
//	shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
//	shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
//	shadowTexDesc.CPUAccessFlags = 0;
//	shadowTexDesc.MiscFlags = 0;
//
//	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
//	HRESULT hr_0 = device->CreateTexture2D(&shadowTexDesc, 0, &depthMap);
//	
//	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc;
//	shadowDSVDesc.Flags = 0;
//	shadowDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//	shadowDSVDesc.Texture2D.MipSlice = 0;
//
//	HRESULT hr_1 = device->CreateDepthStencilView(depthMap.Get(), &shadowDSVDesc, &m_shadowDSV);
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
//	shadowSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	shadowSRVDesc.Texture2D.MipLevels = shadowTexDesc.MipLevels;
//	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
//
//	HRESULT hr_2 = device->CreateShaderResourceView(depthMap.Get(), &shadowSRVDesc, &m_shadowSRV);
//}

void ShadowMapper::initiate() {
	auto device = Renderer::getDevice();

	m_shadowPort.TopLeftX = 0.0f;
	m_shadowPort.TopLeftY = 0.0f;
	m_shadowPort.Width = static_cast<float>(m_shadowPortSize.x);
	m_shadowPort.Height = static_cast<float>(m_shadowPortSize.y);
	m_shadowPort.MinDepth = 0.0f;
	m_shadowPort.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC shadowTexDesc;
	shadowTexDesc.Width = m_shadowPortSize.x;
	shadowTexDesc.Height = m_shadowPortSize.y;
	shadowTexDesc.MipLevels = 1;
	shadowTexDesc.ArraySize = 1;
	shadowTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowTexDesc.SampleDesc.Count = 1;
	shadowTexDesc.SampleDesc.Quality = 0;
	shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowTexDesc.CPUAccessFlags = 0;
	shadowTexDesc.MiscFlags = 0;

	
	HRESULT hr_0 = device->CreateTexture2D(&shadowTexDesc, 0, m_depthMap.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc;
	shadowDSVDesc.Flags = 0;
	shadowDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSVDesc.Texture2D.MipSlice = 0;

	HRESULT hr_1 = device->CreateDepthStencilView(m_depthMap.Get(), &shadowDSVDesc, &m_shadowDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	shadowSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = shadowTexDesc.MipLevels;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr_2 = device->CreateShaderResourceView(m_depthMap.Get(), &shadowSRVDesc, &m_shadowSRV);

	// Create constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(m_vpMatrix_t);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &m_vpMatrix_t;
	HRESULT res = device->CreateBuffer(&bufferDesc, &data, m_matrixBuffer.GetAddressOf());

	createBuffer();
	createVPTBuffer();
}

void ShadowMapper::bindMatrix() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(1, 1, m_matrixBuffer.GetAddressOf());
}

void ShadowMapper::bindVPTMatrix() {
	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->VSSetConstantBuffers(4, 1, m_matrixVPTBuffer.GetAddressOf());
}

void ShadowMapper::bindShadowMap() {
	

	auto deviceContext = Renderer::getDeviceContext();
	deviceContext->PSSetShaderResources(4, 1, m_shadowSRV.GetAddressOf());
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowMapper::getDepthMapSRV() {
	return m_shadowSRV;
}

void ShadowMapper::bindDSVAndSetNullRenderTarget() { 
	auto deviceContext = Renderer::getDeviceContext();
	ID3D11ShaderResourceView* test = { NULL };
	deviceContext->PSSetShaderResources(4, 1, &test);

	deviceContext->RSSetViewports(1, &m_shadowPort);

	//Set null render target to disable color
	
	deviceContext->OMSetRenderTargets(1, m_nullRenderTargets, m_shadowDSV.Get());
	deviceContext->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
