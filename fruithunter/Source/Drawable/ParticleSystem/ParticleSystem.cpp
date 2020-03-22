#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"
#include "VariableSyncer.h"
#include "TerrainManager.h"

ShaderSet ParticleSystem::m_shaderSetCircle;
ShaderSet ParticleSystem::m_shaderSetStar;

void ParticleSystem::load(ParticleSystem::PARTICLE_TYPE type) {
	m_type = type;
	m_description = Description(type);

	if (type != NONE) {
		//average amount of particles needed
		m_size = m_description.m_emitRate *
				 (m_description.m_timeAliveInterval.x + m_description.m_timeAliveInterval.y) * 0.5f;
		m_particles.resize(m_size);
		m_particleProperties.resize(m_size);
	}
}

ParticleSystem::ParticleSystem(ParticleSystem::PARTICLE_TYPE type) { 
	createShaders();
	load(type);
}

void ParticleSystem::setParticle(Description desc, size_t index) {
	Particle* part = &m_particles[index];
	part->setActiveValue(1.0f);
	// Position in world
	float3 spawnPos = m_spawnPoint;
	// Positon on half sphere
	float r = desc.m_spawnRadius + RandomFloat(desc.m_radiusInterval.x, desc.m_radiusInterval.y);
	float theta = RandomFloat(0.f, 3.1415f);
	float phi = RandomFloat(0.f, 3.1415f);
	float x = r * cos(theta) * sin(phi);
	float y = r * sin(theta) * sin(phi);
	float z = r * cos(phi);
	part->setPosition(spawnPos + float3(x, y, z));
	// Color
	float4 pickedColor = float4(1.0f, 1.f, 0.0f, 1.0f);
	int pick = rand() % 3;
	pickedColor = desc.m_color[pick];

	part->setColor(pickedColor);

	// Size
	float size = RandomFloat(desc.m_sizeInterval.x, desc.m_sizeInterval.y);
	part->setSize(size);

	ParticleProperty pp;
	// Time alive
	pp.m_lifeTime = RandomFloat(desc.m_timeAliveInterval.x, desc.m_timeAliveInterval.y);

	// Property (velo and lifetime)
	if (m_type == STARS) {
		pp.m_acceleration = float3(x, y, z) * -4.f / pp.m_lifeTime;
		pp.m_velocity = float3(x, y, z) * 4.f;
	}
	else {
		float randAccX =
			RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
		float randAccY =
			RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
		float randAccZ =
			RandomFloat(desc.m_accelerationOffsetInterval.x, desc.m_accelerationOffsetInterval.y);
		pp.m_acceleration = desc.m_acceleration + float3(randAccX, randAccY, randAccZ);
		float randVeloX =
			RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
		float randVeloY =
			RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
		float randVeloZ =
			RandomFloat(desc.m_velocityOffsetInterval.x, desc.m_velocityOffsetInterval.y);
		pp.m_velocity = desc.m_velocity + float3(randVeloX, randVeloY, randVeloZ);
	};
	pp.m_timeLeft = pp.m_lifeTime;

	m_particleProperties[index] = pp;
}

void ParticleSystem::stopEmiting() { m_isEmitting = false; }

void ParticleSystem::startEmiting() { m_isEmitting = true; }

void ParticleSystem::emit(size_t count) {
	m_isRunning = true;
	for (size_t i = 0; i < m_particles.size() && count > 0; i++) {
		if (m_particles[i].getActiveValue() == 0.f) {
			setParticle(m_description, i);
			count--;
		}
	}
}

void ParticleSystem::updateEmits(float dt) {
	m_emitTimer += dt;
	float rate = m_description.m_emitRate;
	if (rate > 0.f) {
		float emits = m_emitTimer * rate;
		size_t emitCount = (size_t)emits;
		if (emitCount > 0) {
			emit(emitCount);
			m_emitTimer -= (float)emitCount / rate;
		}
	}
}

void ParticleSystem::updateParticles(float dt) {
	size_t nrOfActive = 0;
	Terrain* terrain = TerrainManager::getInstance()->getTerrainFromPosition(m_spawnPoint);
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].getActiveValue() == 1.0f) {
			nrOfActive++;
			m_particleProperties[i].m_timeLeft -= dt;
			if (m_particleProperties[i].m_timeLeft <= 0.f) {
				// Inactivate particles when lifetime is over
				m_particles[i].setActiveValue(0.0f);
			}
			else {
				// get wind
				float3 wind(0, 0, 0);
				if (m_windState == WindState::Static)
					wind = m_staticWind;
				else if (m_windState == WindState::Dynamic && terrain != nullptr)
					wind = terrain->getWindFromPosition(m_particles[i].getPosition());
				//update velocity and position
				m_particleProperties[i].m_velocity += m_particleProperties[i].m_acceleration * dt;
				m_particles[i].update(dt, m_particleProperties[i].m_velocity + wind);
			}
		}
	}

	if (nrOfActive == 0) {
		m_isRunning = false;
	}
}

