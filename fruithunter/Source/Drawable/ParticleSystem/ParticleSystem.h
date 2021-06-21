#pragma once
#include "ShaderSet.h"
#include "Timer.h"
#include "Terrain.h"
#include "Fragment.h"
#include "TextureRepository.h"
#include "fstreamLib.h"

#define PATH_PSD "assets/ParticleSystems/Descriptions/"
#define PSD_END "psd" // psd - particle.system.description

class ParticleSystem : public Fragment, public Transformation {
public:
	struct ParticleDescription {
		string identifier;

		float4 colorVariety[3] = { float4(1.f), float4(1.f), float4(1.f) };
		float2 size_interval = float2(0.25, 0.5); // size.x (min), size.y (max)
		bool randomRotation = true;
		float startRotation = 0; // start rotation if random rotation is set to false
		float2 rotationVelocity_range;	// Rot Vel [v.x, v.y]
		float2 timeAlive_interval = float2(1.f, 2.f); // size.x (min), size.y (max)
		float3 velocity_min,
			velocity_max;		  // velocity inside box, between points min and max. (Normalized)
		float2 velocity_interval; // strength of velocity
		float3 acceleration;	  // can be used to produce gravity for particles for example
		float slowdown = 0;
		bool mapSizeToLifetime = false;
		float mapSize_middleFactor = 0; // percentage in lifetime when size is largest 
		bool mapAlphaToLifetime = false;
		float mapAlpha_middleFactor = 0; // percentage in lifetime when alpha is full opaque 
		enum Shape { Circle, Star, Sprite } shape = Shape::Circle;
		enum DrawMode {
			Opaque,
			AlphaBlend,
			NonPremultiplied,
			Additive,
			Subtractive,
			Multiply
		} drawMode = DrawMode::Opaque;
		string str_sprite = "";
		bool sort = false;

		bool imgui_properties();
		void write(ofstream& file) const;
		void read(ifstream& file);
		bool load(string psdName);
		bool save() const;
		bool save(string psdName) const;
		ParticleDescription();
	};

	struct Particle {
		float3 position;
		float rotation = 0;
		float4 color = float4(1.);
		float size = 1; // size after fade
		float isActive = false;
	};

private:
	static vector<shared_ptr<ParticleDescription>> m_descriptionList;

	// description
	shared_ptr<ParticleDescription> m_particle_description;

	bool m_affectedByWind = false;
	float m_emitRate = 0;
	size_t m_capacity = 0;
	bool m_isActive = true;
	bool m_isEmitting = true;
	float m_emitTimer = 0;

	// particle properties
	struct ParticleProperty { // not going to the GPU
		float3 velocity;	  // current velocity
		float lifeTime = 1;
		float timeLeft = 1;
		float size = 1; // start size
		float rotationVelocity = 0;
		float4 startColor = float4(1.f);
	};
	vector<ParticleProperty> m_particleProperties;

	vector<Particle> m_particles;
	vector<Particle> m_sortedParticles;

	// shaders
	static ShaderSet m_shaderSetCircle;
	static ShaderSet m_shaderSetStar;
	static ShaderSet m_shaderSetSprite;

	// vertex Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	shared_ptr<TextureSet> m_tex_particle;

	// -- Functions --

	void createShaders();

	void resizeBuffer();
	void bindVertexBuffer();

	void updateEmits(float dt);

	void updateParticles(float dt);

	void setParticle(size_t index);

	void syncSystemFromDescription();

	static string asPath(string psFilename);

public:
	static void ReadDescriptionList();
	static vector<shared_ptr<ParticleDescription>>* GetDescriptionList();
	static bool GetDesc(string psdName, shared_ptr<ParticleDescription>& pointer);

	void setEmitingState(bool state);
	bool isEmiting() const;
	void setActiveState(bool state);
	bool isActive() const;
	void setAffectedByWindState(bool state);
	bool isAffectedByWind() const;
	size_t getActiveParticleCount() const;

	bool setDesc(string psdName);
	bool setDesc(size_t index);
	void setCustomDescription(ParticleSystem::ParticleDescription& desc);
	shared_ptr<ParticleSystem::ParticleDescription> getDesc();

	void setEmitRate(float emitRate, bool resize = true);
	float getEmitRate() const;
	void setCapacity(size_t capacity = 0);
	size_t getCapacity() const;

	void update(float dt);
	void emit(size_t count);

	void draw(bool alpha = true);

	void load(string psFilename, float emitRate, size_t capacity = 0);

	void imgui_properties();

	ParticleSystem();
};