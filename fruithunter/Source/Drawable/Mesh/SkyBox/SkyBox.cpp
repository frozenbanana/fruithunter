#include "SkyBox.h"
#include "VariableSyncer.h"
#include <WICTextureLoader.h>
#include "Renderer.h"
#include "ErrorLogger.h"

ShaderSet SkyBox::m_shaderSkyBox;

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

	HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, m_interpolationBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorLogger::messageBox(hr, "Failed creating constant buffer from texture\n");
		return false;
	}
	return true;
}

bool SkyBox::bindTextures() {
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	// update and bind interpolation buffer
	float4 dataBuffer = float4(m_interpolation,0,0,0);
	deviceContext->UpdateSubresource(m_interpolationBuffer.Get(), 0, 0, &dataBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, m_interpolationBuffer.GetAddressOf());

	// bind texture maps
	if (m_textures[m_oldLight].get() != nullptr && m_textures[m_newLight].get() != nullptr) {
		deviceContext->PSSetShaderResources(1, 1, m_textures[m_oldLight]->getSRV().GetAddressOf());
		deviceContext->PSSetShaderResources(2, 1, m_textures[m_newLight]->getSRV().GetAddressOf());
		return true;
	}
	else {
		ErrorLogger::logWarning("(SkyBox) Failed binding textures!");
		return false;//failed binding
	}
}

bool SkyBox::bindTexture(AreaTag tag) { 
	ID3D11Device* device = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	// update and bind interpolation buffer
	float4 dataBuffer = float4(m_interpolation, 0, 0, 0);
	deviceContext->UpdateSubresource(m_interpolationBuffer.Get(), 0, 0, &dataBuffer, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, m_interpolationBuffer.GetAddressOf());

	// bind texture maps
	if (m_textures[tag].get() != nullptr) {
		deviceContext->PSSetShaderResources(1, 1, m_textures[tag]->getSRV().GetAddressOf());
		deviceContext->PSSetShaderResources(2, 1, m_textures[tag]->getSRV().GetAddressOf());
		return true;
	}
	else {
		ErrorLogger::logWarning("(SkyBox) Failed binding texture!");
		return false; // failed binding
	}
}

void SkyBox::load(
	string fileNames[AreaTag::NR_OF_AREAS], lightInfo lightInfos[AreaTag::NR_OF_AREAS]) {
	
	for (size_t i = 0; i < AreaTag::NR_OF_AREAS; i++) {
		//set texture
		m_textures[i] = TextureRepository::get(fileNames[i], TextureRepository::Type::type_texture);
		//set light info
		m_lightInfo[i] = lightInfos[i];
	}

}

void SkyBox::loadStandard() {
	string fileNames[AreaTag::NR_OF_AREAS];
	fileNames[AreaTag::Forest] = "ForestSkybox.jpg";
	fileNames[AreaTag::Plains] = "PlainsSkybox.jpg";
	fileNames[AreaTag::Desert] = "DesertSkybox.jpg";
	fileNames[AreaTag::Volcano] = "VolcanoSkybox.jpg";
	
	lightInfo info[AreaTag::NR_OF_AREAS];
	info[AreaTag::Forest] = { 
		float4(0.3f, 0.4f, 0.6f, 1.0f), 
		float4(0.19f, 0.32f, 1.0f, 1.0f),
		float4(0.4f, 0.5f, 1.0f, 1.0f)
	};
	info[AreaTag::Plains] = { 
		float4(0.7f, 0.7f, 0.7f, 1.0f),
		float4(0.8f, 0.9f, 0.7f, 1.0f),
		float4(1.0f, 1.0f, 1.0f, 1.0f)
	};
	info[AreaTag::Desert] = {
		float4(0.75f, 0.6f, 0.28f, 1.0f),
		float4(0.7f, 0.59f, 0.2f, 1.0f),
		float4(1.0f, 0.9f, 0.65f, 1.0f)
	};
	info[AreaTag::Volcano] = { 
		float4(0.6f, 0.2f, 0.0f, 1.0f), 
		float4(0.7f, 0.2f, 0.1f, 1.0f),
		float4(1.0f, 0.2f, 0.1f, 1.0f) 
	};
	load(fileNames, info);
}

SkyBox::SkyBox() { 
	m_box.load("skybox");

	createLightBuffer();
	createConstantBuffer();
	createShaders();

	loadStandard();
}

SkyBox::~SkyBox() {}

void SkyBox::draw() {
	if (bindTextures()) {
		//sucessfully binded buffers
		m_box.bindMesh();
		m_shaderSkyBox.bindShadersAndLayout();
		Renderer::draw(m_box.getVertexCount(), 0);
	}
}

void SkyBox::draw(AreaTag tag) {
	if (bindTexture(tag)) {
		// sucessfully binded buffers
		updateLightInfo(tag);
		m_box.bindMesh();
		m_shaderSkyBox.bindShadersAndLayout();
		Renderer::draw(m_box.getVertexCount(), 0);
	}
}


void SkyBox::createShaders() {
	if (!m_shaderSkyBox.isLoaded()) {
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
}

void SkyBox::updateLightInfo() {
	m_interpolatedLightInfo.ambient = (m_lightInfo[m_oldLight].ambient * (1 - m_interpolation)) +
									  (m_lightInfo[m_newLight].ambient * m_interpolation);
	m_interpolatedLightInfo.diffuse = (m_lightInfo[m_oldLight].diffuse * (1 - m_interpolation)) +
									  (m_lightInfo[m_newLight].diffuse * m_interpolation);
	m_interpolatedLightInfo.specular = (m_lightInfo[m_oldLight].specular * (1 - m_interpolation)) +
									   (m_lightInfo[m_newLight].specular * m_interpolation);

	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, 0, &m_interpolatedLightInfo, 0, 0);
}

void SkyBox::updateLightInfo(AreaTag tag) {
	m_interpolatedLightInfo = m_lightInfo[tag];

	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	deviceContext->UpdateSubresource(m_lightBuffer.Get(), 0, 0, &m_interpolatedLightInfo, 0, 0);
}

void SkyBox::update(float dt) { 
	m_interpolation = Clamp(m_interpolation + dt, 0.f, 1.f);
}

bool SkyBox::switchLight(AreaTag tag) { 
	if (tag == m_newLight)
		return false;
	if (tag == m_oldLight) {
		m_interpolation = 1.0f - m_interpolation;
	}
	else {
		m_interpolation = 0;
	}
	m_oldLight = m_newLight;
	m_newLight = tag;
	return true;
}

bool SkyBox::createLightBuffer() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(lightInfo);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, m_lightBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorLogger::messageBox(hr, "Failed creating constant buffer for light\n");
		return false;
	}
	return true;
}

void SkyBox::bindLightBuffer() {
	updateLightInfo();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	deviceContext->PSSetConstantBuffers(5, 1, m_lightBuffer.GetAddressOf());
}

void SkyBox::reset() {
	m_oldLight = AreaTag::Plains;
	m_newLight = AreaTag::Plains;
	m_interpolation = 1.f;
	loadStandard();
}