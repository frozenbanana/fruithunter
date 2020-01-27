#pragma once

#include "Window.hpp"
#include <d3d11.h>
#include <WRL/client.h>

class Renderer {
public:
	Renderer(Window& window);
	~Renderer();
	void beginFrame();
	void endFrame();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getDeviceContext();
	ID3D11SamplerState* getSamplerState();

private:
	void createDevice(Window& window);
	void createRenderTarget();

	// Device stuff
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	D3D11_TEXTURE2D_DESC m_backBufferDesc;
};
