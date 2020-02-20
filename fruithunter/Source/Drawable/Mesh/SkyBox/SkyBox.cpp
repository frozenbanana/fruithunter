#include "SkyBox.h"
#include <WICTextureLoader.h>

bool SkyBox::createResourceBuffer(string path, ID3D11ShaderResourceView** buffer) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	wstring wstr = s2ws(path);
	LPCWCHAR str = wstr.c_str();
	HRESULT hrA = DirectX::CreateWICTextureFromFile(device, deviceContext, str, nullptr, buffer);
	if (FAILED(hrA)) {
		ErrorLogger::messageBox(hrA, "Failed creating texturebuffer from texture\n" + path);
		return false;
	}
	return true;
}

bool SkyBox::createConstantBuffer() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(float4);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, m_buffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorLogger::messageBox(hr, "Failed creating constant buffer from texture\n");
		return false;
	}
	return true;
}

wstring SkyBox::s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void SkyBox::bindTextures(int oldSkybox, int newSkybox) {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	//// update buffer
	float4 dataBuffer = float4(m_dt,0,0,0);
	deviceContext->UpdateSubresource(m_buffer.Get(), 0, 0, &dataBuffer, 0, 0);

	// bind texture maps
	deviceContext->PSSetConstantBuffers(0, 1, m_buffer.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, m_textures[oldSkybox].GetAddressOf());
	deviceContext->PSSetShaderResources(2, 1, m_textures[newSkybox].GetAddressOf());
}

SkyBox::SkyBox() { 
	m_box.load("skybox");

	string filePath = "assets/Meshes/Textures/";
	string fileName = "ForestSkybox.jpg";
	string fileNamePath = filePath + fileName;

	for (int i = 0; i < 4; i++) {
		string fileNamePath = filePath + m_fileNames[i];
		createResourceBuffer(fileNamePath, m_textures[i].GetAddressOf());
	}
	m_dt = 0;
	createConstantBuffer();
	createShaders();
}

SkyBox::~SkyBox() {}

void SkyBox::draw() { 
	m_box.bindMaterial(0);
	m_box.bindMesh();
	m_shaderSkyBox.bindShadersAndLayout();
	Renderer::getDeviceContext()->Draw(m_box.getVertexCount(), 0);
	//m_box.draw();
}

void SkyBox::draw(int oldSkybox, int newSkybox) {
	bindTextures(oldSkybox, newSkybox);
	m_box.bindMesh();
	m_shaderSkyBox.bindShadersAndLayout();
	Renderer::getDeviceContext()->Draw(m_box.getVertexCount(), 0);
}


void SkyBox::createShaders() {
	D3D11_INPUT_ELEMENT_DESC inputLayout_skyBox[] = {
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

	m_shaderSkyBox.createShaders(
		L"VertexShaderSkyBox.hlsl", nullptr, L"PixelShaderSkyBox.hlsl", inputLayout_skyBox, 3);
}

void SkyBox::updateDelta(float dt) { 
	m_dt += dt;
	m_dt = clamp(m_dt, 0.0f, 1.0f);
}

void SkyBox::resetDelta() { m_dt = 1.0f - m_dt; }
