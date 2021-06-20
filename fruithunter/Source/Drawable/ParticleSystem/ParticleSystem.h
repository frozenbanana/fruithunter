#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#include "Terrain.h"
#include "Fragment.h"
#include "TextureRepository.h"
#include "fstreamLib.h"

#define PATH_PSD "assets/ParticleSystems/"
#define PSD_END "psd" // psd - particle.system.description

class ParticleSystem : public Fragment, public Transformation {
public:
	struct ParticleDescription {
		string identifier;

		float4 colorVariety[3];
		float2 size_interval;	   // size.x (min), size.y (max)
		float2 timeAlive_interval; // size.x (min), size.y (max)
		float3 velocity_min,
			velocity_max;		  // velocity inside box, between points min and max. (Normalized)
		float2 velocity_interval; // strength of velocity
		float3 acceleration;	  // can be used to produce gravity for particles for example
		float slowdown;
		float fadeInterval_start; // time(start) to time(x) seconds to scale particle to real size
		float fadeInterval_end;	  // time(end-x) to time(end) seconds to scale particle to nothing
		string str_sprite;
		enum Shape { Circle, Star, Sprite } shape;
		enum DrawMode { Normal, Add, Sub } drawMode;

		bool imgui_properties();
		void write(ofstream& file) const;
		void read(ifstream& file);
		bool load(string psdName);
		bool save() const;
		bool save(string psdName) const;
		ParticleDescription();
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
	};
	vector<ParticleProperty> m_particleProperties;
	struct Particle {
		float3 position;
		float4 color = float4(1.);
		float size = 1; // full size
		float isActive = false;
	};
	vector<Particle> m_particles;

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