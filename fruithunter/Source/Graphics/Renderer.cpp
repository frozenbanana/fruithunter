#include "Renderer.h"
#include "ErrorLogger.h"
#include "Settings.h"
#include <WICTextureLoader.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <ScreenGrab.h>
#include <wincodec.h>

Renderer Renderer::m_this(1280, 720);

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
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

void Renderer::bindDepthSRVCopy(int slot) {
	m_deviceContext->PSSetShaderResources(slot, 1, m_depthSRV.GetAddressOf());
}

void Renderer::bindTargetSRVCopy(int slot) {
	m_deviceContext->PSSetShaderResources(slot, 1, m_targetSRVCopy.GetAddressOf());
}

void Renderer::enableAlphaBlending() {
	float blendFactor[4] = { 0 };
	m_deviceContext->OMSetBlendState(m_blendStateAlphaBlending.Get(), blendFactor, 0xffffffff);
}

void Renderer::disableAlphaBlending() {
	float blendFactor[4] = { 0 };
	m_deviceContext->OMSetBlendState(
		m_blendStateWithoutAlphaBlending.Get(), blendFactor, 0xffffffff);
}

void Renderer::changeResolution(int width, int height) {
	m_screenWidth = width;
	m_screenHeight = height;

	// 1. clear the existing references to the backbuffer
	ID3D11RenderTargetView* nullView = nullptr;
	m_deviceContext->OMSetRenderTargets(1, &nullView, nullptr);
	m_renderTargetView.Reset();
	m_depthDSV.Reset();
	m_deviceContext->Flush(); // not quite sure necessary ?

	// 2. Resize the existing swapchain
	HRESULT hr =
		m_swapChain->ResizeBuffers(2, m_screenWidth, m_screenHeight, m_backBufferDesc.Format, NULL);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		ErrorLogger::log("In renderer, could not resize buffers");
	DXGI_MODE_DESC modeDesc = { 0 };
	modeDesc.Width = m_screenWidth;
	modeDesc.Height = m_screenHeight;
	// modeDesc.Format = m_swapChain->GetDesc.format;
	m_swapChain->ResizeTarget(&modeDesc);
	// 3. Get the new backbuffer texture to use as a render target
	createRenderTarget();

	// 4. Create a depth/stencil buffer and create the depth stencil view
	DXGI_SWAP_CHAIN_DESC swap_desc;
	m_swapChain->GetDesc(&swap_desc);
	createDepthBuffer(swap_desc);

	// 5. Make sure other parts in program update with new screen sizes.
}

void Renderer::setFullscreen(bool value) { m_swapChain->SetFullscreenState(value, nullptr); }

void Renderer::bindConstantBuffer_ScreenSize(int slot) {
	XMINT4 data = XMINT4(m_screenWidth, m_screenHeight, 0, 0);
	m_deviceContext->UpdateSubresource(m_screenSizeBuffer.Get(), 0, 0, &data, 0, 0);
	m_deviceContext->PSSetConstantBuffers(slot, 1, m_screenSizeBuffer.GetAddressOf());
}

void Renderer::bindQuadVertexBuffer() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(float3);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexQuadBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::copyDepthToSRV() {
	ID3D11Resource *dst, *src;
	m_depthSRV.Get()->GetResource(&dst);
	m_depthDSV.Get()->GetResource(&src);
	m_deviceContext->CopyResource(dst, src);
}

void Renderer::copyTargetToSRV() {
	ID3D11Resource *dst, *src;
	m_targetSRVCopy.Get()->GetResource(&dst);
	m_renderTargetView.Get()->GetResource(&src);
	m_deviceContext->CopyResource(dst, src);
}

void Renderer::captureFrame() {
	auto hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_backBufferTex);
	if (FAILED(hr)) {
		ErrorLogger::logError(hr, "Failed to capture backbuffer.");
	}
	else {
		// Write out the render target to png
		hr = SaveWICTextureToFile(Renderer::getDeviceContext(), m_backBufferTex.Get(),
			GUID_ContainerFormatPng, L"assets/captures/backbuffer.png", nullptr, nullptr);
		m_capturedFrame.init();
		m_capturedFrameLoaded = true;
	}
}

void Renderer::drawCapturedFrame() {
	if (!m_capturedFrameLoaded) {
		m_capturedFrame.init();
		m_capturedFrameLoaded = true;
	}
	m_capturedFrame.draw();
}

void Renderer::draw_darkEdges() {
	if (Settings::getInstance()->getDarkEdges()) {
		//copy depth buffer
		copyDepthToSRV();
		bindDepthSRVCopy(0);
		//copy renderTarget
		copyTargetToSRV();
		bindTargetSRVCopy(1);
		// bind shader
		m_shader_darkEdges.bindShadersAndLayout();
		//bind contant buffer
		bindConstantBuffer_ScreenSize(6);
		//bind vertex buffer
		bindQuadVertexBuffer();

		enableAlphaBlending();
		m_deviceContext->Draw(6, 0);
		disableAlphaBlending();
	}
}

