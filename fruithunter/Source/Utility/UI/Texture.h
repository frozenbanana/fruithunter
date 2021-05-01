#pragma once
#include "GlobalNamespaces.h"
#include <WICTextureLoader.h>

#define PATH_SPRITE "assets/sprites/"

class Texture {
protected:
	bool m_loaded = false;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_tex2D;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	XMINT2 m_size = XMINT2(0, 0);
	string m_path = "";

	bool create(XMINT2 size,
		D3D11_BIND_FLAG bindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE,
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

public:
	bool isLoaded() const;
	XMINT2 getSize() const;
	string getPath() const;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>& getTex2D();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& getSRV();

	bool create(XMINT2 size, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
	bool load(string path);

	virtual ~Texture();
};

class RenderTexture : public Texture {
private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;

public:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& getRTV();

	bool create(XMINT2 size, DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
	bool load(string path);
};
