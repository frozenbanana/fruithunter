#pragma once
#include "Fragment.h"
#include "Transformation.h"
#include "SimpleDirectX.h"
#include "TextureRepository.h"
#include "filesystemHelper.h"
#include "fstreamlib.h"

#define PATH_PRESET "assets/ParticleSystems/Presets/"
#define PRESET_END "esp" // Effect System Preset

class EffectSystem : public Transformation, public Fragment {
public:
	enum SubEmitters { Trail, Death, Collision, SubEmitter_Length, Free };
	class ParticleCache;
	class Emitter {
	private:
		bool firstUpdate = true;
		float3 previousPosition;
		float distanceRest = 0;
		float emitTimer = 0;
		shared_ptr<ParticleCache> system;
		clock_t latestBurst = 0;

		bool isLinked() const;

	public:
		void reset();
		void update(float dt, const Transformation& transform);
		void link(shared_ptr<ParticleCache> _system);
		void unlink();
	};
	struct ParticleBuffer {
		float3 position;
		float rotation = 0;
		Color color;
		float size = 1;
		int enabled = false;
	};
	struct Particle {
		ParticleBuffer vars;
		float startSize = 0;
		Color startColor;

		float3 velocity;
		float rotationVelocity = 0;

		float lifetime = 0;
		float timeleft = 0;

		Emitter subEmitters[SubEmitter_Length];
	};
	struct EmitterDescription {
		class SpawnShape {
		private:
			float3 m_offset;
			virtual float3 _getSpawn() const { return float3(0, 0, 0); }
			virtual void _imgui_properties() {}
			virtual void _read(ifstream& file) {}
			virtual void _write(ofstream& file) {}

		public:
			void read(ifstream& file) {
				fileRead(file, m_offset);
				_read(file);
			}
			void write(ofstream& file) {
				fileWrite(file, m_offset);
				_write(file);
			}
			float3 getSpawn() const { return _getSpawn() + m_offset; }
			void imgui_properties() {
				ImGui::InputFloat3("Offset", (float*)&m_offset);
				_imgui_properties();
			}
		};
		class SphereShape : public SpawnShape {
			float2 m_radiusRange = float2(0, 1);
			float3 _getSpawn() const {
				float radius = RandomFloat(m_radiusRange.x, m_radiusRange.y);
				return Normalize(float3(RandomFloat(-1.f, 1.f), RandomFloat(-1.f, 1.f),
						   RandomFloat(-1.f, 1.f))) *
					   radius;
			}
			void _imgui_properties() {
				ImGui::DragFloatRange2(
					"Radius Range", &m_radiusRange.x, &m_radiusRange.y, 0.1, 0, 10, "%.1f", "%.1f");
			}
			void _read(ifstream& file) { fileRead(file, m_radiusRange); }
			void _write(ofstream& file) { fileWrite(file, m_radiusRange); }
		};
		class BoxShape : public SpawnShape {
			float3 m_pointStart = float3(-1.f), m_pointEnd = float3(1.f);
			float3 _getSpawn() const {
				return float3(RandomFloat(m_pointStart.x, m_pointEnd.x),
					RandomFloat(m_pointStart.y, m_pointEnd.y),
					RandomFloat(m_pointStart.z, m_pointEnd.z));
			}
			void _imgui_properties() {
				ImGui::InputFloat3("Start Point", (float*)&m_pointStart);
				ImGui::InputFloat3("End Point", (float*)&m_pointEnd);
			}
			void _read(ifstream& file) {
				fileRead(file, m_pointStart);
				fileRead(file, m_pointEnd);
			}
			void _write(ofstream& file) {
				fileWrite(file, m_pointStart);
				fileWrite(file, m_pointEnd);
			}
		};
		enum SpawnType {
			SpawnType_Point,
			SpawnType_Sphere,
			SpawnType_Box,
			SpawnType_Length
		} spawnType = SpawnType_Point;
		const char* SpawnTypeStr[SpawnType_Length] = { "Point", "Sphere", "Box" };
		shared_ptr<SpawnShape> shapes[SpawnType_Length] = { make_shared<SpawnShape>(),
			make_shared<SphereShape>(), make_shared<BoxShape>() };

