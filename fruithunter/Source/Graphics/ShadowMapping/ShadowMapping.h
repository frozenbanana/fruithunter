#pragma once
#include "GlobalNamespaces.h"

#define SMAP_SIZE 2048.f

struct shadowInfo {
	float2 ShadowMapRes;
	float2 nearFarPlane;
	float4 toLight;
};

class ShadowMapper {
private:
	//Variables
	XMINT2 m_shadowPortSize = XMINT2(static_cast<int>(SMAP_SIZE), static_cast<int>(SMAP_SIZE));
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
	shadowInfo m_shadowInfo;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ShadowInfoBuffer;

	//Functions
	void createCameraBuffer();
	void createVPTBuffer();
	void createVPTMatrix();
	void createInfoBuffer();
	
	
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

	void createShadowInfo();
	void bindInfoBuffer();

	// Shadow functions
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getDepthMapSRV();
	void bindDSVAndSetNullRenderTarget();
	void bindDSVAndSetNullRenderTargetAndCopyStatic();
	void bindDSVAndSetNullRenderTargetStatic();
};