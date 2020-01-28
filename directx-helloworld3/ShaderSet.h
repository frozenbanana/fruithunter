#include "Renderer.hpp"
#include <d3dcompiler.h>

struct ShaderSet {
protected:
	bool loaded = false;
	ID3D11InputLayout* mVertexLayout = nullptr;

	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11GeometryShader* mGeometryShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3DBlob* createVertexShader(LPCWSTR filename);
	HRESULT createGeometryShader(LPCWSTR filename);
	HRESULT createFragmentShader(LPCWSTR filename);

public:
	bool isLoaded() const { return loaded; }
	bool createShaders(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0);
	void bindShadersAndLayout();
	void release();
	ShaderSet(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0);
	ShaderSet();
	~ShaderSet();
};