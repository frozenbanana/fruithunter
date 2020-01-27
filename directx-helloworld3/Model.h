#pragma once
#include "Mesh.h"
class Model : public Mesh {
private:
	//variables for color and normal map
	ID3D11Buffer* gColorBuffer;

	void updateColorBuffer(float3 color);

public:

	void draw();
	void draw_noMaterial();
	void draw_noMaps();

};
