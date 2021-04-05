#pragma once
#include "Mesh.h"
#include "MeshRepository.h"

#define NR_OF_MESHES_TO_SEND 2 // for now 2. May change later if we want to
// interpolate between more meshes at the same time

class Animated {
private:
	const int ANIMATED_SLOT = 3;

	std::vector<shared_ptr<Mesh>> m_meshes;
	int m_nrOfMeshes;
	int m_frameTargets[2];
	float m_frameTimer;

	ShaderSet m_shaderObject_animation;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_animationBuffer;

	void bindMeshes();
	void createInputAssembler();
	void createAnimationConstantBuffer();
	void bindConstantBuffer();

public:
	Animated();
	~Animated();

	void setMaterials(int index);
	void setFrameTargets(int first, int second);
	void setFrameTimer(float timer);
	float getFrameTimer();

	void update(float dt);
	void updateSpecific(float frameTime); // or deltaFrameTime

	void draw(float3 color = float3(1.));
	void draw_onlyMesh(float3 color = float3(1.));

	bool load(std::string filename, int nrOfFrames = 1, bool combineParts = true);
	void loadMaterials(std::vector<string> fileNames);

	float3 getBoundingBoxHalfSizes() const;
	float3 getBoundingBoxPos() const;
};