void ParticleSystem::update(float dt) { 
	if (m_isEmitting) {
		updateEmits(dt);
	}
	if (m_isRunning) {
		updateParticles(dt);
	}

}

void ParticleSystem::createShaders() {
	if (!m_shaderSetCircle.isLoaded() || !m_shaderSetStar.isLoaded()) {
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

		m_shaderSetCircle.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_circle.hlsl",
			inputLayout, 4);
		m_shaderSetStar.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_star.hlsl",
			inputLayout, 4);
	}
}

void ParticleSystem::resizeVertexBuffer(size_t size) {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();

	//  Buffer for particle data
	m_vertexBuffer.Reset();
	D3D11_BUFFER_DESC buffDesc;
	memset(&buffDesc, 0, sizeof(buffDesc));
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.Usage = D3D11_USAGE_DEFAULT;
	buffDesc.ByteWidth = (UINT)(sizeof(Particle) * size);

	HRESULT check = device->CreateBuffer(&buffDesc, NULL, m_vertexBuffer.GetAddressOf());

	if (FAILED(check))
		ErrorLogger::logError("(ParticleSystem) Failed creating vertex buffer!\n", check);
}

void ParticleSystem::bindVertexBuffer() {
	auto deviceContext = Renderer::getDeviceContext();

	deviceContext->UpdateSubresource(
		m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);

	UINT strides = sizeof(Particle);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void ParticleSystem::draw() {
	if (m_isRunning) {
		auto deviceContext = Renderer::getDeviceContext();

		//bind
		m_currentShaderSet->bindShadersAndLayout();
		bindVertexBuffer();

		//draw
		Renderer::getInstance()->enableAlphaBlending();
		deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
		Renderer::getInstance()->disableAlphaBlending();
	}
}

void ParticleSystem::drawNoAlpha() {
	if (m_isRunning) {
		auto deviceContext = Renderer::getDeviceContext();

		//bind
		m_currentShaderSet->bindShadersAndLayout();
		bindVertexBuffer();

		//draw
		deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
	}
}

void ParticleSystem::activateAllParticles() { emit(m_particles.size()); }

void ParticleSystem::setPosition(float3 position) { m_spawnPoint = position; }

void ParticleSystem::setWind(WindState state, float3 staticWind) { 
	m_staticWind = staticWind;
	m_windState = state;
}

float3 ParticleSystem::getPosition() const { return m_spawnPoint; }

ParticleSystem::PARTICLE_TYPE ParticleSystem::getType() const { return m_type; }

void ParticleSystem::setEmitRate(float emitRate) { m_description.m_emitRate = emitRate; }

void ParticleSystem::setColors(float4 colors[3]) {
	m_description.m_color[0] = colors[0];
	m_description.m_color[1] = colors[1];
	m_description.m_color[2] = colors[2];
}

void ParticleSystem::Description::load(ParticleSystem::PARTICLE_TYPE type) {
	switch (type) {
	case FOREST_BUBBLE:
		m_emitRate = 25.0f;						   // particles per sec
		m_acceleration = float3(0.0f, 0.0f, 0.0f); // float3(0.1f, 0.5f, -0.1f);
		m_accelerationOffsetInterval = float2(-0.15f, 0.15f);
		m_spawnRadius = 40.f;
		m_radiusInterval = float2(-35.2f, 0.2f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(-0.43f, 0.43f); // for x, y and z
		m_sizeInterval = float2(0.10f, 0.20f);
		m_timeAliveInterval = float2(4.f, 6.f);
		m_color[0] = float4(0.0f, 0.65f, 0.05f, 1.0f);
		m_color[1] = float4(0.0f, 0.65f, 0.4f, 1.0f);
		m_color[2] = float4(0.0f, 0.65f, 0.55f, 1.0f);
		break;
	case GROUND_DUST:
		m_emitRate = 25.0f; // particles per sec
		m_acceleration = float3(0.0f, 0.0f, 0.0f);
		m_accelerationOffsetInterval = float2(-0.15f, 0.15f);
		m_spawnRadius = 40.f;
		m_radiusInterval = float2(-35.2f, 0.2f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(-0.43f, 0.43f); // for x, y and z
		m_sizeInterval = float2(0.10f, 0.20f);
		m_timeAliveInterval = float2(2.5f, 3.5f);
		m_color[0] = float4(0.77f, 0.35f, 0.51f, 1.0f);
		m_color[1] = float4(0.71f, 0.55f, 0.31f, 1.0f);
		m_color[2] = float4(0.81f, 0.58f, 0.39f, 1.0f);
		break;
	case VULCANO_FIRE:
		m_emitRate = 70.0f; // particles per sec
		m_acceleration = float3(0.f, 0.75f, 0.f);
		m_accelerationOffsetInterval = float2(-1.20f, 1.20f);
		m_spawnRadius = 0.5f;
		m_radiusInterval = float2(-0.25f, 0.25f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(-0.5f, 0.5f); // for x, y and z
		m_sizeInterval = float2(0.48f, 2.8f);
		m_timeAliveInterval = float2(1.f, 1.5f);
		m_color[0] = float4(1.0f, 0.00f, 0.00f, 1.0f);
		m_color[1] = float4(0.71f, 0.35f, 0.0f, 1.0f);
		m_color[2] = float4(0.81f, 0.58f, 0.0f, 1.0f);
		break;
	case VULCANO_SMOKE:
		m_emitRate = 120.0f; // particles per sec
		m_acceleration = float3(0.f, -0.9f, 0.f);
		m_accelerationOffsetInterval = float2(-1.80f, 1.80f);
		m_spawnRadius = 0.75f;
		m_radiusInterval = float2(-0.7f, 0.0f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(-1.5f, 1.5f); // for x, y and z
		m_sizeInterval = float2(0.48f, 2.8f);
		m_timeAliveInterval = float2(2.f, 3.f);
		m_color[0] = float4(0.50f, 0.40f, 0.40f, 1.0f);
		m_color[1] = float4(0.30f, 0.30f, 0.30f, 1.0f);
		m_color[2] = float4(0.60f, 0.60f, 0.60f, 1.0f);
		break;
	case LAVA_BUBBLE:
		m_emitRate = 28.0f; // particles per sec
		m_acceleration = float3(0.3f, 0.3f, 0.3f);
		m_accelerationOffsetInterval = float2(-0.05f, 0.05f); // for x, y and z
		m_spawnRadius = 45.f;
		m_radiusInterval = float2(-35.f, 0.1f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(-0.75f, 0.75f); // for x, y and z
		m_sizeInterval = float2(0.11f, 0.22f);
		m_timeAliveInterval = float2(0.4f, 1.6f);
		m_color[0] = float4(0.70f, 0.20f, 0.20f, 1.0f);
		m_color[1] = float4(0.41f, 0.25f, 0.23f, 1.0f);
		m_color[2] = float4(0.51f, 0.34f, 0.17f, 1.0f);
		break;
	case ARROW_GLITTER:
		m_emitRate = 32.0f; // particles per sec
		m_acceleration = float3(0.0f, 0.0f, 0.0f);
		m_accelerationOffsetInterval = float2(0.0f, 0.0f);
		m_spawnRadius = 0.0f;
		m_radiusInterval = float2(0.002f, 0.003f);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(0.0f, 0.0f); // for x, y and z
		m_sizeInterval = float2(0.050f, 0.050f);
		m_timeAliveInterval = float2(0.5f, 0.5f);
		m_color[0] = float4(0.88f, 0.87f, 0.81f, 1.0f);
		m_color[1] = float4(0.92f, 0.89f, 0.83f, 1.0f);
		m_color[2] = float4(0.87f, 0.82f, 0.80f, 1.0f);
		break;
	case CONFETTI:
		m_emitRate = 12.0f; // particles per sec
		m_acceleration = float3(0.0f, 0.0f, 0.0f);
		m_accelerationOffsetInterval = float2(-0.2f, 0.2f);
		m_spawnRadius = 2.5f;
		m_radiusInterval = float2(-2.3f, 0.0f);
		m_velocity = float3(0.f, 0.0f, 0.f);
		m_velocityOffsetInterval = float2(-0.2f, 0.2f); // for x, y and z
		m_sizeInterval = float2(0.035f, 0.09f);
		m_timeAliveInterval = float2(1.5f, 3.0f);
		m_color[0] = float4(1.00f, 0.00f, 0.00f, 1.0f);
		m_color[1] = float4(0.00f, 1.00f, 0.00f, 1.0f);
		m_color[2] = float4(0.00f, 0.00f, 1.00f, 1.0f);
		break;
	case STARS:
		m_emitRate = -1.0f; // particles per sec
		m_acceleration = float3(0.0f, 0.0f, 0.0f);
		m_accelerationOffsetInterval = float2(0.0f, 0.0f);
		m_spawnRadius = 0.9f;
		m_radiusInterval = float2(0.0f, 0.0f);
		m_velocity = float3(0.f, 0.0f, 0.f);
		m_velocityOffsetInterval = float2(0.0f, 0.0f); // for x, y and z
		m_sizeInterval = float2(0.2f, .3f);
		m_timeAliveInterval = float2(0.6f, .65f);
		m_color[0] = float4(0.81f, 0.90f, 0.00f, 1.0f);
		m_color[1] = float4(0.90f, 0.81f, 0.00f, 1.0f);
		m_color[2] = float4(0.88f, 0.78f, 0.00f, 1.0f);
		break;
	default:
		m_emitRate = 0; // particles per sec
		m_acceleration = float3(0, 0, 0);
		m_spawnRadius = 0;
		m_radiusInterval = float2(0, 0);
		m_velocity = float3(0.f, 0.f, 0.f);
		m_velocityOffsetInterval = float2(0, 0); // for x, y and z
		m_sizeInterval = float2(0, 0);
		m_timeAliveInterval = float2(0, 0);
		break;
	}
}

ParticleSystem::Description::Description(ParticleSystem::PARTICLE_TYPE type) { load(type); }
