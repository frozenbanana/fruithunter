#pragma once
#include "GlobalNamespaces.h"

#define SMAP_SIZE 2048.0f

class ShadowMapper {
private:
	//Variables
	XMINT2 m_shadowPortSize =
		XMINT2(static_cast<int>(SMAP_SIZE), static_cast<int>(SMAP_SIZE));
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowSRV;
	D3D11_VIEWPORT m_shadowPort;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthMap;
	ID3D11RenderTargetView* m_nullRenderTargets[1] = { 0 };
	Matrix m_vpMatrix_t;
	Matrix m_viewMatrix;
	Matrix m_projMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixVPTBuffer;
	Matrix m_VPT;

	//Functions
	void createBuffer();
	void createVPTBuffer();
	void createVPTMatrix();
	
	
public:
	//Utility
	ShadowMapper();
	~ShadowMapper();
	//void initiate(UINT width, UINT height);
	void initiate();
	void bindMatrix(); //Camera
	void bindVPTMatrix(); //Matrix that moves from localSpace to NDC
	void bindShadowMap();

	// Shadow functions
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getDepthMapSRV();
	void bindDSVAndSetNullRenderTarget();
};