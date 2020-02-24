#pragma once
#include "Camera.h"
class ShadowMapper : public Camera {
private:
	//Variables
	XMINT2 m_shadowPortSize;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowSRV;
	D3D11_VIEWPORT m_shadowPort;

	//Functions

public:
	//Utility
	ShadowMapper();
	~ShadowMapper();
	void initiate(UINT width, UINT height);
	void initiate();

	// Shadow functions
	void createDepthBuffer(/*DXGI_SWAP_CHAIN_DESC& scd*/);
	//void update();
};