#include "ShadowMapping.h"
#include "Renderer.h"

void ShadowMapper::initiate(UINT width, UINT height) {
	auto device = Renderer::getDevice();

	m_shadowPort.TopLeftX = 0.0f;
	m_shadowPort.TopLeftY = 0.0f;
	m_shadowPort.Width = static_cast<float>(width);
	m_shadowPort.Height = static_cast<float>(height);
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

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
	HRESULT hr_0 = device->CreateTexture2D(&shadowTexDesc, 0, &depthMap);
	
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc;
	shadowDSVDesc.Flags = 0;
	shadowDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSVDesc.Texture2D.MipSlice = 0;

	HRESULT hr_1 = device->CreateDepthStencilView(depthMap.Get(), &shadowDSVDesc, &m_shadowDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	shadowSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = shadowTexDesc.MipLevels;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr_2 = device->CreateShaderResourceView(depthMap.Get(), &shadowSRVDesc, &m_shadowSRV);
}

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

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
	HRESULT hr_0 = device->CreateTexture2D(&shadowTexDesc, 0, &depthMap);

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc;
	shadowDSVDesc.Flags = 0;
	shadowDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSVDesc.Texture2D.MipSlice = 0;

	HRESULT hr_1 = device->CreateDepthStencilView(depthMap.Get(), &shadowDSVDesc, &m_shadowDSV);

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	shadowSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = shadowTexDesc.MipLevels;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;

	HRESULT hr_2 = device->CreateShaderResourceView(depthMap.Get(), &shadowSRVDesc, &m_shadowSRV);
}
