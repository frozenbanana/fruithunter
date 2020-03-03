#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#define MAX_PARTICLES 512


class ParticleSystem {
public:
	enum PARTICLE_TYPE {
		NONE = 0,
		FOREST_BUBBLE = 1,
		GROUND_DUST = 2,
		VULCANO_BUBBLE = 3,
		LAVA_BUBBLE = 4,
		ARROW_GLITTER = 5,
		TYPE_LENGTH = 6,
	};

	ParticleSystem(ParticleSystem::PARTICLE_TYPE type = NONE);
	void activateParticle();
	void update(float dt, float3 wind = float3(0, 0, 0));
	void draw();
	void setActive();
	void setInActive();
	bool getIsActive();
	void setPosition(float3 position);
	float3 getPosition() const;

private:
	struct Description {
		int m_nrOfParticles;
		float m_emitRate; // particles per sec
		float3 m_acceleration;
		float2 m_accelerationOffsetInterval;
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
				m_nrOfParticles = MAX_PARTICLES - 1;
				m_emitRate = 25.0f; // particles per sec
				m_acceleration = float3(0.1f, 0.5f, -0.1f);
				m_accelerationOffsetInterval = float2(-0.05f, 0.05f);
				m_spawnRadius = 40.f;
				m_radiusInterval = float2(-35.2f, 0.2f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-3.8f, 3.8f); // for x, y and z
				m_sizeInterval = float2(0.10f, 0.20f);
				m_timeAliveInterval = float2(3.f, 5.f);
				m_color[0] = float3(0.0f, 0.65f, 0.05f);
				m_color[1] = float3(0.0f, 0.65f, 0.4f);
				m_color[2] = float3(0.0f, 0.65f, 0.55f);
				break;
			case GROUND_DUST:
				m_nrOfParticles = MAX_PARTICLES - 1;
				m_emitRate = 25.0f; // particles per sec
				m_acceleration = float3(0.9f, 1.5f, -0.6f);
				m_accelerationOffsetInterval = float2(-0.05f, 0.05f);
				m_spawnRadius = 30.f;
				m_radiusInterval = float2(-15.2f, 0.2f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-2.8f, 2.8f); // for x, y and z
				m_sizeInterval = float2(0.10f, 0.20f);
				m_timeAliveInterval = float2(2.5f, 3.5f);
				m_color[0] = float3(0.77f, 0.35f, 0.51f);
				m_color[1] = float3(0.71f, 0.55f, 0.31f);
				m_color[2] = float3(0.81f, 0.58f, 0.39f);
				break;
			case VULCANO_BUBBLE:
				m_nrOfParticles = MAX_PARTICLES - 1;
				m_emitRate = 88.0f; // particles per sec
				m_acceleration = float3(0.3f, 3.5f, 0.3f);
				m_accelerationOffsetInterval = float2(-0.05f, 0.05f);
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
			case LAVA_BUBBLE:
				m_nrOfParticles = MAX_PARTICLES / 2;
				m_emitRate = 25.0f; // particles per sec
				m_acceleration = float3(0.3f, 0.3f, 0.3f);
				m_accelerationOffsetInterval = float2(-0.05f, 0.05f); // for x, y and z
				m_spawnRadius = 45.f;
				m_radiusInterval = float2(-35.f, 0.1f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-0.75f, 0.75f); // for x, y and z
				m_sizeInterval = float2(0.11f, 0.22f);
				m_timeAliveInterval = float2(0.4f, 1.1f);
				m_color[0] = float3(0.70f, 0.20f, 0.20f);
				m_color[1] = float3(0.41f, 0.25f, 0.23f);
				m_color[2] = float3(0.51f, 0.34f, 0.17f);
				break;
			case ARROW_GLITTER:
				m_nrOfParticles = 30;
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
		float3 m_acceleration;
		float3 m_velocity;
		float m_lifeTime;
		float m_timeLeft;
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
