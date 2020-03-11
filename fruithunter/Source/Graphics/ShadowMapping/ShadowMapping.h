#pragma once
#include "GlobalNamespaces.h"

#define SMAP_SIZE 2048.f

class ShadowMapper {
private:
	//Variables
	XMINT2 m_shadowPortSize =
		XMINT2(static_cast<int>(1920), static_cast<int>(1080));
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_staticShadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_staticShadowSRV;
	D3D11_VIEWPORT m_shadowPort;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthMap;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthMapStatic;
	ID3D11RenderTargetView* m_nullRenderTargets[1] = { 0 };
	Matrix m_vpMatrix_t;
	Matrix m_viewMatrix;
	Matrix m_projMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixVPTBuffer;
	Matrix m_VPT;
	bool m_staticShadowDrawn = false;

	//Functions
	void createCameraBuffer();
	void createVPTBuffer();
	void createVPTMatrix();
	
	
public:
	//Utility
	ShadowMapper();
	~ShadowMapper();
	//void initiate(UINT width, UINT height);
	void initiate();
	void bindCameraMatrix(); // Camera
	void bindVPTMatrix(); //Matrix that moves from localSpace to NDC
	void bindShadowMap();
	void update(float3);
	void copyStaticToDynamic();
	void clearAllShadows();
	bool IsStaticDrawn();

	// Shadow functions
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getDepthMapSRV();
	void bindDSVAndSetNullRenderTarget();
	void bindDSVAndSetNullRenderTargetStatic();
};