		// particle description
		enum EmitType { Constant, Distance, Burst, EmitType_Length } emitType = Constant;
		float emitRate = 2;
		float emitsPerDistance = 5;
		int burstCount = 1;
		vector<Color> colors = { Color(1, 1, 1) };
		bool randomRotation = true;
		float startRotation = 0;	   // start rotation if random rotation is set to false
		float2 rotationVelocity_range; // Rot Vel [v.x, v.y]
		float2 sizeRange = float2(0.2, 0.5f);
		float2 lifetimeRange = float2(1, 2);
		float2 velocityRadiusRange = float2(0, XM_PI);
		float2 velocityIntensityRange = float2(0.2f, 0.5f);
		float3 gravity = float3(0, 0, 0);
		float friction = 0;
		bool mapSizeToLifetime = false;
		float mapSize_middleFactor = 0; // percentage in lifetime when size is largest
		bool mapAlphaToLifetime = false;
		float mapAlpha_middleFactor = 0; // percentage in lifetime when alpha is full opaque
		bool sort = false;
		enum DrawMode {
			Opaque,
			AlphaBlend,
			NonPremultiplied,
			Additive,
			Subtractive,
			Multiply
		} drawMode = DrawMode::Opaque;
		shared_ptr<Texture> texture;

		bool collideWithTerrain = false;
		bool destroyOnCollision = false;
		float collisionBounceIntensity = 1.f;

		static string toString(EmitType type);
		void imgui_properties(SubEmitters type);
		void read(ifstream& file);
		void write(ofstream& file);
		EmitterDescription();
	};
	class ParticleCache {
	private:
		SubEmitters m_type = Free;

		// tree structure
		ParticleCache* m_parentCache = nullptr;

		shared_ptr<ParticleCache> m_subCaches[SubEmitter_Length];

		// data
		vector<Particle> m_particles;
		VertexBuffer<ParticleBuffer> m_buffer; // contains buffer data

		bool m_active = true;
		bool m_isEmitting = true;
		float m_emitTimer = 0;
		EmitterDescription m_description;

		void refreshParticle(size_t index, const Transformation& transform);
		void killParticle(size_t index, float dt);

		void updateParticles(float dt);

		void bindVertexBuffer() {
			auto dc = Renderer::getDeviceContext();
			UINT strides = sizeof(ParticleBuffer);
			UINT offset = 0;
			dc->IASetVertexBuffers(0, 1, m_buffer.GetAddressOf(), &strides, &offset);
			dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		}

		bool adjustSize();

		void addSubCache(SubEmitters emitterType);
		void delSubCache(SubEmitters emitterType);

	public:
		static string toString(SubEmitters type);
		EmitterDescription getDescription() const;
		size_t getParticleCount() const;
		size_t getActiveParticleCount() const;
		size_t getLocalActiveParticleCount() const;
		bool isEmitting() const;
		void setEmittingState(bool state);
		bool hasSubEmitters() const;
		void setCacheType(SubEmitters type);
		void setParentCache(ParticleCache* cache);
		void unlinkParticlesToCache(SubEmitters emitTypeCache);

		void emit(size_t count, const Transformation& transform);
		void burst(const Transformation& transform);

		void update(float dt);

		void draw();

		void imgui_properties();
		void imgui_tree(ParticleCache** selected);

		void read(ifstream& file);
		void write(ofstream& file);

		void clear();

		ParticleCache();
	};

private:
	static bool m_fetchedPresets;
	static vector<string> m_presets;
	string m_previousPresetLoaded = "";
	string m_tempStoreString = "";

	struct SourceCache {
		shared_ptr<ParticleCache> source;
		Emitter emitter;
		SourceCache() {
			source = make_shared<ParticleCache>();
			emitter.link(source);
		}
	};
	vector<SourceCache> m_caches;

	ParticleCache* m_selected = nullptr;

	static ShaderSet m_shader;

	static string asPath(string filename);
	static void fetchPresets();

	void read_preset(ifstream& file);
	void write_preset(ofstream& file);

	void emit(size_t count);

public:
	size_t getActiveParticleCount() const;
	void setEmittingState(bool state);
	bool isEmitting() const;

	void update(float dt);
	void burst();

	void draw();

	void imgui_properties();

	void read(ifstream& file);
	void write(ofstream& file);
	bool loadFromPreset(string filename);
	bool storeAsPreset(string filename);

	EffectSystem();
};