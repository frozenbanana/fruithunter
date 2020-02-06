#include "Animated.hpp"

void Animated::bindMeshes() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	const int t = 3;
	UINT strides[NR_OF_MESHES_TO_SEND] = { sizeof(Vertex) };
	UINT offset[NR_OF_MESHES_TO_SEND] = { 0 };
	for (int i = 0; i < 2; ++i)
		deviceContext->IASetVertexBuffers(
			i, 1, m_meshes[m_frameTargets[i]].getVertexBuffer().GetAddressOf(), strides, offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

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
	};

	int nrOfMeshesToSend = NR_OF_MESHES_TO_SEND; // for now 2. May change later if we want to
												 // interpolate between more meshes at the same time

	unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> inputLayoutTotal;
	inputLayoutTotal = make_unique<D3D11_INPUT_ELEMENT_DESC[]>((size_t)nrOfMeshesToSend * 3);
	for (int iVertex = 0; iVertex < nrOfMeshesToSend; ++iVertex) {
		for (int i = 0; i < 3; ++i) {
			inputLayoutTotal[(size_t)iVertex * 3 + i] = inputLayout[i];
			inputLayoutTotal[(size_t)iVertex * 3 + i].SemanticIndex = iVertex;
			inputLayoutTotal[(size_t)iVertex * 3 + i].InputSlot = iVertex;
		}
	}

	if (!m_shaderObject_animation.isLoaded())
		m_shaderObject_animation.createShaders(L"VertexShader_model_animated.hlsl", nullptr,
			L"PixelShader_model.hlsl", inputLayoutTotal.get(), nrOfMeshesToSend * 3);
}

void Animated::createAnimationConstantBuffer() {
	ID3D11Device* gDevice = Renderer::getDevice();
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();

	HRESULT check;

	// Animation buffer
	m_animationBuffer.Reset();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(float) * 4;

	D3D11_SUBRESOURCE_DATA data;
	float zero = 0.0f;
	data.pSysMem = &zero;

	check = gDevice->CreateBuffer(&bufferDesc, &data, m_animationBuffer.GetAddressOf());
	if (FAILED(check))
		ErrorLogger::logError(check, "Failed creating animation buffer in Animated class!\n");
}

void Animated::bindConstantBuffer() {
	// bind buffer to pipeline
	Renderer::getDeviceContext()->VSSetConstantBuffers(
		ANIMATION_BUFFER_SLOT, 1, m_animationBuffer.GetAddressOf());
}

Animated::Animated() {
	m_frameTimer = 0.0f;
	m_nrOfMeshes = 0;
	m_frameTargets[0] = 0;
	m_frameTargets[1] = 1;
	createAnimationConstantBuffer();
}

Animated::~Animated() {}

void Animated::setFrameTargets(int first, int second) {
	m_frameTargets[0] = first;
	m_frameTargets[1] = second;
}

void Animated::setFrameTimer(float timer) { m_frameTimer = timer; }

float Animated::getFrameTimer() { return m_frameTimer; }

// Default update to update self
void Animated::update(float dt) {
	if (m_frameTimer > 1)
		dt *= 3;
	m_frameTimer = (float)fmod(m_frameTimer + dt, m_nrOfMeshes - 1);
	if (m_frameTimer > 1) {
		setFrameTargets(1, 2);
	}
	else
		setFrameTargets(0, 1);

	// Update buffer
	float4 data = { fmod(m_frameTimer, 1.0f), 0, 0, 0 };
	Renderer::getDeviceContext()->UpdateSubresource(m_animationBuffer.Get(), 0, 0, &data, 0, 0);
}

// Used by classes with specific frameTime functions
void Animated::updateSpecific(float frameTime) {
	// Update buffer
	m_frameTimer = frameTime;
	float4 data = { fmod(m_frameTimer, 1.0f), 0, 0, 0 };
	Renderer::getDeviceContext()->UpdateSubresource(m_animationBuffer.Get(), 0, 0, &data, 0, 0);
}

void Animated::draw() {
	bindMeshes();
	bindConstantBuffer();
	m_shaderObject_animation.bindShadersAndLayout();

	m_meshes[0].draw_withoutBinding();
}

bool Animated::load(std::string filename, int nrOfFrames, bool combineParts) {
	bool allClear = true;
	m_nrOfMeshes = nrOfFrames;
	m_meshes.clear();
	m_meshes.resize(nrOfFrames);
	for (int i = 0; i < nrOfFrames && allClear; ++i) {
		if (!m_meshes[i].load(filename + "_00000" + std::to_string(i))) {
			allClear = false;
			ErrorLogger::messageBox(0, "In Animated::load, failed to load mesh: " + filename +
										   " number: " + std::to_string(i));
		}
	}
	createInputAssembler();
	return allClear;
}
