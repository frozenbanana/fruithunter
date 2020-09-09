#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"
#include "VariableSyncer.h"
#include "SceneManager.h"

ShaderSet ParticleSystem::m_shaderSetCircle;
ShaderSet ParticleSystem::m_shaderSetStar;

void ParticleSystem::load(ParticleSystem::Type type, float emitRate, size_t capacity) {
	setType(type, false);
	setEmitRate(emitRate, false);
	setCapacity(capacity);
}

ParticleSystem::ParticleSystem()
	: Fragment(Fragment::Type::particleSystem), Transformation(float3(0.),float3(0.)) {
	createShaders();
}

void ParticleSystem::setParticle(size_t index) {
	Particle* part = &m_particles[index];
	part->isActive = true;
	// Position in world
	float3 position = getPosition();
	float3 scale = getScale();
	// Positon in box
	part->position = position + float3(RandomFloat(-scale.x / 2, scale.x / 2),
									RandomFloat(-scale.y / 2, scale.y / 2),
									RandomFloat(-scale.z / 2, scale.z / 2));
	// Color
	part->color = m_particle_description.colorVariety[rand() % 3];
	// Size
	part->size = RandomFloat(
			m_particle_description.size_interval.x, m_particle_description.size_interval.y);

	ParticleProperty* pp = &m_particleProperties[index];
	// Time alive
	pp->lifeTime = RandomFloat(
		m_particle_description.timeAlive_interval.x, m_particle_description.timeAlive_interval.y);
	pp->timeLeft = pp->lifeTime;

	// velocity
	float randVeloX =
		RandomFloat(m_particle_description.velocity_min.x, m_particle_description.velocity_max.x);
	float randVeloY =
		RandomFloat(m_particle_description.velocity_min.y, m_particle_description.velocity_max.y);
	float randVeloZ =
		RandomFloat(m_particle_description.velocity_min.z, m_particle_description.velocity_max.z);
	float3 direction = Normalize(float3(randVeloX, randVeloY, randVeloZ));
	float strength = RandomFloat(
		m_particle_description.velocity_interval.x, m_particle_description.velocity_interval.y);
	pp->velocity = direction * strength;
}

void ParticleSystem::emitingState(bool state) { m_isEmitting = state; }

void ParticleSystem::activeState(bool state) { m_isActive = state; }

void ParticleSystem::emit(size_t count) {
	for (size_t i = 0; i < m_particles.size() && count > 0; i++) {
		if (m_particles[i].isActive == false) {
			setParticle(i);
			count--;
		}
	}
}

void ParticleSystem::updateEmits(float dt) {
	m_emitTimer += dt;
	float rate = m_emitRate;
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
	Terrain* terrain = SceneManager::getScene()->m_terrains.getTerrainFromPosition(getPosition());
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].isActive == true) {
			m_particleProperties[i].timeLeft -= dt;
			if (m_particleProperties[i].timeLeft <= 0.f) {
				// Inactivate particles when lifetime is over
				m_particles[i].isActive = false;
			}
			else {
				float density = 1; // air
				float dragCoefficient = 1;

				float r = m_particles[i].size / 2.f;
				float area = 3.1415f * pow(r, 2);
				float mass = 3.1415f * (4.f / 3.f) * pow(r, 3);
				// get wind
				float3 acceleration = m_particle_description.acceleration;
				if (terrain != nullptr && m_affectedByWind) {
					float3 wind = terrain->getWindStatic();
					float3 v_relative = (m_particleProperties[i].velocity - wind);//velocity relative wind
					float v_length = v_relative.Length();
					float Fd = 0.5 * density * pow(v_length, 2) * area * dragCoefficient; // drag from wind
					acceleration += (-Fd/mass) * Normalize(v_relative);
				}
				// update velocity and position
				m_particleProperties[i].velocity += acceleration * dt;
				m_particles[i].position += m_particleProperties[i].velocity * dt;
				m_particleProperties[i].velocity *= pow(m_particle_description.slowdown, dt);
			}
		}
	}
}