void Renderer::drawLoading() {
	beginFrame();
	if (!m_loadingScreenInitialised) {
		m_loadingScreen.init();
		m_loadingScreenInitialised = true;
	}
	m_loadingScreen.draw();
	endFrame();
}

void Renderer::setDrawState(DrawingState state) { m_drawState = state; }

ShadowMapper* Renderer::getShadowMapper() { return &m_shadowMapper; }

void Renderer::draw(size_t vertexCount, size_t vertexOffset) {
	auto renderer = Renderer::getInstance();
	if (renderer->m_drawState == state_shadow)
		renderer->m_deviceContext->PSSetShader(nullptr, nullptr, 0); // unplug pixelshader
	renderer->m_deviceContext->Draw((UINT)vertexCount, (UINT)vertexOffset);
}

Renderer::Renderer(int width, int height) {
	m_screenHeight = height;
	m_screenWidth = width;
	// Define window style
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WinProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = m_windowTitle;
	RegisterClass(&wc);

	// Create the window
	bool showTopBorder = true;
	m_handle = CreateWindow(m_windowTitle, m_windowTitle,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 0, m_screenWidth,
		m_screenHeight + 30 * showTopBorder, nullptr, nullptr, nullptr, nullptr);

	// Create device, deviceContext and swapchain
	Renderer* r = Renderer::getInstance();
	if (!r->m_isLoaded && r->m_device.Get() == nullptr && r->m_deviceContext.Get() == nullptr &&
		r->m_swapChain.Get() == nullptr) {
		r->createDevice(m_handle);
		r->createRenderTarget();
		r->createConstantBuffers();
		r->createQuadVertexBuffer();
		m_shadowMapper.initiate();
		r->m_isLoaded = true;
	}

	// compile shaders
	if (!m_shader_darkEdges.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout_onlyMesh[] = { {
			"Position",					 // "semantic" name in shader
			0,							 // "semantic" index (not used)
			DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
			0,							 // input slot
			0,							 // offset of first element
			D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
			0							 // used for INSTANCING (ignore)
		} };
		m_shader_darkEdges.createShaders(L"VertexShader_quadSimplePass.hlsl", nullptr,
			L"PixelShader_darkEdge.hlsl", inputLayout_onlyMesh, 1);
	}

	// Set texture paths to quads
	m_capturedFrame.setTexturePath("assets/captures/backbuffer.png");
	m_capturedFrame.setPixelShaderPath("PixelShader_blur.hlsl");
}

Renderer::~Renderer() {}

Renderer* Renderer::getInstance() { return &m_this; }

HWND Renderer::getHandle() { return m_handle; }

ID3D11DepthStencilState* Renderer::getDepthDSS() const { return m_depthDSS.Get(); }

float Renderer::getScreenWidth() const { return (float)m_screenWidth; }

float Renderer::getScreenHeight() const { return (float)m_screenHeight; }

void Renderer::initalize(HWND window) {}

void Renderer::beginFrame() {
	m_deviceContext->RSSetState(0);

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
	m_swapChain->Present(Settings::getInstance()->getVsync(), 0);
}

ID3D11Device* Renderer::getDevice() {
	Renderer* r = Renderer::getInstance();
	if (r->m_device.Get() != nullptr) {
		return r->m_device.Get();
	}
	else {
		ErrorLogger::logError(NULL, "Renderer : Trying to get device without being initalized.");
		return nullptr;
	}
}
ID3D11DeviceContext* Renderer::getDeviceContext() {
	Renderer* r = Renderer::getInstance();
	if (r->m_deviceContext.Get() != nullptr) {
		return r->m_deviceContext.Get();
	}
	else {
		ErrorLogger::logError(
			NULL, "Renderer : Trying to get device context without being initalized.");
		return nullptr;
	}
}

