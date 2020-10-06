#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#include "Terrain.h"
#include "Fragment.h"

class ParticleSystem : public Fragment, public Transformation {
public:
	enum Type {
		NONE,
		FOREST_BUBBLE,
		GROUND_DUST,
		VULCANO_FIRE,
		VULCANO_SMOKE,
		LAVA_BUBBLE,
		ARROW_GLITTER,
		CONFETTI,
		STARS_GOLD,
		STARS_SILVER,
		STARS_BRONZE,
		EXPLOSION_APPLE,
		EXPLOSION_BANANA,
		EXPLOSION_MELON,
		EXPLOSION_DRAGON,
		SPARKLE_APPLE,
		SPARKLE_BANANA,
		SPARKLE_MELON,
		SPARKLE_DRAGON,
		EXPLOSION_GOLD,
		EXPLOSION_SILVER,
		EXPLOSION_BRONZE,
		JUMP_DUST,
		TYPE_LENGTH
	};
	struct ParticleDescription {
		float4 colorVariety[3];
		float2 size_interval;	   // size.x (min), size.y (max)
		float2 timeAlive_interval; // size.x (min), size.y (max)
		float3 velocity_min,
			velocity_max;		  // velocity inside box, between points min and max. (Normalized)
		float2 velocity_interval; // strength of velocity
		float3 acceleration;	  // can be used to produce gravity for particles for example
		float slowdown;
		float fadeInterval_start; // time(start) to time(x) seconds to scale particle to real size
		float fadeInterval_end; // time(end-x) to time(end) seconds to scale particle to nothing 
		enum Shape { Circle, Star } shape;
		ParticleDescription(ParticleSystem::Type type = ParticleSystem::Type::NONE);
	};

private:
	Type m_type = NONE;

	bool m_isActive = true;
	bool m_isEmitting = true;
	float m_emitTimer = 0;
	// description
	ParticleDescription m_particle_description;
	bool m_affectedByWind = false;
	float m_emitRate = 0;
	size_t m_capacity = 0;
	// particle properties
	struct ParticleProperty { // not going to the GPU
		float3 velocity;// current velocity
		float lifeTime = 1;
		float timeLeft = 1;
		float size = 1; // start size
	};
	vector<ParticleProperty> m_particleProperties;
	struct Particle {
		float3 position;
		float4 color = float4(1.);
		float size = 1; // size after fade scaling is applied
		float isActive = false;
	};
	vector<Particle> m_particles;

	// shaders
	static ShaderSet m_shaderSetCircle;
	static ShaderSet m_shaderSetStar;

	// vertex Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	// -- Functions --

	void createShaders();

	void resizeBuffer();
	void bindVertexBuffer();

	void updateEmits(float dt);

	void updateParticles(float dt);

	void setParticle(size_t index);

public:
	void emitingState(bool state);
	void activeState(bool state);
	void affectedByWindState(bool state);
	bool isActive() const;
	size_t activeParticleCount() const;
	bool isEmiting() const;
	bool isAffectedByWind() const;

	void setType(Type type, bool resize = true);
	void setCustomDescription(ParticleSystem::ParticleDescription desc, bool resize = true);
	void setEmitRate(float emitRate, bool resize = true);
	void setCapacity(size_t capacity = 0);
	float getEmitRate() const;
	size_t getCapacity() const;
	Type getType() const;
	ParticleSystem::ParticleDescription getParticleDescription() const;

	void update(float dt);
	void emit(size_t count);

	void draw(bool alpha = true);

	void load(ParticleSystem::Type type, float emitRate, size_t capacity = 0);
	ParticleSystem();
};