#include "Animated.h"

void Animated::bindMeshes() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();


	UINT strides[2] = { sizeof(Vertex) };
	UINT offset[] = { 0, 0 };
	for (int i = 0; i < m_nrOfMeshes; ++i) {
		deviceContext->IASetVertexBuffers(
			i, 1, m_meshes[i].getVertexBuffer().GetAddressOf(), strides, offset);
	}
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

// TODO: Able to use more than 2 meshes
void Animated::createInputAssembler() {
	D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
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
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Position", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TexCoordinate", 1, DXGI_FORMAT_R32G32_FLOAT, 1, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (!m_shaderObject_animation.isLoaded())
		m_shaderObject_animation.createShaders(L"VertexShader_model_animated.hlsl", nullptr,
			L"PixelShader_model.hlsl", inputLayout, 6);
}



Animated::Animated() {
	m_frameTimer = 0.0f;
	m_nrOfMeshes = 0;
	createInputAssembler();
}

Animated::~Animated() {}

void Animated::draw() {
	bindMeshes();
	m_shaderObject_animation.bindShadersAndLayout();
	// bind constantbuffer
	m_meshes[0].draw_withoutBinding();
}

bool Animated::load(std::string filename, int nrOfFrames, bool combineParts) {
	bool allClear = true;
	m_nrOfMeshes = nrOfFrames;
	m_meshes.empty();
	m_meshes.resize(nrOfFrames);
	for (int i = 0; i < nrOfFrames && allClear; ++i) {
		if (!m_meshes[i].load(filename + std::to_string(i))) {
			allClear = false;
			ErrorLogger::messageBox(0, "In Animated::load, failed to load mesh: " + filename +
										   " number: " + std::to_string(i));
		}
	}
	return allClear;
}
