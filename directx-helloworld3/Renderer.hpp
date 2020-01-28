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
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getDeviceContext();

private:
	void createDevice(Window& window);
	void createRenderTarget();

	// Device stuff
	static Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	static Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	D3D11_TEXTURE2D_DESC m_backBufferDesc;
};
