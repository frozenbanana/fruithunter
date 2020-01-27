#include "Renderer.hpp"
#include "ErrorLogger.hpp"

Renderer::Renderer(Window& window) {
	createDevice(window);
	createRenderTarget();
}

Renderer::~Renderer() {}

void Renderer::beginFrame() {
	// Bind rendertarget
	m_deviceContext.Get()->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	// Set viewport
	auto viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)m_backBufferDesc.Width, (float)m_backBufferDesc.Height);
	m_deviceContext->RSSetViewports(1, &viewport);

	float clearColor[] = { 0.25f, .5f, 1, 1 };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}

void Renderer::endFrame() {
	// Swap the buffer
	m_swapChain->Present(1, 0);
}

ID3D11Device* Renderer::getDevice() { return m_device.Get(); }
ID3D11DeviceContext* Renderer::getDeviceContext() { return m_deviceContext.Get(); }
ID3D11SamplerState* Renderer::getSamplerState() { return m_samplerState.Get(); };

void Renderer::createDevice(Window& window) {
	// Define our swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window.getHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	// Create the swap chain, device and device context
	HRESULT swpFlag = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
		D3D11_SDK_VERSION, &swapChainDesc, m_swapChain.GetAddressOf(), m_device.GetAddressOf(), nullptr,
		m_deviceContext.GetAddressOf());

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(swpFlag)) {
		ErrorLogger::log(swpFlag, L"Error creating DX11.");
		return;
	}

	// Sampler
	HRESULT ssFlag = m_device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());

	if (FAILED(ssFlag)) {
		ErrorLogger::log(ssFlag, "Failed to initalize sampler state.");
		return;
	}
}

void Renderer::createRenderTarget() {
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT bFlag = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(bFlag)) {
		ErrorLogger::log(bFlag, "Failed to get back buffer.");
		return;
	};

	HRESULT rtFlag = m_device->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetView.GetAddressOf());
	if (FAILED(rtFlag)) {
		ErrorLogger::log(bFlag, "Failed to get create render target view.");
		return;
	};

	backBuffer->GetDesc(&m_backBufferDesc);
	if (backBuffer != nullptr)
		backBuffer->Release();
}
