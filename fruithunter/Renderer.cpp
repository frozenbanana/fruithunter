#include "Renderer.h"
#include "ErrorLogger.h"
#include <WICTextureLoader.h>
#include <Keyboard.h>
#include <Mouse.h>

Renderer Renderer::m_this(STANDARD_WIDTH, STANDARD_HEIGHT);

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
	// if (msg == WM_DESTROY || msg == WM_CLOSE) {
	//	PostQuitMessage(0);
	//	return 0;
	//}
	switch (msg) {
	case WM_DESTROY:
	case WM_CLOSE:
	case WM_QUIT:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
		DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
		break;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
}

void Renderer::bindBackAndDepthBuffer() {
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthDSV.Get());
}

void Renderer::clearDepth() {
	m_deviceContext->ClearDepthStencilView(m_depthDSV.Get(), D3D11_CLEAR_DEPTH, 1, 0);
}

void Renderer::bindEverything() { bindBackAndDepthBuffer(); }


Renderer::Renderer(int width, int height) {
	// Define window style
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WinProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = m_windowTitle;
	RegisterClass(&wc);

	// Create the window
	m_handle = CreateWindow(m_windowTitle, m_windowTitle,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, STANDARD_CORNER_X, STANDARD_CORNER_Y,
		width, height, nullptr, nullptr, nullptr, nullptr);

	// Create device, deviceContext and swapchain
	Renderer* r = Renderer::getInstance();
	if (!r->m_isLoaded && r->m_device.Get() == nullptr && r->m_deviceContext.Get() == nullptr &&
		r->m_swapChain.Get() == nullptr) {
		r->createDevice(m_handle);
		r->createRenderTarget();
		r->m_isLoaded = true;
	}
}

Renderer::~Renderer() {}

Renderer* Renderer::getInstance() { return &m_this; }

HWND Renderer::getHandle() { return m_handle; }

void Renderer::initalize(HWND window) {}

void Renderer::beginFrame() {
	// Bind rendertarget
	m_deviceContext.Get()->OMSetRenderTargets(
		1, m_renderTargetView.GetAddressOf(), m_depthDSV.Get());

	// Set viewport
	// auto viewport =
	//	CD3D11_VIEWPORT(0.f, 0.f, (float)m_backBufferDesc.Width, (float)m_backBufferDesc.Height);
	// m_deviceContext->RSSetViewports(1, &viewport);
	D3D11_VIEWPORT vp;
	vp.Width = (float)m_backBufferDesc.Width;
	vp.Height = (float)m_backBufferDesc.Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);

	float clearColor[] = { 0.25f, .5f, 1, 1 };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_deviceContext->ClearDepthStencilView(m_depthDSV.Get(), D3D11_CLEAR_DEPTH, 1, 0);
}

void Renderer::endFrame() {
	// Swap the buffer
	m_swapChain->Present(1, 0);
}

ID3D11Device* Renderer::getDevice() {
	Renderer* r = Renderer::getInstance();
	return r->m_device.Get();
}
ID3D11DeviceContext* Renderer::getDeviceContext() {
	Renderer* r = Renderer::getInstance();
	return r->m_deviceContext.Get();
}

void Renderer::createDevice(HWND window) {
	// Define our swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	// Create the swap chain, device and device context
	HRESULT swpFlag = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(), nullptr, m_deviceContext.GetAddressOf());

	if (FAILED(swpFlag)) {
		ErrorLogger::messageBox(swpFlag, L"Error creating DX11.");
		return;
	}

	createDepthBuffer(swapChainDesc);
}

void Renderer::createRenderTarget() {
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT bFlag = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(bFlag)) {
		ErrorLogger::messageBox(bFlag, "Failed to get back buffer.");
		return;
	};

	HRESULT rtFlag =
		m_device->CreateRenderTargetView(backBuffer, NULL, m_renderTargetView.GetAddressOf());
	if (FAILED(rtFlag)) {
		ErrorLogger::messageBox(bFlag, "Failed to get create render target view.");
		return;
	};

	backBuffer->GetDesc(&m_backBufferDesc);
	if (backBuffer != nullptr)
		backBuffer->Release();
}

void Renderer::createDepthBuffer(DXGI_SWAP_CHAIN_DESC& scd) {
	D3D11_TEXTURE2D_DESC DeStDesc;
	DeStDesc.Width = 800;
	DeStDesc.Height = 600;
	DeStDesc.ArraySize = 1;
	DeStDesc.MipLevels = 1;
	DeStDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	DeStDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	DeStDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	DeStDesc.CPUAccessFlags = 0;
	DeStDesc.MiscFlags = 0;
	DeStDesc.SampleDesc = scd.SampleDesc; // same as swapChain
	ID3D11Texture2D* tex = 0;
	HRESULT res = m_device->CreateTexture2D(&DeStDesc, 0, &tex);
	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags = 0;
	viewDesc.Texture2D.MipSlice = 0;
	HRESULT hr2 = m_device->CreateDepthStencilView(tex, &viewDesc, m_depthDSV.GetAddressOf());

	tex->Release();
}
