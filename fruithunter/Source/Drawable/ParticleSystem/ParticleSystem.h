#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#define MAX_PARTICLES 256


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
		TYPE_LENGTH,
	};

	ParticleSystem(ParticleSystem::PARTICLE_TYPE type = NONE);
	void activateOneParticle();
	void emit(size_t count);
	void updateEmits(float dt);
	void updateParticles(float dt, float3 wind);
	void update(float dt, float3 wind = float3(0, 0, 0));
	void draw();
	void drawNoAlpha();
	void activateAllParticles();
	void inactivateAllParticles();
	void run(bool startAll = false);
	void stop();
	bool isRunning();
	void setPosition(float3 position);
	float3 getPosition() const;
	void setEmitState(bool state);

private:
	PARTICLE_TYPE m_type = NONE;
	float3 m_spawnPoint = float3(0, 0, 0);
	bool m_isRunning;
	bool m_isEmitting = true;
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
		float4 m_color[3];
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
				m_color[0] = float4(0.0f, 0.65f, 0.05f, 1.0f);
				m_color[1] = float4(0.0f, 0.65f, 0.4f, 1.0f);
				m_color[2] = float4(0.0f, 0.65f, 0.55f, 1.0f);
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
				m_color[0] = float4(0.77f, 0.35f, 0.51f, 1.0f);
				m_color[1] = float4(0.71f, 0.55f, 0.31f, 1.0f);
				m_color[2] = float4(0.81f, 0.58f, 0.39f, 1.0f);
				break;
			case VULCANO_FIRE:
				m_nrOfParticles = MAX_PARTICLES - 1;
				m_emitRate = 80.0f; // particles per sec
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
				m_nrOfParticles = MAX_PARTICLES - 1;
				m_emitRate = 80.0f; // particles per sec
				m_acceleration = float3(0.f, -0.9f, 0.f);
				m_accelerationOffsetInterval = float2(-2.20f, 2.20f);
				m_spawnRadius = 0.75f;
				m_radiusInterval = float2(-0.7f, 0.0f);
				m_velocity = float3(0.f, 0.f, 0.f);
				m_velocityOffsetInterval = float2(-1.5f, 1.5f); // for x, y and z
				m_sizeInterval = float2(0.48f, 2.8f);
				m_timeAliveInterval = float2(4.f, 5.f);
				m_color[0] = float4(0.50f, 0.40f, 0.40f, 1.0f);
				m_color[1] = float4(0.30f, 0.30f, 0.30f, 1.0f);
				m_color[2] = float4(0.60f, 0.60f, 0.60f, 1.0f);
				break;
			case LAVA_BUBBLE:
				m_nrOfParticles = 2 * MAX_PARTICLES / 3;
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
				m_nrOfParticles = 64;
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
				m_nrOfParticles = MAX_PARTICLES - 1;
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
				m_nrOfParticles = MAX_PARTICLES - 1;
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
	static ShaderSet m_shaderSetCircle;
	static ShaderSet m_shaderSetStar;
	ShaderSet* m_currentShaderSet;

	// Buffers
	// All particle systems share the same buffer since they wil all be the same size
	// And data will always update before each indivudal draw call
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	void createBuffers();
	void bindBuffers();
	void initialize();

	void setParticle(Description desc, size_t index);

public:
	void setEmitRate(float emitRate);
	void setColors(float4 colors[3]);
	void setAmountOfParticles(int nrOf);
	void setDesciption(Description newDescription);
	Description* getDescription();
};