void ParticleSystem::update(float dt) {
	if (m_isActive) {
		if (m_isEmitting) {
			updateEmits(dt);
		}
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

void ParticleSystem::resizeBuffer() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	// calc size
	size_t size = m_capacity;
	if (m_capacity == 0)
		size = round(m_emitRate * m_particle_description.timeAlive_interval.y);
	if (size != m_particles.size()) {
		// resize buffers
		m_particles.resize(size);
		m_particleProperties.resize(size);

		//  Buffer for particle data
		m_vertexBuffer.Reset();
		if (size != 0) {
			D3D11_BUFFER_DESC buffDesc;
			memset(&buffDesc, 0, sizeof(buffDesc));
			buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
			buffDesc.ByteWidth = (UINT)(sizeof(Particle) * size);

			HRESULT check = device->CreateBuffer(&buffDesc, NULL, m_vertexBuffer.GetAddressOf());

			if (FAILED(check))
				ErrorLogger::logError("(ParticleSystem) Failed creating vertex buffer!\n", check);
		}
	}
	else {
		// already the correct size
	}
}

void ParticleSystem::bindVertexBuffer() {
	auto deviceContext = Renderer::getDeviceContext();
	if (m_vertexBuffer.Get() != nullptr) {
		deviceContext->UpdateSubresource(m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);

		UINT strides = sizeof(Particle);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	}
}

void ParticleSystem::draw(bool alpha) {
	if (m_isActive && m_particles.size() > 0) {
		auto deviceContext = Renderer::getDeviceContext();

		// bind
		switch (m_particle_description.shape) {
		case ParticleDescription::Shape::Circle:
			m_shaderSetCircle.bindShadersAndLayout();
			break;
		case ParticleDescription::Shape::Star:
			m_shaderSetStar.bindShadersAndLayout();
			break;
		default:
			m_shaderSetCircle.bindShadersAndLayout();
			break;
		}
		bindVertexBuffer();

		// draw
		if (alpha) {
			Renderer::getInstance()->enableAlphaBlending();
			deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
			Renderer::getInstance()->disableAlphaBlending();
			ShaderSet::clearShaderBindings(); // removes bug of sprites not being able to be
											  // drawn(by removing geometry shade)
		}
		else {
			deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
		}
	}
}

void ParticleSystem::setType(Type type, bool resize) {
	m_type = type;
	m_particle_description = ParticleDescription(type); 
	if (resize)
		resizeBuffer();
}

void ParticleSystem::setCustomDescription(ParticleSystem::ParticleDescription desc, bool resize) {
	m_particle_description = desc;
	if (resize)
		resizeBuffer();
}

void ParticleSystem::affectedByWindState(bool state) { m_affectedByWind = state; }

bool ParticleSystem::isActive() const { return m_isActive; }

size_t ParticleSystem::activeParticleCount() const {
	size_t sum = 0;
	for (size_t i = 0; i < m_particles.size(); i++) {
		sum += (m_particles[i].isActive == 1);
	}
	return sum;
}

bool ParticleSystem::isEmiting() const { return m_isEmitting; }

bool ParticleSystem::isAffectedByWind() const { return m_affectedByWind; }

void ParticleSystem::setEmitRate(float emitRate, bool resize) { 
	m_emitRate = emitRate; 
	if (resize)
		resizeBuffer();
}

void ParticleSystem::setCapacity(size_t capacity) { 
	m_capacity = capacity;
	resizeBuffer();
}

float ParticleSystem::getEmitRate() const { return m_emitRate; }

size_t ParticleSystem::getCapacity() const { return m_capacity; }

ParticleSystem::Type ParticleSystem::getType() const { return m_type; }

ParticleSystem::ParticleDescription ParticleSystem::getParticleDescription() const {
	return m_particle_description;
}

ParticleSystem::ParticleDescription::ParticleDescription(ParticleSystem::Type type) {
	switch (type) {
	case ParticleSystem::FOREST_BUBBLE:
		colorVariety[0] = float4(0.0f, 0.65f, 0.05f, 1.0f);
		colorVariety[1] = float4(0.0f, 0.65f, 0.4f, 1.0f);
		colorVariety[2] = float4(0.0f, 0.65f, 0.55f, 1.0f);
		size_interval = float2(0.04f, 0.06f);
		timeAlive_interval = float2(4.f, 6.f);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(0.43, 0.43);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::GROUND_DUST:
		colorVariety[0] = float4(0.77f, 0.35f, 0.51f, 1.0f);
		colorVariety[1] = float4(0.71f, 0.55f, 0.31f, 1.0f);
		colorVariety[2] = float4(0.81f, 0.58f, 0.39f, 1.0f);
		size_interval = float2(0.04f, 0.06f);
		timeAlive_interval = float2(2.5f, 3.5f);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(0.43, 0.43);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::VULCANO_FIRE:
		colorVariety[0] = float4(179 / 255.f, 0 / 255.f, 0 / 255.f, 1.0f);
		colorVariety[1] = float4(229 / 255.f, 74 / 255.f, 10 / 255.f, 1.0f);
		colorVariety[2] = float4(200 / 255.f, 160 / 255.f, 4 / 255.f, 1.0f);
		size_interval = float2(0.7,1.5);
		timeAlive_interval = float2(1.5,3);
		velocity_max = float3(1.);
		velocity_min = float3(-1,0,-1);
		velocity_interval = float2(1,2);
		acceleration = float3(0, 10, 0);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::VULCANO_SMOKE:
		colorVariety[0] = float4(0.50f, 0.40f, 0.40f, 1.0f);
		colorVariety[1] = float4(0.30f, 0.30f, 0.30f, 1.0f);
		colorVariety[2] = float4(0.60f, 0.60f, 0.60f, 1.0f);
		size_interval = float2(0.9,1.4);
		timeAlive_interval = float2(2,10);
		velocity_max = float3(1.) * 0.25f;
		velocity_min = float3(-1, 0, -1) * 0.25f;
		velocity_interval = float2(0.75,1.5);
		acceleration = float3(0, 3, 0);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::LAVA_BUBBLE:
		colorVariety[0] = float4(0.70f, 0.20f, 0.20f, 1.0f);
		colorVariety[1] = float4(0.41f, 0.25f, 0.23f, 1.0f);
		colorVariety[2] = float4(0.51f, 0.34f, 0.17f, 1.0f);
		size_interval = float2(0.04f, 0.06f);
		timeAlive_interval = float2(0.4f, 1.6f);
		velocity_max = float3(1.);
		velocity_min = float3(-1);
		velocity_interval = float2(0.2, 0.5);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::ARROW_GLITTER:
		colorVariety[0] = float4(0.88f, 0.87f, 0.81f, 1.0f);
		colorVariety[1] = float4(0.92f, 0.89f, 0.83f, 1.0f);
		colorVariety[2] = float4(0.87f, 0.82f, 0.80f, 1.0f);
		size_interval = float2(0.05, 0.05);
		timeAlive_interval = float2(0.45,0.55);
		velocity_max = float3(0.);
		velocity_min = float3(0.);
		velocity_interval = float2(0.);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::CONFETTI:
		colorVariety[0] = float4(214 / 255.f, 53 / 255.f, 53 / 255.f, 1.0f);
		colorVariety[1] = float4(62 / 255.f, 201 / 255.f, 62 / 255.f, 1.0f);
		colorVariety[2] = float4(61 / 255.f, 61 / 255.f, 211 / 255.f, 1.0f);
		size_interval = float2(0.035f, 0.09f);
		timeAlive_interval = float2(1.5f, 3.0f);
		velocity_max = float3(0.2, 1, 0.2);
		velocity_min = float3(-0.2, 0.5, -0.2);
		velocity_interval = float2(5, 7);
		acceleration = float3(0, -10, 0);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	case ParticleSystem::STARS_GOLD:
		colorVariety[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
		colorVariety[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
		colorVariety[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
		size_interval = float2(0.2f, .3f);
		timeAlive_interval = float2(0.6f, .65f);
		velocity_max = float3(1.);
		velocity_min = float3(-1, 0.5, -1);
		velocity_interval = float2(1.5, 3.5);
		acceleration = float3(0.);
		slowdown = 0.1;
		shape = Shape::Star;
		break;
	case ParticleSystem::STARS_SILVER:
		colorVariety[0] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		colorVariety[1] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		colorVariety[2] = float4(0.75f, 0.75f, 0.75f, 1.0f);
		size_interval = float2(0.2f, .3f);
		timeAlive_interval = float2(0.6f, .65f);
		velocity_max = float3(1.);
		velocity_min = float3(-1, 0.5, -1);
		velocity_interval = float2(1.5, 3.5);
		acceleration = float3(0.);
		slowdown = 0.1;
		shape = Shape::Star;
		break;
	case ParticleSystem::STARS_BRONZE:
		colorVariety[0] = float4(0.69f, 0.34f, 0.05f, 1.0f);
		colorVariety[1] = float4(0.71f, 0.36f, 0.07f, 1.0f);
		colorVariety[2] = float4(0.70f, 0.32f, 0.09f, 1.0f);
		size_interval = float2(0.2f, .3f);
		timeAlive_interval = float2(0.6f, .65f);
		velocity_max = float3(1.);
		velocity_min = float3(-1, 0.5, -1);
		velocity_interval = float2(1.5, 3.5);
		acceleration = float3(0.);
		slowdown = 0.1;
		shape = Shape::Star;
		break;
	case ParticleSystem::EXPLOSION_APPLE:
		colorVariety[0] = float4(228 / 255.f, 83 / 255.f, 83 / 255.f, 1.f);
		colorVariety[1] = float4(186 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
		colorVariety[2] = float4(255 / 255.f, 150 / 255.f, 150 / 255.f, 1.f);
		size_interval = float2(0.075, 0.1);
		timeAlive_interval = float2(0.4, 0.7);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(3, 20);
		acceleration = float3(0.);
		slowdown = 0.0001;
		shape = Shape::Circle;
		break;
	case ParticleSystem::EXPLOSION_BANANA:
		colorVariety[0] = float4(249 / 255.f, 255 / 255.f, 158 / 255.f, 1.f);
		colorVariety[1] = float4(232 / 255.f, 255 / 255.f, 58 / 255.f, 1.f);
		colorVariety[2] = float4(225 / 255.f, 255 / 255.f, 0 / 255.f, 1.f);
		size_interval = float2(0.075, 0.1);
		timeAlive_interval = float2(0.4, 0.7);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(3, 20);
		acceleration = float3(0.);
		slowdown = 0.0001;
		shape = Shape::Circle;
		break;
	case ParticleSystem::EXPLOSION_MELON:
		colorVariety[0] = float4(58 / 255.f, 176 / 255.f, 60 / 255.f, 1.f);
		colorVariety[1] = float4(72 / 255.f, 141 / 255.f, 65 / 255.f, 1.f);
		colorVariety[2] = float4(72 / 255.f, 226 / 255.f, 39 / 255.f, 1.f);
		size_interval = float2(0.075, 0.1);
		timeAlive_interval = float2(0.4, 0.7);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(3, 20);
		acceleration = float3(0.);
		slowdown = 0.0001;
		shape = Shape::Circle;
		break;
	case ParticleSystem::EXPLOSION_DRAGON:
		colorVariety[0] = float4(78 / 255.f, 158 / 255.f, 80 / 255.f, 1.f);
		colorVariety[1] = float4(235 / 255.f, 83 / 255.f, 83 / 255.f, 1.f);
		colorVariety[2] = float4(215 / 255.f, 51 / 255.f, 51 / 255.f, 1.f);
		size_interval = float2(0.075, 0.1);
		timeAlive_interval = float2(0.4, 0.7);
		velocity_max = float3(1.);
		velocity_min = float3(-1.);
		velocity_interval = float2(3, 20);
		acceleration = float3(0.);
		slowdown = 0.0001;
		shape = Shape::Circle;
		break;
	case ParticleSystem::SPARKLE_APPLE:
		colorVariety[0] = float4(228 / 255.f, 83 / 255.f, 83 / 255.f, 1.f);
		colorVariety[1] = float4(186 / 255.f, 33 / 255.f, 33 / 255.f, 1.f);
		colorVariety[2] = float4(255 / 255.f, 150 / 255.f, 150 / 255.f, 1.f);
		size_interval = float2(0.15, 0.3);
		timeAlive_interval = float2(0.1, 1.0);
		velocity_max = float3(-1.);
		velocity_min = float3(1.);
		velocity_interval = float2(0.1, 0.2);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Star;
		break;
	case ParticleSystem::SPARKLE_BANANA:
		colorVariety[0] = float4(249 / 255.f, 255 / 255.f, 158 / 255.f, 1.f);
		colorVariety[1] = float4(232 / 255.f, 255 / 255.f, 58 / 255.f, 1.f);
		colorVariety[2] = float4(225 / 255.f, 255 / 255.f, 0 / 255.f, 1.f);
		size_interval = float2(0.15, 0.3);
		timeAlive_interval = float2(0.1, 1.0);
		velocity_max = float3(-1.);
		velocity_min = float3(1.);
		velocity_interval = float2(0.1, 0.2);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Star;
		break;
	case ParticleSystem::SPARKLE_MELON:
		colorVariety[0] = float4(58 / 255.f, 176 / 255.f, 60 / 255.f, 1.f);
		colorVariety[1] = float4(72 / 255.f, 141 / 255.f, 65 / 255.f, 1.f);
		colorVariety[2] = float4(72 / 255.f, 226 / 255.f, 39 / 255.f, 1.f);
		size_interval = float2(0.15, 0.3);
		timeAlive_interval = float2(0.1, 1.0);
		velocity_max = float3(-1.);
		velocity_min = float3(1.);
		velocity_interval = float2(0.1, 0.2);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Star;
		break;
	case ParticleSystem::SPARKLE_DRAGON:
		colorVariety[0] = float4(78 / 255.f, 158 / 255.f, 80 / 255.f, 1.f);
		colorVariety[1] = float4(235 / 255.f, 83 / 255.f, 83 / 255.f, 1.f);
		colorVariety[2] = float4(215 / 255.f, 51 / 255.f, 51 / 255.f, 1.f);
		size_interval = float2(0.15, 0.3);
		timeAlive_interval = float2(0.1, 1.0);
		velocity_max = float3(-1.);
		velocity_min = float3(1.);
		velocity_interval = float2(0.1, 0.2);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Star;
		break;
	case ParticleSystem::EXPLOSION_GOLD:
		colorVariety[0] = float4(255 / 255.f, 240 / 255.f, 0 / 255.f, 1.f);
		colorVariety[1] = float4(255 / 255.f, 244 / 255.f, 68 / 255.f, 1.f);
		colorVariety[2] = float4(255 / 255.f, 255 / 255.f, 118 / 255.f, 1.f);
		size_interval = float2(0.4,0.6);
		timeAlive_interval = float2(0.4,2.5);
		velocity_max = float3(-0.5,0.5,-0.5);
		velocity_min = float3(0.5, 1, 0.5);
		velocity_interval = float2(10,25);
		acceleration = float3(0,-3,0);
		slowdown = 0.05f;
		shape = Shape::Star;
		break;
	case ParticleSystem::EXPLOSION_SILVER:
		colorVariety[0] = float4(194 / 255.f, 194 / 255.f, 194 / 255.f, 1.f);
		colorVariety[1] = float4(223 / 255.f, 223 / 255.f, 223 / 255.f, 1.f);
		colorVariety[2] = float4(154 / 255.f, 154 / 255.f, 154 / 255.f, 1.f);
		size_interval = float2(0.4, 0.6);
		timeAlive_interval = float2(0.4, 2.5);
		velocity_max = float3(-0.5, 0.5, -0.5);
		velocity_min = float3(0.5, 1, 0.5);
		velocity_interval = float2(10, 25);
		acceleration = float3(0, -3, 0);
		slowdown = 0.05f;
		shape = Shape::Star;
		break;
	case ParticleSystem::EXPLOSION_BRONZE:
		colorVariety[0] = float4(206 / 255.f, 115 / 255.f, 0 / 255.f, 1.f);
		colorVariety[1] = float4(190 / 255.f, 84 / 255.f, 0 / 255.f, 1.f);
		colorVariety[2] = float4(231 / 255.f, 122 / 255.f, 25 / 255.f, 1.f);
		size_interval = float2(0.4, 0.6);
		timeAlive_interval = float2(0.4, 2.5);
		velocity_max = float3(-0.5, 0.5, -0.5);
		velocity_min = float3(0.5, 1, 0.5);
		velocity_interval = float2(10, 25);
		acceleration = float3(0, -3, 0);
		slowdown = 0.05f;
		shape = Shape::Star;
		break;
	default:
		colorVariety[0] = float4(1.);
		colorVariety[1] = float4(1.);
		colorVariety[2] = float4(1.);
		size_interval = float2(1,1);
		timeAlive_interval = float2(1,1);
		velocity_max = float3(0.);
		velocity_min = float3(0.);
		velocity_interval = float2(0, 0);
		acceleration = float3(0.);
		slowdown = 1;
		shape = Shape::Circle;
		break;
	}
}