void Renderer::createDevice(HWND window) {
	// Define our swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format =
		DXGI_FORMAT_R8G8B8A8_UNORM; // This bufferdesc becomed m_backBufferDesc
	swapChainDesc.BufferDesc.Width = m_screenWidth;
	swapChainDesc.BufferDesc.Height = m_screenHeight;
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

	//Render ShaderResourceView Copy
	D3D11_TEXTURE2D_DESC copyDesc;
	copyDesc.Width = m_screenWidth;
	copyDesc.Height = m_screenHeight;
	copyDesc.ArraySize = 1;
	copyDesc.MipLevels = 1;
	copyDesc.Format = swapChainDesc.BufferDesc.Format;
	copyDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	copyDesc.CPUAccessFlags = 0;
	copyDesc.MiscFlags = 0;
	copyDesc.SampleDesc = swapChainDesc.SampleDesc; // same as swapChain
	ID3D11Texture2D* texCopy = 0;
	HRESULT res = m_device->CreateTexture2D(&copyDesc, 0, &texCopy);
	if (FAILED(res))
		ErrorLogger::logError(res, "(Renderer) Failed creating render 2d texture copy!");
	//Shader resourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = copyDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HRESULT srvHR =
		m_device->CreateShaderResourceView(texCopy, &srvDesc, m_targetSRVCopy.GetAddressOf());
	if (FAILED(srvHR))
		ErrorLogger::logError(srvHR, "(Renderer) Failed creating render SRV copy!");
	texCopy->Release();

	createDepthState();
	createDepthBuffer(swapChainDesc);
	createBlendState();
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
	// texture 2d
	D3D11_TEXTURE2D_DESC DeStDesc;
	DeStDesc.Width = m_screenWidth;
	DeStDesc.Height = m_screenHeight;
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
	if (FAILED(res))
		ErrorLogger::logError(res, "(Renderer) Failed creating depth 2D texture!");

	// depth stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags = 0;
	viewDesc.Texture2D.MipSlice = 0;
	HRESULT hr2 = m_device->CreateDepthStencilView(tex, &viewDesc, m_depthDSV.GetAddressOf());
	if (FAILED(hr2))
		ErrorLogger::logError(hr2, "(Renderer) Failed creating depth stencil view!");

	tex->Release();

	//	DEPTH COPY

	// texture 2d copy
	ID3D11Texture2D* texCopy = 0;
	D3D11_TEXTURE2D_DESC CopyDeStDesc = DeStDesc;
	DeStDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	res = m_device->CreateTexture2D(&CopyDeStDesc, 0, &texCopy);
	if (FAILED(res))
		ErrorLogger::logError(res, "(Renderer) Failed creating depth 2D texture copy!");

	// depth shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT; // D32_FLOAT = INVALID, R32_FLOAT = SUCCESS
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HRESULT srvHR =
		m_device->CreateShaderResourceView(texCopy, &srvDesc, m_depthSRV.GetAddressOf());
	if (FAILED(srvHR))
		ErrorLogger::logError(srvHR, "(Renderer) Failed creating depthSRV!");

	texCopy->Release();
}

void Renderer::createDepthState() {
	D3D11_DEPTH_STENCIL_DESC DeStState;
	DeStState.DepthEnable = true;
	DeStState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DeStState.DepthFunc = D3D11_COMPARISON_LESS;
	DeStState.StencilEnable = true;
	DeStState.StencilReadMask = 0xFF;
	DeStState.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	DeStState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DeStState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DeStState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DeStState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	DeStState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DeStState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DeStState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DeStState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT hr = m_device->CreateDepthStencilState(&DeStState, m_depthDSS.GetAddressOf());
	m_deviceContext->OMSetDepthStencilState(m_depthDSS.Get(), 1);
}

void Renderer::createConstantBuffers() {
	// screen size Buffer
	m_screenSizeBuffer.Reset();
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(XMINT4);

	HRESULT res =
		Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_screenSizeBuffer.GetAddressOf());
	if (FAILED(res))
		ErrorLogger::logError(res, "Failed creating screen size buffer in Renderer class!\n");
}

void Renderer::createQuadVertexBuffer() {
	if (m_vertexQuadBuffer.Get() == nullptr) {
		float3 points[4] = { float3(-1, -1, 0), float3(1, -1, 0), float3(-1, 1, 0),
			float3(1, 1, 0) };
		float3 quadData[6] = { points[0], points[3], points[1], points[0], points[2], points[3] };
		// vertex buffer
		m_vertexQuadBuffer.Reset();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)6 * sizeof(float3);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = quadData;
		HRESULT res = Renderer::getDevice()->CreateBuffer(
			&bufferDesc, &data, m_vertexQuadBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Failed creating quad vertex buffer in Renderer class!\n");
	}
}

void Renderer::createBlendState() {
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.RenderTarget->BlendEnable = false;
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;

	HRESULT hr = m_device->CreateBlendState(NULL, m_blendStateWithoutAlphaBlending.GetAddressOf());
	// HRESULT hr = m_device->CreateBlendState(&blendStateDesc,
	// m_blendStateWithoutAlphaBlending.GetAddressOf());

	// create a blend state with alpha blending
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.RenderTarget->BlendEnable = true;
	blendStateDesc.AlphaToCoverageEnable = true;
	blendStateDesc.IndependentBlendEnable = false;

	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	HRESULT hr2 =
		m_device->CreateBlendState(&blendStateDesc, m_blendStateAlphaBlending.GetAddressOf());
	disableAlphaBlending();
}
