#pragma once
#include "Mesh.h"
#include "Timer.hpp"
#include <stdlib.h>
#define ANIMATION_BUFFER_SLOT 3
class Animated {
private:
	std::vector<Mesh> m_meshes;
	int m_nrOfMeshes;
	float m_frameTimer;
	Timer m_timer;

	ShaderSet m_shaderObject_animation;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_animationBuffer;

	void bindMeshes();
	void createInputAssembler();
	void createAnimationConstantBuffer();
	void bindConstantBuffer();

public:
	Animated();
	~Animated();

	void update();
	void draw();
	bool load(std::string filename, int nrOfFrames = 1, bool combineParts = true);
};
