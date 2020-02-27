#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"

#define RAND_MAXIMUM 100.f;
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAXIMUM;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

ParticleSystem::ParticleSystem(size_t nrOfParticles) {

	m_particles.resize(nrOfParticles);
	if (!m_shaderSet.isLoaded()) {

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
			{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shaderSet.createShaders(L"", L"", L"", inputLayout, 2);
	}
}

void ParticleSystem::initialize() {
	float3 playerPos = float3(20.f, 0.0f, 20.f);
	float3 randomOffset;
	for (auto p : m_particles) {
		randomOffset = float3(RandomFloat(-1.f, 1.0f), 1.0f, RandomFloat(-1.f, 1.0f));
		p.setPosition(playerPos + randomOffset);
	}
	createBuffers();
}

void ParticleSystem::update(float dt) {
	for (auto p : m_particles) {
		p.update(dt, float3(1.0f, 0.0f, 0.0f));
	}
}

void ParticleSystem::createBuffers() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	HRESULT check;

	// vertex buffer
	m_vertexBuffer.Reset();
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = (UINT)m_nrOfParticles * sizeof(Particle);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = m_particles.data();

	check = device->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
	if (FAILED(check))
		ErrorLogger::logError(check, "Failed creating vertex buffer in Particle System class!\n");

	//  Buffer for particle data
	if (m_vertexBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC buffDesc;
		memset(&buffDesc, 0, sizeof(buffDesc));

		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		buffDesc.ByteWidth = sizeof(Particle) * MAX_PARTICLES;

		check = device->CreateBuffer(&buffDesc, nullptr, m_vertexBuffer.GetAddressOf());
		if (FAILED(check))
			ErrorLogger::logError(check, "Failed creating buffer in ParticleSystem class!\n");
	}
}

void ParticleSystem::bindBuffers() {
	auto deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Particle);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}


void ParticleSystem::draw() {
	auto deviceContext = Renderer::getDeviceContext();
	bindBuffers();
	deviceContext->Draw(m_particles.size(), 0);
}
