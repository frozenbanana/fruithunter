#pragma once

#include "Window.hpp"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <WRL/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ErrorLogger.hpp"

#include <WICTextureLoader.h>
#include <SimpleMath.h>

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

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
