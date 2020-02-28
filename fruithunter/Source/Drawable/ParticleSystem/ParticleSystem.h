#pragma once
#include "particle.h"
#include "ShaderSet.h"
#include "Timer.h"
#define MAX_PARTICLES 100

class ParticleSystem {
private:
	float m_timePassed;
	vector<Particle> m_particles;
	size_t m_nrOfParticles;
	static ShaderSet m_shaderSet;

	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_colorBuffer;

	void createBuffers();
	void bindBuffers();
	void initialize();

public:
	ParticleSystem(size_t nrOfParticles = 90);
	void update(float dt);
	void draw();
};
