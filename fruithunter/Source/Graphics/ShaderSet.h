#pragma once
#include "Renderer.h"
#include "ErrorLogger.h"

class ShaderSet {
protected:
	bool m_loaded = false;
	ID3D11InputLayout* m_vertexLayout = nullptr;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11GeometryShader* m_geometryShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;

	ID3DBlob* createVertexShader(LPCWSTR filename);
	HRESULT createGeometryShader(LPCWSTR filename);
	HRESULT createFragmentShader(LPCWSTR filename);

	void release();
	std::string convertLPCWSTR(LPCWSTR LPCWstring);

public:
	bool isLoaded() const;
	bool createShaders(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0);
	void bindShadersAndLayout();

	ShaderSet(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0);
	ShaderSet();
	~ShaderSet();
};