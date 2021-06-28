#include "Texture.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "filesystemHelper.h"

bool Texture::create(XMINT2 size, DXGI_FORMAT format) { return create(size, D3D11_BIND_SHADER_RESOURCE, format); }

bool Texture::load(string path) {
	wstring str(path.begin(), path.end());

	m_SRV.Reset();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT res = CreateWICTextureFromFile(Renderer::getDevice(), str.c_str(),
		resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(Texture) Failed to create SRV buffer! Path: " + path, res);
		return false;
	}

	resource.As(&m_tex2D);
	CD3D11_TEXTURE2D_DESC texDesc;
	m_tex2D->GetDesc(&texDesc);

	m_path = path;
	vector<string> sections = splitPath(path);
	m_filename = sections.back();
	m_size = XMINT2(texDesc.Width, texDesc.Height);
	m_loaded = true;
	return true;
}

Texture::~Texture() {}

bool Texture::create(XMINT2 size, D3D11_BIND_FLAG bindFlags, DXGI_FORMAT format) { 	// texture 2d
	m_size = size;
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = size.x;
	texDesc.Height = size.y;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = format;
	texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = bindFlags;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	HRESULT res = Renderer::getDevice()->CreateTexture2D(&texDesc, NULL, m_tex2D.GetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(Texture) Failed creating Texture2D buffer!", res);
		return false;
	}

	// srv
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	res = Renderer::getDevice()->CreateShaderResourceView(
		m_tex2D.Get(), &srvDesc, m_SRV.GetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(Texture) Failed creating ShaderResourceView buffer!", res);
		return false;
	}
	m_loaded = true;
	return true;
}

bool Texture::isLoaded() const { return m_loaded; }

XMINT2 Texture::getSize() const { return m_size; }

string Texture::getPath() const { return m_path; }

string Texture::getFilename() const { return m_filename; }

Microsoft::WRL::ComPtr<ID3D11Texture2D>& Texture::getTex2D() { return m_tex2D; }

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& Texture::getSRV() { return m_SRV; }

Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& RenderTexture::getRTV() { return m_RTV; }

bool RenderTexture::create(XMINT2 size, DXGI_FORMAT format) {
	if (Texture::create(
			size, (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET), format)) {
		CD3D11_TEXTURE2D_DESC texDesc;
		m_tex2D->GetDesc(&texDesc);
		// create RTV
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		HRESULT res = Renderer::getDevice()->CreateRenderTargetView(
			m_tex2D.Get(), &rtvDesc, m_RTV.GetAddressOf());
		if (FAILED(res)) {
			ErrorLogger::logError(
				"(RenderTexture) Failed to create RenderTargetView buffer!", res);
			return false;
		}
		return true;
	}
	return false;
}

bool RenderTexture::load(string path) {
	wstring str(path.begin(), path.end());
	// create SRV and Tex2D
	m_SRV.Reset();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT res = CreateWICTextureFromFileEx(Renderer::getDevice(), str.c_str(), 0,
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, 0,
		WIC_LOADER_DEFAULT, resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(RenderTexture) Failed to create SRV buffer! Path: " + path, res);
		return false;
	}
	// get properties
	m_tex2D.Reset();
	resource.As(&m_tex2D);
	CD3D11_TEXTURE2D_DESC texDesc;
	m_tex2D->GetDesc(&texDesc);
	m_size = XMINT2(texDesc.Width, texDesc.Height);

	// create RTV
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	res = Renderer::getDevice()->CreateRenderTargetView(resource.Get(), &rtvDesc, m_RTV.GetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(RenderTexture) Failed to create RTV buffer! Path: " + path, res);
		return false;
	}

	m_path = path;
	vector<string> sections = splitPath(path);
	m_filename = sections.back();

	m_loaded = true;
	return true;
}
