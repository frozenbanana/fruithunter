#pragma once
#include "GlobalNamespaces.h"
#include "ShaderSet.h"
#include "ShadowMapping.h"
#include <CommonStates.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SCREEN_HEIGHT Renderer::getInstance()->getScreenHeight()
#define SCREEN_WIDTH Renderer::getInstance()->getScreenWidth()

class Renderer {
public:
	enum DrawingState { state_normal, state_shadow };
	enum ScreenMode { Screen_Windowed = 0, Screen_Fullscreen = 1, Screen_Borderless = 2, ScreenModeSize };
	static void initalize(HWND window);
	void beginFrame();
	void endFrame();
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getDeviceContext();
	static Renderer* getInstance();
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> getDepthStencilView();
	HWND getHandle();
	float getScreenWidth() const;
	float getScreenHeight() const;
	LONG getWindowWidth() const;
	LONG getWindowHeight() const;

	void clearDepth();

	void bindDepthSRVCopy(int slot);
	void bindDepthSRV(int slot);
	void bindTargetSRVCopy(int slot);
	void bindConstantBuffer_ScreenSize(int slot);
	void bindQuadVertexBuffer();
	void changeResolution(int width, int height);
	void setFullscreen(bool value);
	bool isFullscreen() const;
	void setScreenMode(ScreenMode mode);
	ScreenMode getScreenMode() const;
	void copyDepthToSRV();
	void copyTargetToSRV();

	void bindRenderAndDepthTarget();
	void bindRenderTarget();

	void createCommonStates();
	// BlendState
	void setBlendState_Opaque(); // ignore alpha
	void setBlendState_AlphaBlend(); // use alpha channel
	void setBlendState_Additive();
	void setBlendState_Subtractive();
	void setBlendState_NonPremultiplied();
	// Rasterizer
	void setRasterizer_CullCounterClockwise(); // backface culling
	void setRasterizer_CullNone(); // None culling
	void setRasterizer_Wireframe();
	// DepthState
	void setDepthState_None(); // none
	void setDepthState_Default(); // read & write
	void setDepthState_Read(); // read

	void captureFrame();
	void drawCapturedFrame();

	void draw_darkEdges();

	void draw_FXAA();

	void setGodRaysSourcePosition(float3 position);
	void draw_godRays(const float4x4& viewProjMatrix);

	void setDrawState(DrawingState state);
	ShadowMapper* getShadowMapper();
	static void draw(size_t vertexCount, size_t vertexOffset);

private:
	Renderer(int width, int height);
	~Renderer();
	void createDevice(HWND window);
	void createRenderTarget();
	void createDepthBuffer(DXGI_SWAP_CHAIN_DESC& scd);
	void createConstantBuffers();
	void createQuadVertexBuffer();
	void createBlendStates();

	bool SetWindowStyle_Borderless();
	bool SetWindowStyle_Windowed();

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

	//Depth buffer
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;	 // Depth stencil view
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV; // Depth shader resource view
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthCopySRV; // Depth copy shader resource view

	//buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_screenSizeBuffer;

	// post process dark edges variables
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexQuadBuffer;
	ShaderSet m_shader_darkEdges;

	// post process FXAA variables
	ShaderSet m_shader_FXAA;

	// God Rays
	ShaderSet m_shader_godRays;
	float3 m_godRays_position;
	struct GodRaysSettings {
		float2 gSunPos;
		float gInitDecay = 0.116f;
		float gDistDecay = 1;
		float3 gRayColor = float3(25 / 255.f, 25 / 255.f, 16 / 255.f);
		float gMaxDeltaLen = 1;
	} m_settings_godRays;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbuffer_godRays;

	// Menu Background
	ShaderSet m_shader_drawTexture;

	// shadows
	ShadowMapper m_shadowMapper;
	DrawingState m_drawState = state_normal;

	// Resolution
	ScreenMode m_screenMode = ScreenMode::Screen_Windowed;
	int m_screenWidth;
	int m_screenHeight;

	std::unique_ptr<CommonStates> m_commonStates;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState_subtractive;
};
