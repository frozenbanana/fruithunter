#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <WRL/client.h>
#include "ErrorLogger.hpp"
#include <Keyboard.h>
#include <Mouse.h>

#define STANDARD_WIDTH 800
#define STANDARD_HEIGHT 600
#define STANDARD_CORNER_X 100
#define STANDARD_CORNER_Y 100


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
	HWND getHandle();

	void bindBackAndDepthBuffer();
	void clearDepth();
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;


private:
	Renderer(int width, int height);
	~Renderer();
	void createDevice(HWND window);
	void createRenderTarget();
	void createDepthBuffer(DXGI_SWAP_CHAIN_DESC& scd);
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


	// RECT r
};
