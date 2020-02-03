#pragma once
#include "Mesh.h"
class Animated {
private:
	std::vector<Mesh> m_meshes;
	float m_frameTimer;
	int m_nrOfMeshes;

	ShaderSet m_shaderObject_animation;

	void bindMeshes();
	void createInputAssembler();

public:
	Animated();
	~Animated();

	void draw();
	bool load(std::string filename, int nrOfFrames = 1, bool combineParts = true);
};
