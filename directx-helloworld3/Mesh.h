#pragma once
#include <WICTextureLoader.h>
#include "MeshHandler.h"
#include "ShaderSet.h"
#include "Renderer.hpp"
#include <SimpleMath.h>

class Mesh
{
private:
	static int meshCount;
	static ShaderSet shader_object;
	static ShaderSet shader_object_onlyMesh;

	std::string loadedMeshName = "";
	MeshHandler handler;

	bool minmaxChanged = false;
	float2 MinMaxXPosition = float2(-1, -1);//.x is min, .y is max
	float2 MinMaxYPosition = float2(-1, -1);
	float2 MinMaxZPosition = float2(-1, -1);

	std::vector<Part> parts;//describes what parts exists and what material they each use
	std::vector<Vertex> mesh;//vertices of mesh, position, uv, normal
	std::vector<VertexMaterialBuffer> mesh_materials;//different way of rendering with material
	std::vector<Material> materials;
	//bounding box
	static std::vector<Vertex> box;
	static ID3D11Buffer* vertexBuffer_BoundingBox;
	//vertex buffers
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* vertexMaterialBuffer = nullptr;
	//color buffer
	static ID3D11Buffer* m_colorBuffer;

	//FUNCTIONS
	void bindMesh() const;
	bool findMinMaxValues();
	//bounding box functions
	void updateBoundingBoxBuffer();
	void loadBoundingBox();
	//buffer functions
	void createBuffers(bool instancing = false);
	void freeBuffers();
	//intersection
	static float triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2);
	static float obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale);

public:
	const std::vector<Vertex>& getVertexPoints() const;
	std::string getName()const;

	void draw();
	void draw_noMaterial(float3 color = float3(1,1,1));
	void draw_BoundingBox();
	void draw_forShadowMap();

	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;

	bool load(std::string filename, bool combineParts = true);

	float castRayOnMesh(float3 rayPos, float3 rayDir);

	Mesh(std::string OBJFile = "");
	//Mesh& operator=(const Mesh& other);
	~Mesh();
};
