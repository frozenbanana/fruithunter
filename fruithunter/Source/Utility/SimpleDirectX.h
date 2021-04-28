#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <WRL/client.h>
#include <SimpleMath.h>

#include "Renderer.h"
#include "ErrorLogger.h"

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

class Layer {
private:
	bool m_initilized = false;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_tex2D;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;

public:
	Layer();
	Layer(XMUINT2 size, DXGI_FORMAT format, UINT d3d11_bind_flags,
		D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	bool set(XMUINT2 size, DXGI_FORMAT format, UINT d3d11_bind_flags,
		D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void reset();
	bool isInitilized() const;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getSRV();
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> getRTV();
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> getUAV();
	D3D11_TEXTURE2D_DESC getDescription() const;
};

template<typename STRUCT> class ConstantBuffer : public Microsoft::WRL::ComPtr<ID3D11Buffer> {
private:
	bool m_initilized = false;
	bool createBuffer();

public:
	ConstantBuffer();

	void update(const STRUCT& data);

	void bindVS(UINT slot);
	void bindGS(UINT slot);
	void bindPS(UINT slot);
	void bindCS(UINT slot);

};

template <typename STRUCT> inline bool ConstantBuffer<STRUCT>::createBuffer() {
	m_initilized = true;
	Reset();

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(STRUCT);

	HRESULT res = Renderer::getDevice()->CreateBuffer(&desc, nullptr, GetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(ConstantBuffer) Failed creating constant buffer!\n", res);
		return false;
	}
	return true;
}

template <typename STRUCT>
inline ConstantBuffer<STRUCT>::ConstantBuffer() : Microsoft::WRL::ComPtr<ID3D11Buffer>() {
}

template <typename STRUCT> inline void ConstantBuffer<STRUCT>::update(const STRUCT& data) {
	if (!m_initilized)
		createBuffer();
	Renderer::getDeviceContext()->UpdateSubresource(Get(), 0, 0, &data, 0, 0);
}

template <typename STRUCT>
inline void ConstantBuffer<STRUCT>::bindVS(UINT slot) {
	if (!m_initilized)
		createBuffer();
	Renderer::getDeviceContext()->VSSetConstantBuffers(slot, 1, GetAddressOf());
}

template <typename STRUCT>
inline void ConstantBuffer<STRUCT>::bindGS(UINT slot) {
	if (!m_initilized)
		createBuffer();
	Renderer::getDeviceContext()->GSSetConstantBuffers(slot, 1, GetAddressOf());
}

template <typename STRUCT>
inline void ConstantBuffer<STRUCT>::bindPS(UINT slot) {
	if (!m_initilized)
		createBuffer();
	Renderer::getDeviceContext()->PSSetConstantBuffers(slot, 1, GetAddressOf());
}

template <typename STRUCT> inline void ConstantBuffer<STRUCT>::bindCS(UINT slot) {
	if (!m_initilized)
		createBuffer();
	Renderer::getDeviceContext()->CSSetConstantBuffers(slot, 1, GetAddressOf());
}
