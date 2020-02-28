#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"

float RandomFloat(float low = 0.f, float high = 1.f) {
	float randomCoefficent = (float)(rand() % (int)100.f) / 100.f; // normalize
	return low + randomCoefficent * (high - low);
}

ShaderSet ParticleSystem::m_shaderSet;

ParticleSystem::ParticleSystem(size_t nrOfParticles) {
	if (nrOfParticles > MAX_PARTICLES) {
		ErrorLogger::logWarning(NULL, "Particle System is not allowed " + to_string(nrOfParticles) +
										  ", limit is " + to_string(MAX_PARTICLES));
	}
	m_particles.resize(min(nrOfParticles, MAX_PARTICLES));

	initialize();
	// random seed
	srand((unsigned int)time(NULL));

	// Timer
	m_timePassed = 0.0f;
	// shader
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
			{ "Size", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shaderSet.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem.hlsl", inputLayout,
			3);
	}
}

void ParticleSystem::initialize() {
	// float3 playerPos = float3(20.f, 0.0f, 20.f);
	float3 randomOffset;
	float4 randomColor;
	float randomSize;

	for (size_t i = 0; i < m_particles.size(); i++) {
		randomOffset = float3(RandomFloat(-4.f, 4.0f), RandomFloat(-4.f, 4.0f), 0.0f);
		randomColor = float4(RandomFloat(), RandomFloat(), RandomFloat(), 1.f);
		randomSize = RandomFloat(0.05f, 0.3f);
		m_particles[i].setPosition(randomOffset);
		m_particles[i].setColor(randomColor);
		m_particles[i].setSize(randomSize);
	}

	createBuffers();
}

void ParticleSystem::update(float dt) {
	m_timePassed += dt;
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particles[i].update(
			dt, float3(sin(m_timePassed + float(i)), cos(m_timePassed + float(i % 4)), 0.0f));
	}

	Renderer::getDeviceContext()->UpdateSubresource(
		m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);
}

void ParticleSystem::createBuffers() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	//  Buffer for particle data
	if (m_vertexBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC buffDesc;
		memset(&buffDesc, 0, sizeof(buffDesc));

		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		buffDesc.ByteWidth = sizeof(Particle) * MAX_PARTICLES;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_particles.data();

		HRESULT check = device->CreateBuffer(&buffDesc, &data, m_vertexBuffer.GetAddressOf());
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
	m_shaderSet.bindShadersAndLayout();
	bindBuffers();
	deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
}
