#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"
#include "VariableSyncer.h"

ShaderSet ParticleSystem::m_shaderSetCircle;
ShaderSet ParticleSystem::m_shaderSetStar;
Microsoft::WRL::ComPtr<ID3D11Buffer> ParticleSystem::m_vertexBuffer;

ParticleSystem::ParticleSystem(ParticleSystem::PARTICLE_TYPE type) {

	m_type = type;
	m_description = make_shared<Description>(type);

	if (type != NONE) {
		// string sid = "ParticleSystem" + to_string((long)this) + "-" + to_string(type) + ".txt";
		/*VariableSyncer::getInstance()->create(sid);
		VariableSyncer::getInstance()->bind(sid,
			"ParticleCount:i&emitRate:f&acceleration:v3&spawnRadius:f&radiusInterval:v2&velocity:"
			"v3&velocityOffsetInterval:v2&sizeInterval:v2&timeAliveInteral:v2&color[0]:v3&color[1]:"
			"v3&color[2]:v3",
			m_description.get());*/

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
	inactivateAllParticles();

	// Buffer
	createBuffers();

	// shader
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

	switch (m_type) {
	case PARTICLE_TYPE::STARS:
		m_currentShaderSet = &m_shaderSetStar;
		break;
	default:
		m_currentShaderSet = &m_shaderSetCircle;
		break;
	}

	run();
}

void ParticleSystem::setParticle(Description desc, size_t index) {
	Particle* part = &m_particles[index];
	ParticleProperty* partProp = &m_particleProperties[index];
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
	int pick = rand() % 3; // 0..1
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
	*partProp = pp;
}


void ParticleSystem::activateOneParticle() {
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].getActiveValue() == 0.f) {
			setParticle(*m_description, i);
			break;
		}
	}
}

void ParticleSystem::emit(size_t count) {
	m_isRunning = true;

	for (size_t i = 0; i < count; i++)
		activateOneParticle();
}

void ParticleSystem::updateEmits(float dt) {
	m_emitTimer += dt;
	float rate = m_description->m_emitRate;
	if (rate >= 0.f) {
		float emits = m_emitTimer * rate;
		size_t emitCount = (size_t)emits;
		if (emits > 1.0f) {
			emit(emitCount);
			m_emitTimer -= (1.f / rate) * emitCount;
		}
	}
}

void ParticleSystem::updateParticles(float dt, Terrain* terrain) {
	size_t nrOfActive = 0;
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].getActiveValue() == 1.0f) {
			nrOfActive++;
			m_particleProperties[i].m_velocity += m_particleProperties[i].m_acceleration * dt;
			m_particleProperties[i].m_timeLeft -= dt;
			m_particles[i].update(
				dt, m_particleProperties[i].m_velocity +
						terrain->getWindFromPosition(m_particles[i].getPosition()));

			// Inactivate particles when lifetime is over
			if (m_particleProperties[i].m_timeLeft <= 0.f) {
				m_particles[i].setActiveValue(0.0f);
			}
		}
	}

	if (nrOfActive == 0) {
		m_isRunning = false;
	}
}

void ParticleSystem::updateParticles(float dt, float3 wind) {
	size_t nrOfActive = 0;
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].getActiveValue() == 1.0f) {
			nrOfActive++;
			m_particleProperties[i].m_velocity += m_particleProperties[i].m_acceleration * dt;
			m_particleProperties[i].m_timeLeft -= dt;
			m_particles[i].update(dt, m_particleProperties[i].m_velocity + wind);

			// Inactivate particles when lifetime is over
			if (m_particleProperties[i].m_timeLeft <= 0.f) {
				m_particles[i].setActiveValue(0.0f);
			}
		}
	}

	if (nrOfActive == 0) {
		m_isRunning = false;
	}
}

void ParticleSystem::update(float dt, Terrain* terrain) {
	m_timePassed += dt;

	if (m_isEmitting) {
		updateEmits(dt);
	}

	if (m_isRunning) {
		updateParticles(dt, terrain);
	}
}

void ParticleSystem::update(float dt, float3 wind) {
	m_timePassed += dt;

	if (m_isEmitting) {
		updateEmits(dt);
	}

	if (m_isRunning) {
		updateParticles(dt, wind);
	}
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
		m_particles.reserve(MAX_PARTICLES);
		buffDesc.ByteWidth = (UINT)(sizeof(Particle) * m_particles.size());

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
	if (m_isRunning) {
		auto deviceContext = Renderer::getDeviceContext();
		// Since we are using the same vertex buffer for all Particle Systems
		// the buffer update needs to be next to the draw call.

		m_currentShaderSet->bindShadersAndLayout();
		Renderer::getDeviceContext()->UpdateSubresource(
			m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);
		bindBuffers();

		Renderer::getInstance()->enableAlphaBlending();
		deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
		Renderer::getInstance()->disableAlphaBlending();
	}
}

void ParticleSystem::drawNoAlpha() {
	if (m_isRunning) {
		auto deviceContext = Renderer::getDeviceContext();
		// Since we are using the same vertex buffer for all Particle Systems
		// the buffer update needs to be next to the draw call.
		m_currentShaderSet->bindShadersAndLayout();
		Renderer::getDeviceContext()->UpdateSubresource(
			m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);
		bindBuffers();
		deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
	}
}

void ParticleSystem::activateAllParticles() { emit(m_particles.size()); }

void ParticleSystem::inactivateAllParticles() {
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particles[i].setActiveValue(0.0f);
	}
}

void ParticleSystem::run(bool startAll) {
	if (startAll) {
		activateAllParticles();
	}
	if (m_type == ARROW_GLITTER)
		m_isRunning = true;
}

void ParticleSystem::stop() {
	if (m_isRunning) {
		m_isRunning = false;
	}
}


bool ParticleSystem::isRunning() { return m_isRunning; }

void ParticleSystem::setPosition(float3 position) { m_spawnPoint = position; }

float3 ParticleSystem::getPosition() const { return m_spawnPoint; }

void ParticleSystem::setEmitState(bool state) { m_isEmitting = state; }

ParticleSystem::PARTICLE_TYPE ParticleSystem::getType() const { return m_type; }

void ParticleSystem::setEmitRate(float emitRate) { m_description->m_emitRate = emitRate; }

void ParticleSystem::setColors(float4 colors[3]) {
	m_description->m_color[0] = colors[0];
	m_description->m_color[1] = colors[1];
	m_description->m_color[2] = colors[2];
}

void ParticleSystem::setAmountOfParticles(int nrOf) {
	m_description->m_nrOfParticles = (int)nrOf;
	m_particles.resize(m_description->m_nrOfParticles);
}

void ParticleSystem::setDesciption(Description newDescription) { *m_description = newDescription; }

ParticleSystem::Description* ParticleSystem::getDescription() { return m_description.get(); }
