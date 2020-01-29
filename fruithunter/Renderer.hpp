#pragma once

#include "Window.hpp"
#include <d3d11.h>
#include <WRL/client.h>

class Renderer {
public:
	static void initalize(HWND window);
	void beginFrame();
	void endFrame();
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getDeviceContext();
	static Renderer* getInstance();

private:
	Renderer();
	~Renderer();
	void createDevice(HWND window);
	void createRenderTarget();
	static Renderer m_this;
	bool m_isLoaded = false;

	// Device stuff
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	D3D11_TEXTURE2D_DESC m_backBufferDesc;
};
