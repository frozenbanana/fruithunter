#pragma once

#include <d3d11.h>
#include <WRL/client.h>
#include "ErrorLogger.hpp"
#include <Keyboard.h>
#include <Mouse.h>

#define STANDARD_WIDTH 800
#define STANDARD_HEIGHT 600

class Renderer {
public:
	static void initalize(HWND window);
	void beginFrame();
	void endFrame();
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getDeviceContext();
	static Renderer* getInstance();
	HWND getHandle();

private:
	Renderer(int width, int height);
	~Renderer();
	void createDevice(HWND window);
	void createRenderTarget();
	static Renderer m_this;
	bool m_isLoaded = false;

	HWND m_handle;
	const LPCWSTR m_windowTitle = L"Fruit Hunter";

	// Device stuff
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	D3D11_TEXTURE2D_DESC m_backBufferDesc;
};
