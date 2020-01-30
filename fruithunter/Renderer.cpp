#include "Renderer.hpp"

Renderer Renderer::m_this(STANDARD_WIDTH, STANDARD_HEIGHT);

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY || msg == WM_CLOSE) {
		PostQuitMessage(0);
		return 0;
	}

	switch (msg) {
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

Renderer::Renderer(int width, int height) {
	// Define window style
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WinProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = m_windowTitle;
	RegisterClass(&wc);

	// Create the window
	m_handle = CreateWindow(m_windowTitle, L"C++11 and DX11 Tutorial",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 100, 100, width, height, nullptr, nullptr,
		nullptr, nullptr);

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
	m_deviceContext.Get()->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	// Set viewport
	auto viewport =
		CD3D11_VIEWPORT(0.f, 0.f, (float)m_backBufferDesc.Width, (float)m_backBufferDesc.Height);
	m_deviceContext->RSSetViewports(1, &viewport);

	float clearColor[] = { 0.25f, .5f, 1, 1 };
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
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
}

void Renderer::createRenderTarget() {
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT bFlag = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(bFlag)) {
		ErrorLogger::messageBox(bFlag, "Failed to get back buffer.");
		return;
	};

	HRESULT rtFlag =
		m_device->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetView.GetAddressOf());
	if (FAILED(rtFlag)) {
		ErrorLogger::messageBox(bFlag, "Failed to get create render target view.");
		return;
	};

	backBuffer->GetDesc(&m_backBufferDesc);
	if (backBuffer != nullptr)
		backBuffer->Release();

	// Depth state
	auto depthDesc = CD3D11_DEPTH_STENCIL_DESC(FALSE, D3D11_DEPTH_WRITE_MASK_ZERO,
		D3D11_COMPARISON_LESS, FALSE, D3D11_DEFAULT_STENCIL_READ_MASK,
		D3D11_DEFAULT_STENCIL_WRITE_MASK, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS);
	m_device->CreateDepthStencilState(&depthDesc, m_depthState.GetAddressOf());
}
