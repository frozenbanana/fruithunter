#pragma once
#include "GlobalNamespaces.h"

#define MATRIX_VPT_SLOT 4
#define MATRIX_CAMERA_SLOT 1
#define MATRIX_SHADOWINFO_SLOT 6

#define SMAP_WIDTH (2048)
#define SHADOW_NEARPLANE 1.f
#define SHADOW_FARPLANE 300.f

class ShadowMapper {
private:
	//Transformation Variables
	XMINT2 m_smapSize = XMINT2(SMAP_WIDTH, SMAP_WIDTH);
	float m_nearPlane = SHADOW_NEARPLANE;
	float m_farPlane = SHADOW_FARPLANE;
	float2 m_size = float2(1.f, 1.f);
	float3 m_position = float3(0, 0, 0);
	float3 m_lightDirection;
	Matrix m_VPT;

	//Variables
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
	struct shadowInfo {
		float2 ShadowMapRes;
		float2 nearFarPlane;
		float4 toLight;
	} m_shadowInfo;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ShadowInfoBuffer;

	//Functions
	void createCameraBuffer();
	void createVPTBuffer();
	void createInfoBuffer();

	void updateViewMatrix();
	void updateProjMatrix();

	
public:
	void bindVPTBuffer();//Matrix that moves from localSpace to NDC
	void bindCameraBuffer(); // camera
	void bindShadowInfoBuffer();
	//Utility
	ShadowMapper();
	~ShadowMapper();
	//void initiate(UINT width, UINT height);
	void setDirection(float3 direction);
	void mapShadowToFrustum(vector<float3> frustum);

	void initiate();
	void bindShadowMap();
	void update(float3);
	void copyStaticToDynamic();

	void createShadowInfo();
	void bindInfoBuffer();

	vector<FrustumPlane> getFrustumPlanes() const;

	// Shadow functions
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getDepthMapSRV();
	void bindDSVAndSetNullRenderTarget();
	void bindDSVAndSetNullRenderTargetAndCopyStatic();
	void bindDSVAndSetNullRenderTargetStatic();
};