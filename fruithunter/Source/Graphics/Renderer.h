#pragma once
#include "GlobalNamespaces.h"
#include "ShaderSet.h"
#include "Quad.h"
#include "ShadowMapping.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SCREEN_HEIGHT Renderer::getInstance()->getScreenHeight()
#define SCREEN_WIDTH Renderer::getInstance()->getScreenWidth()

class Renderer {
public:
	enum DrawingState { state_normal, state_shadow };
	static void initalize(HWND window);
	void beginFrame();
	void endFrame();
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getDeviceContext();
	static Renderer* getInstance();
	HWND getHandle();
	ID3D11DepthStencilState* getDepthDSS() const;
	float getScreenWidth() const;
	float getScreenHeight() const;

	void bindBackAndDepthBuffer();
	void clearDepth();

	void bindEverything();
	void bindDepthSRVCopy(int slot);
	void bindTargetSRVCopy(int slot);
	void bindConstantBuffer_ScreenSize(int slot);
	void bindQuadVertexBuffer();
	void enableAlphaBlending();
	void disableAlphaBlending();
	void changeResolution(int width, int height);
	void setFullscreen(bool value);
	void copyDepthToSRV();
	void copyTargetToSRV();

	void bindRenderAndDepthTarget();
	void bindRenderTarget();

	void captureFrame();

	void drawCapturedFrame();

	void draw_darkEdges();

	void drawLoading();

	void setDrawState(DrawingState state);
	ShadowMapper* getShadowMapper();
	static void draw(size_t vertexCount, size_t vertexOffset);

private:
	Renderer(int width, int height);
	~Renderer();
	void createDevice(HWND window);
	void createRenderTarget();
	void createDepthBuffer(DXGI_SWAP_CHAIN_DESC& scd);
	void createDepthState();
	void createConstantBuffers();
	void createQuadVertexBuffer();
	void createBlendState();

	static Renderer m_this;
	bool m_isLoaded = false;

	ImGuiContext* ctx = nullptr;

	HWND m_handle;
	const LPCWSTR m_windowTitle = L"Fruit Hunter";

	// Device stuff
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

	//RenderTarget
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_targetSRVCopy;
	D3D11_TEXTURE2D_DESC m_backBufferDesc;

	//BlendState
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateAlphaBlending;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateWithoutAlphaBlending;

	//Depth buffer
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;	 // Depth stencil view
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDSS;	 // Depth stencil state
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV; // Depth shader resource view
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBufferTex;

	//buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_screenSizeBuffer;

	// post process dark edges variables
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexQuadBuffer;
	ShaderSet m_shader_darkEdges;

	// Loading screen
	Quad m_loadingScreen;
	bool m_loadingScreenInitialised = false;

	// Menu Background
	Quad m_capturedFrame;
	bool m_capturedFrameLoaded = false;

	// shadows
	ShadowMapper m_shadowMapper;
	DrawingState m_drawState = state_normal;

	// Resolution
	int m_screenWidth;
	int m_screenHeight;
};
