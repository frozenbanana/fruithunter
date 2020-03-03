#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"
#include "VariableSyncer.h"

ShaderSet ParticleSystem::m_shaderSet;

ParticleSystem::ParticleSystem(ParticleSystem::PARTICLE_TYPE type) {

	m_description = make_shared<Description>(type);

	if (type != NONE) {
		string sid = "ParticleSystem" + to_string((long)this) + "-" + to_string(type) + ".txt";
		VariableSyncer::getInstance()->create(sid);
		VariableSyncer::getInstance()->bind(sid,
			"ParticleCount:i&emitRate:f&acceleration:v3&spawnRadius:f&radiusInterval:v2&velocity:"
			"v3&velocityOffsetInterval:v2&sizeInterval:v2&timeAliveInteral:v2&color[0]:v3&color[1]:"
			"v3&color[2]:v3",
			m_description.get());

		if (m_description->m_nrOfParticles > MAX_PARTICLES) {
			ErrorLogger::logWarning(NULL, "Particle System is not allowed " +
											  to_string(m_description->m_nrOfParticles) +
											  ", limit is " + to_string(MAX_PARTICLES));
		}
		m_particles.resize(min(m_description->m_nrOfParticles, MAX_PARTICLES));
		m_particleProperties.resize(min(m_description->m_nrOfParticles, MAX_PARTICLES));

		initialize();
	}
	// random seed
	srand((unsigned int)time(NULL));
}

void ParticleSystem::initialize() {
	// Timer
	m_timePassed = 0.0f;
	m_emitTimer = 0.f;
	float3 spawnPos = m_spawnPoint;
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particles[i].setIsActive(0.f);
	}

	// Buffer
	createBuffers();

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
			{ "IsActive", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shaderSet.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem.hlsl", inputLayout,
			4);
	}
	setActive();
}

void ParticleSystem::setParticle(Description desc, size_t index) {
	Particle* part = &m_particles[index];
	ParticleProperty* partProp = &m_particleProperties[index];
	part->setIsActive(1.0f);
	// Position in world
	float3 spawnPos = m_spawnPoint;
	// Positon on half sphere
	float r = desc.m_spawnRadius + RandomFloat(desc.m_radiusInterval.x, desc.m_radiusInterval.y);
	float theta = RandomFloat(0.f, 3.1415f);
	float phi = RandomFloat(0, 3.1415f);
	float x = r * cos(theta) * sin(phi);
	float y = r * sin(theta) * sin(phi);
	float z = r * cos(phi);
	part->setPosition(spawnPos + float3(x, y, z));

	// Color
	float3 pickedColor = float3(1.0f, 1.f, 0.0f);
	int pick = rand() % 3; // 0..1
	pickedColor = desc.m_color[pick];

	part->setColor(float4(pickedColor.x, pickedColor.y, pickedColor.z, 1.0f));

	// Size
	float size = RandomFloat(desc.m_sizeInterval.x, desc.m_sizeInterval.y);
	part->setSize(size);

	// Property (velo and lifetime)
	ParticleProperty pp;
	float randAccX =
		RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
	float randAccY =
		RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
	float randAccZ =
		RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
	pp.m_acceleration = desc.m_acceleration + float3(randAccX, randAccY, randAccZ);
	float randVeloX = RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
	float randVeloY = RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
	float randVeloZ = RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
	pp.m_velocity = desc.m_velocity + float3(randVeloX, randVeloY, randVeloZ);
	pp.m_lifeTime = RandomFloat(desc.m_timeAliveInterval.x, desc.m_timeAliveInterval.y);
	pp.m_timeLeft = pp.m_lifeTime;
	*partProp = pp;
}


void ParticleSystem::activateParticle() {
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].getIsActive() == 0.f) {
			setParticle(*m_description, i);
			break;
		}
	}
}

void ParticleSystem::update(float dt, float3 wind) {
	if (m_isActive) {
		m_timePassed += dt;
		m_emitTimer += dt;
		float rate = m_description->m_emitRate;
		float emits = m_emitTimer * rate;
		size_t emitCount = (size_t)emits;
		if (emits > 1.0f) {
			for (size_t i = 0; i < emitCount; i++)
				activateParticle();

			m_emitTimer -= (1.f / rate) * emitCount;
		}
		for (size_t i = 0; i < m_particles.size(); i++) {
			if (m_particles[i].getIsActive() == 1.0f) {
				m_particleProperties[i].m_velocity += m_particleProperties[i].m_acceleration * dt;
				m_particleProperties[i].m_timeLeft -= dt;
				m_particles[i].update(dt, m_particleProperties[i].m_velocity + wind);
				// Controlling size
				// float sizeFactor =
				//	m_particleProperties[i].m_timeLeft / m_particleProperties[i].m_lifeTime;
				// m_particles[i].setSize(m_particles[i].getSize() * sizeFactor * sizeFactor);
				// Inactivate particles when lifetime is over
				if (m_particleProperties[i].m_timeLeft <= 0.f) {
					m_particles[i].setIsActive(0.0f);
				}
			}
		}

		Renderer::getDeviceContext()->UpdateSubresource(
			m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);
	}
}

void ParticleSystem::createBuffers() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	//  Buffer for particle data
	m_vertexBuffer.Reset();

	D3D11_BUFFER_DESC buffDesc;
	memset(&buffDesc, 0, sizeof(buffDesc));

	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	buffDesc.ByteWidth = (UINT)(sizeof(Particle) * MAX_PARTICLES);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = m_particles.data();

	HRESULT check = device->CreateBuffer(
		&buffDesc, 
		&data, 
		m_vertexBuffer.GetAddressOf());

	if (FAILED(check))
		ErrorLogger::logError(check, "Failed creating buffer in ParticleSystem class!\n");
}

void ParticleSystem::bindBuffers() {
	auto deviceContext = Renderer::getDeviceContext();

	UINT strides = sizeof(Particle);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}


void ParticleSystem::draw() {
	if (m_isActive) {
		auto deviceContext = Renderer::getDeviceContext();
		m_shaderSet.bindShadersAndLayout();
		bindBuffers();
		Renderer::getInstance()->enableAlphaBlending();
		deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
		Renderer::getInstance()->disableAlphaBlending();
	}
}

void ParticleSystem::setActive() { m_isActive = true; }

void ParticleSystem::setInActive() { m_isActive = false; }

bool ParticleSystem::getIsActive() { return m_isActive; }

void ParticleSystem::setPosition(float3 position) { m_spawnPoint = position; }

float3 ParticleSystem::getPosition() const { return m_spawnPoint; }

void ParticleSystem::setDesciption(Description newDescription) { *m_description = newDescription; }
