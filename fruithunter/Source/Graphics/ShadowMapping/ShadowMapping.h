#pragma once
#include "GlobalNamespaces.h"

class ShadowMapper {
private:
	//Variables
	XMINT2 m_shadowPortSize;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowSRV;
	D3D11_VIEWPORT m_shadowPort;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthMap;
	ID3D11RenderTargetView* m_nullRenderTargets[1] = { 0 };
	Matrix m_vpMatrix_t;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	//Functions
	void createBuffer();
	
public:
	//Utility
	ShadowMapper();
	~ShadowMapper();
	//void initiate(UINT width, UINT height);
	void initiate();
	void bindMatrix();

	// Shadow functions
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getDepthMapSRV();
	void bindDSVAndSetNullRenderTarget();
};