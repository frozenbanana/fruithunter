#include "SimpleDirectX.h"

Layer::Layer() {}

Layer::Layer(XMUINT2 size, DXGI_FORMAT format, UINT d3d11_bind_flags, D3D11_USAGE usage) { set(size, format, d3d11_bind_flags, usage); }

bool Layer::set(XMUINT2 size, DXGI_FORMAT format, UINT d3d11_bind_flags, D3D11_USAGE usage) {
	reset();

	D3D11_TEXTURE2D_DESC descTex;
	descTex.Width = size.x;
	descTex.Height = size.y;
	descTex.ArraySize = 1;
	descTex.MipLevels = 1;
	descTex.Format = format;
	descTex.Usage = usage;
	descTex.BindFlags = d3d11_bind_flags;
	descTex.CPUAccessFlags = 0;
	descTex.MiscFlags = 0;
	descTex.SampleDesc.Count = 1;
	descTex.SampleDesc.Quality = 0;
	HRESULT res = Renderer::getDevice()->CreateTexture2D(&descTex, 0, m_tex2D.GetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(Layer) Failed creating Texture2D!", res);
		return false;
	}

	if (d3d11_bind_flags & D3D11_BIND_SHADER_RESOURCE) {
		// shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = descTex.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		HRESULT srvHR = Renderer::getDevice()->CreateShaderResourceView(
			m_tex2D.Get(), &srvDesc, m_srv.GetAddressOf());
		if (FAILED(srvHR)) {
			ErrorLogger::logError("(Layer) Failed creating ShaderResourceView!", srvHR);
			return false;
		}
	}
	if (d3d11_bind_flags & D3D11_BIND_UNORDERED_ACCESS) {
		// unordered access view
		HRESULT hr = Renderer::getDevice()->CreateUnorderedAccessView(
			m_tex2D.Get(), NULL, m_uav.GetAddressOf());
		if (FAILED(hr)) {
			ErrorLogger::logError("(Layer) Failed creating UnorderedAccessView!", hr);
			return false;
		}
	}
	if (d3d11_bind_flags & D3D11_BIND_RENDER_TARGET) {
		// render target
		HRESULT hr = Renderer::getDevice()->CreateRenderTargetView(
			m_tex2D.Get(), NULL, m_rtv.GetAddressOf());
		if (FAILED(hr)) {
			ErrorLogger::logError("(Layer) Failed creating RenderTargetView!", hr);
			return false;
		}
	}
	m_initilized = true;
	return true;
}

void Layer::reset() {
	m_initilized = false;
	m_tex2D.Reset();
	m_srv.Reset();
	m_rtv.Reset();
	m_uav.Reset();
}

bool Layer::isInitilized() const { return m_initilized; }

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Layer::getSRV() { return m_srv; }

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Layer::getRTV() { return m_rtv; }

Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> Layer::getUAV() { return m_uav; }

D3D11_TEXTURE2D_DESC Layer::getDescription() const {
	D3D11_TEXTURE2D_DESC desc;
	if (m_tex2D.Get())
		m_tex2D->GetDesc(&desc);
	return desc;
}