#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#define MAX_PARTICLES 1000


class ParticleSystem {
public:
	enum PARTICLE_TYPE {
		NONE,
		FOREST_BUBBLE,
		GROUND_DUST,
		VULCANO_BUBBLE,
		ARROW_GLITTER,
		TYPE_LENGTH,
	};

	ParticleSystem(ParticleSystem::PARTICLE_TYPE type = NONE);
	void activateParticle();
	void update(float dt);
	void draw();
	void setActive();
	void setInActive();
	bool getIsActive();
	void setPosition(float3 position);

private:
	struct Description {
		int m_nrOfParticles;
		float m_emitRate; // particles per sec
		float3 m_acceleration;
		float m_spawnRadius;
		float2 m_radiusInterval;
		float3 m_velocity;
		float2 m_velocityOffsetInterval;
		float2 m_sizeInterval; // min and max
		float2 m_timeAliveInterval;
		float3 m_color[3];
		Description(ParticleSystem::PARTICLE_TYPE type = NONE) {
			switch (type) {
			case FOREST_BUBBLE:
				m_nrOfParticles = 999;
				m_emitRate = 35.0f; // particles per sec
				m_acceleration = float3(0.1f, 0.5f, -0.1f);
				m_spawnRadius = 50.f;
				m_radiusInterval = float2(-45.2f, 0.2f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-3.8f, 3.8f); // for x, y and z
				m_sizeInterval = float2(0.15f, 0.25f);
				m_timeAliveInterval = float2(3.f, 5.f);
				m_color[0] = float3(0.0f, 0.75f, 0.1f);
				m_color[1] = float3(0.0f, 0.75f, 0.4f);
				m_color[2] = float3(0.0f, 0.75f, 0.65f);
				break;
			case GROUND_DUST:
				m_nrOfParticles = 999;
				m_emitRate = 25.0f; // particles per sec
				m_acceleration = float3(0.9f, 1.5f, -0.6f);
				m_spawnRadius = 30.f;
				m_radiusInterval = float2(-15.2f, 0.2f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-3.8f, 3.8f); // for x, y and z
				m_sizeInterval = float2(0.15f, 0.3f);
				m_timeAliveInterval = float2(4.f, 5.f);
				m_color[0] = float3(0.77f, 0.35f, 0.51f);
				m_color[1] = float3(0.71f, 0.55f, 0.31f);
				m_color[2] = float3(0.81f, 0.58f, 0.39f);
				break;
			case VULCANO_BUBBLE:
				m_nrOfParticles = 999;
				m_emitRate = 88.0f; // particles per sec
				m_acceleration = float3(0.3f, 3.5f, 0.3f);
				m_spawnRadius = 0.f;
				m_radiusInterval = float2(-0.1f, 0.1f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-0.75f, 0.75f); // for x, y and z
				m_sizeInterval = float2(0.43f, 0.8f);
				m_timeAliveInterval = float2(4.f, 5.f);
				m_color[0] = float3(1.0f, 0.f, 0.f);
				m_color[1] = float3(0.31f, 0.35f, 0.31f);
				m_color[2] = float3(0.81f, 0.58f, 0.0f);
				break;
			case ARROW_GLITTER:
				m_nrOfParticles = 100;
				m_emitRate = 12.0f; // particles per sec
				m_acceleration = float3(0.02f, 0.01f, 0.02f);
				m_spawnRadius = 0.1f;
				m_radiusInterval = float2(0.02f, 0.03f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-0.2f, 0.2f); // for x, y and z
				m_sizeInterval = float2(0.075f, 0.09f);
				m_timeAliveInterval = float2(0.5f, 1.0f);
				m_color[0] = float3(0.0f, 0.2f, 1.0f);
				m_color[1] = float3(0.0f, 0.6f, 0.79f);
				m_color[2] = float3(0.0f, 0.58f, 1.0f);
				break;
			default:
				m_nrOfParticles = 0;
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
	};
	float3 m_spawnPoint = float3(0, 0, 0);
	bool m_isActive;
	shared_ptr<Description> m_description;
	struct ParticleProperty { // not going to the GPU
		float3 m_velocity;
		float m_lifeTime;
	};

	vector<ParticleProperty> m_particleProperties;
	float m_timePassed;
	float m_emitTimer;

	vector<Particle> m_particles;
	size_t m_nrOfParticles;
	static ShaderSet m_shaderSet;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_colorBuffer;

	void createBuffers();
	void bindBuffers();
	void initialize();

	void setParticle(Description desc, size_t index);

public:
	void setDesciption(Description newDescription);
};
