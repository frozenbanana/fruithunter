#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#include "Terrain.h"

class ParticleSystem {
public:
	enum PARTICLE_TYPE {
		NONE,
		FOREST_BUBBLE,
		GROUND_DUST,
		VULCANO_FIRE,
		VULCANO_SMOKE,
		LAVA_BUBBLE,
		ARROW_GLITTER,
		CONFETTI,
		STARS,
		TYPE_LENGTH
	};

private:
	PARTICLE_TYPE m_type = NONE;
	float3 m_spawnPoint = float3(0, 0, 0);
	size_t m_size = 0;

	bool m_isRunning;
	bool m_isEmitting = true;
	float m_emitTimer = 0;
	//wind
	enum WindState {
		None,
		Static,
		Dynamic
	} m_windState = None;
	float3 m_staticWind = float3(0, 0, 0);
	//description
	struct Description {
		float m_emitRate; // particles per sec
		float3 m_acceleration;
		float2 m_accelerationOffsetInterval;
		float m_spawnRadius;
		float2 m_radiusInterval;
		float3 m_velocity;
		float2 m_velocityOffsetInterval;
		float2 m_sizeInterval; // min and max
		float2 m_timeAliveInterval;
		float4 m_color[3];
		void load(ParticleSystem::PARTICLE_TYPE type);
		Description(ParticleSystem::PARTICLE_TYPE type = NONE);
	};
	Description m_description;
	//particle properties
	struct ParticleProperty { // not going to the GPU
		float3 m_acceleration;
		float3 m_velocity;
		float m_lifeTime;
		float m_timeLeft;
	};
	vector<ParticleProperty> m_particleProperties;
	vector<Particle> m_particles;

	//shaders
	static ShaderSet m_shaderSetCircle;
	static ShaderSet m_shaderSetStar;
	ShaderSet* m_currentShaderSet = nullptr;

	// vertex Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	// -- Functions --

	void createShaders();

	void resizeVertexBuffer(size_t size);
	void bindVertexBuffer();

	void updateEmits(float dt);
	void updateParticles(float dt);

	void setParticle(Description desc, size_t index);

public:
	void setEmitRate(float emitRate);
	void setColors(float4 colors[3]);
	void setPosition(float3 position);
	void setWind(WindState state, float3 wind = float3(0, 0, 0));

	float3 getPosition() const;
	PARTICLE_TYPE getType() const;

	void update(float dt);

	void draw();
	void drawNoAlpha();

	void activateAllParticles();

	void stopEmiting();
	void startEmiting();
	void emit(size_t count);

	void load(ParticleSystem::PARTICLE_TYPE type);
	ParticleSystem(ParticleSystem::PARTICLE_TYPE type = NONE);
};
