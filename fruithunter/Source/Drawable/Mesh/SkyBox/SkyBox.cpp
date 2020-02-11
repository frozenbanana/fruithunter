#include "SkyBox.h"
#include <WICTextureLoader.h>

SkyBox::SkyBox() { 
	m_box.load("skybox");
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
