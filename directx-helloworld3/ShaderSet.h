#include <d3d11.h>
#include <d3dcompiler.h>

struct ShaderSet {
protected:
	bool loaded = false;
	ID3D11InputLayout* mVertexLayout = nullptr;

	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11GeometryShader* mGeometryShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3DBlob* createVertexShader(LPCWSTR filename) {
		ID3DBlob* pVS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		HRESULT result = D3DCompileFromFile(filename, // filename
			nullptr,								  // optional macros
			nullptr,								  // optional include files
			"VS_main",								  // entry point
			"vs_5_0",								  // shader model (target)
			D3DCOMPILE_DEBUG,						  // shader compile options (DEBUGGING)
			0,										  // IGNORE...DEPRECATED.
			&pVS,									  // double pointer to ID3DBlob
			&errorBlob								  // pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pVS)
				pVS->Release();
			// return result;
		}

		result = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &mVertexShader);
		return pVS;
	}
	HRESULT createGeometryShader(LPCWSTR filename) {
		ID3DBlob* pGS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT result =
			D3DCompileFromFile(filename, nullptr, nullptr, "GS_main", "gs_5_0", D3DCOMPILE_DEBUG, 0, &pGS, &errorBlob);

		// compilation failed?
		if (FAILED(result)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pGS)
				pGS->Release();
			return result;
		}

		gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &mGeometryShader);

		pGS->Release();
	}
	HRESULT createFragmentShader(LPCWSTR filename) {
		ID3DBlob* pPS = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT result = D3DCompileFromFile(filename, // filename
			nullptr,								  // optional macros
			nullptr,								  // optional include files
			"PS_main",								  // entry point
			"ps_5_0",								  // shader model (target)
			D3DCOMPILE_DEBUG,						  // shader compile options
			0,										  // effect compile options
			&pPS,									  // double pointer to ID3DBlob
			&errorBlob								  // pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pPS)
				pPS->Release();
			return result;
		}

		gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &mPixelShader);
		// we do not need anymore this COM object, so we release it.
		pPS->Release();
	}

public:
	bool isLoaded() const { return loaded; }
	bool createShaders(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName,
		D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0) {
		loaded = true;
		bool check = true;
		ID3DBlob* pVS = createVertexShader(vertexName);
		HRESULT res;
		if (inputDesc == nullptr) {
			D3D11_INPUT_ELEMENT_DESC standardInputDesc[] = {
				{
					"Position",					 // "semantic" name in shader
					0,							 // "semantic" index (not used)
					DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
					0,							 // input slot
					0,							 // offset of first element
					D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
					0							 // used for INSTANCING (ignore)
				},
				{ "TexCoordinate", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			res = gDevice->CreateInputLayout(standardInputDesc, ARRAYSIZE(standardInputDesc), pVS->GetBufferPointer(),
				pVS->GetBufferSize(), &mVertexLayout);
			if (FAILED(res))
				check = false;
		}
		else {
			res = gDevice->CreateInputLayout(
				inputDesc, inputDescCount, pVS->GetBufferPointer(), pVS->GetBufferSize(), &mVertexLayout);
			if (FAILED(res))
				check = false;
		}
		pVS->Release();
		if (geometryName != nullptr && FAILED(createGeometryShader(geometryName)))
			check = false;
		if (fragmentName != nullptr && FAILED(createFragmentShader(fragmentName)))
			check = false;
		return check;
	}
	void bindShadersAndLayout() {
		gDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		gDeviceContext->HSSetShader(nullptr, nullptr, 0);
		gDeviceContext->DSSetShader(nullptr, nullptr, 0);
		gDeviceContext->GSSetShader(mGeometryShader, nullptr, 0);
		gDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

		gDeviceContext->IASetInputLayout(mVertexLayout);
	}
	void release() {
		if (mVertexShader != nullptr)
			mVertexShader->Release();
		if (mGeometryShader != nullptr)
			mGeometryShader->Release();
		if (mPixelShader != nullptr)
			mPixelShader->Release();
		if (mVertexLayout != nullptr)
			mVertexLayout->Release();
	}
	ShaderSet(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName,
		D3D11_INPUT_ELEMENT_DESC* inputDesc = nullptr, int inputDescCount = 0) {
		createShaders(vertexName, geometryName, fragmentName, inputDesc, inputDescCount);
	}
	ShaderSet() {}
	~ShaderSet() { release(); }
};