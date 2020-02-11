#pragma once
#include "ShaderSet.h"
#include "GlobalNamespaces.h"
#include "Mesh.h"

class SkyBox {
private:
	ShaderSet m_shaderSkyBox;
	Mesh m_box;

public:
	SkyBox();
	~SkyBox();

	void draw();
	void createShaders();
	
};