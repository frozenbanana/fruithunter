#include "ShaderSet.h"

ID3DBlob* ShaderSet::createVertexShader(LPCWSTR filename) {

	ID3D11Device* device = Renderer::getDevice();


	ID3DBlob* pVS = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	HRESULT result = D3DCompileFromFile(filename, // filename
		nullptr,								  // optional macros
		nullptr,								  // optional include files
		"main",									  // entry point
		"vs_5_0",								  // shader model (target)
		D3DCOMPILE_DEBUG,						  // shader compile options (DEBUGGING)
		0,										  // IGNORE...DEPRECATED.
		&pVS,									  // double pointer to ID3DBlob
		&errorBlob								  // pointer for Error Blob messages.
	);
	if (FAILED(result)) {
		// compilation failed
		ErrorLogger::messageBox(
			result, "Failed compiling vertex shader with name: " + convertLPCWSTR(filename) +
						"\nPotential solution is to set shader function name to 'main'");
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			// release "reference" to errorBlob interface object
			errorBlob->Release();
		}
		if (pVS)
			pVS->Release();
		// return result;
	}

	result = device->CreateVertexShader(
		pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result))
		ErrorLogger::messageBox(
			result, "Failed creating vertex shader with name: " + convertLPCWSTR(filename));

	return pVS;
}

HRESULT ShaderSet::createGeometryShader(LPCWSTR filename) {
	ID3D11Device* device = Renderer::getDevice();

	ID3DBlob* pGS = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(
		filename, nullptr, nullptr, "GS_main", "gs_5_0", D3DCOMPILE_DEBUG, 0, &pGS, &errorBlob);

	if (FAILED(result)) {
		// compilation failed
		ErrorLogger::messageBox(
			result, "Failed compiling geometry shader with name: " + convertLPCWSTR(filename) +
						"\nPotential solution is to set shader function name to 'main'");
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			// release "reference" to errorBlob interface object
			errorBlob->Release();
		}
		if (pGS)
			pGS->Release();
		return result;
	}

	result = device->CreateGeometryShader(
		pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &m_geometryShader);
	if (FAILED(result))
		ErrorLogger::messageBox(
			result, "Failed creating geometry shader with name: " + convertLPCWSTR(filename));

	pGS->Release();
}

HRESULT ShaderSet::createFragmentShader(LPCWSTR filename) {
	ID3D11Device* device = Renderer::getDevice();

	ID3DBlob* pPS = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(filename, // filename
		nullptr,								  // optional macros
		nullptr,								  // optional include files
		"main",									  // entry point
		"ps_5_0",								  // shader model (target)
		D3DCOMPILE_DEBUG,						  // shader compile options
		0,										  // effect compile options
		&pPS,									  // double pointer to ID3DBlob
		&errorBlob								  // pointer for Error Blob messages.
	);

	if (FAILED(result)) {
		// compilation failed
		ErrorLogger::messageBox(
			result, "Failed compiling fragment shader with name: " + convertLPCWSTR(filename) +
						"\nPotential solution is to set shader function name to 'main'");
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			// release "reference" to errorBlob interface object
			errorBlob->Release();
		}
		if (pPS)
			pPS->Release();
		return result;
	}

	result = device->CreatePixelShader(
		pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result))
		ErrorLogger::messageBox(
			result, "Failed creating fragment shader with name: " + convertLPCWSTR(filename));

	// we do not need anymore this COM object, so we release it.
	pPS->Release();
}

bool ShaderSet::isLoaded() const { return m_loaded; }

bool ShaderSet::createShaders(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName,
	D3D11_INPUT_ELEMENT_DESC* inputDesc, int inputDescCount) {

	ID3D11Device* device = Renderer::getDevice();

	// comppile & create vertex shader
	m_loaded = true;
	bool check = true;
	ID3DBlob* pVS = createVertexShader(vertexName);

	// create input layout
	HRESULT res;
	if (inputDesc == nullptr) {
		// pre made ElementDescription
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
		res = device->CreateInputLayout(standardInputDesc, ARRAYSIZE(standardInputDesc),
			pVS->GetBufferPointer(), pVS->GetBufferSize(), &m_vertexLayout);
		if (FAILED(res)) {
			ErrorLogger::messageBox(
				res, "Failed creating InputLayout\n" + convertLPCWSTR(vertexName) + "\n" +
						 convertLPCWSTR(geometryName) + "\n" + convertLPCWSTR(fragmentName));
			check = false;
		}
	}
	else {
		// custom ElementDescription
		auto test = inputDesc;
		res = device->CreateInputLayout(inputDesc, inputDescCount, pVS->GetBufferPointer(),
			pVS->GetBufferSize(), &m_vertexLayout);
		if (FAILED(res)) {
			ErrorLogger::messageBox(
				res, "Failed creating InputLayout\n" + convertLPCWSTR(vertexName) + "\n" +
						 convertLPCWSTR(geometryName) + "\n" + convertLPCWSTR(fragmentName));
			check = false;
		}
	}
	pVS->Release();

	// compile&create geometry and fragment shader
	if (geometryName != nullptr && FAILED(createGeometryShader(geometryName)))
		check = false;
	if (fragmentName != nullptr && FAILED(createFragmentShader(fragmentName)))
		check = false;
	return check;
}

void ShaderSet::bindShadersAndLayout() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->HSSetShader(nullptr, nullptr, 0);
	deviceContext->DSSetShader(nullptr, nullptr, 0);
	deviceContext->GSSetShader(m_geometryShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->IASetInputLayout(m_vertexLayout);
}

void ShaderSet::release() {
	if (m_vertexShader != nullptr)
		m_vertexShader->Release();
	if (m_geometryShader != nullptr)
		m_geometryShader->Release();
	if (m_pixelShader != nullptr)
		m_pixelShader->Release();
	if (m_vertexLayout != nullptr)
		m_vertexLayout->Release();
}

std::string ShaderSet::convertLPCWSTR(LPCWSTR LPCWstring) {
	if (LPCWstring == nullptr)
		return "";
	std::wstring wstr(LPCWstring);
	return std::string(wstr.begin(), wstr.end());
}

ShaderSet::ShaderSet(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName,
	D3D11_INPUT_ELEMENT_DESC* inputDesc, int inputDescCount) {
	createShaders(vertexName, geometryName, fragmentName, inputDesc, inputDescCount);
}

ShaderSet::ShaderSet() {}

ShaderSet::~ShaderSet() { release(); }
