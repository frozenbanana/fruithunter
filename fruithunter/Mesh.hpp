#pragma once
#include "MeshHandler.hpp"
#include "ShaderSet.hpp"

#define COLOR_BUFFER_SLOT 2
#define MATERIAL_BUFFER_SLOT 2

class Mesh {
private:
	static ShaderSet m_shaderObject;
	static ShaderSet m_shaderObject_onlyMesh;

	std::string m_loadedMeshName = "";
	MeshHandler m_handler;

	bool m_minmaxChanged = false;
	float2 m_MinMaxXPosition = float2(-1.f, -1.f); //.x is min, .y is max
	float2 m_MinMaxYPosition = float2(-1.f, -1.f);
	float2 m_MinMaxZPosition = float2(-1.f, -1.f);

	std::vector<Part> m_parts; // describes what parts exists and what material they each use
	std::vector<Vertex> m_meshVertices; // vertices of mesh, position, uv, normal
	std::vector<Material> m_materials;
	// bounding box
	static std::vector<Vertex> m_boxVertices;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer_BoundingBox;
	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	// color buffer
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_colorBuffer;

	// FUNCTIONS
	void bindMesh() const;
	bool findMinMaxValues();
	// bounding box functions
	void updateBoundingBoxBuffer();
	void loadBoundingBox();
	// buffer functions
	void createBuffers(bool instancing = false);
	// intersection
	static float triangleTest(
		float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2);
	static float obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale);

public:
	const std::vector<Vertex>& getVertexPoints() const;
	const Microsoft::WRL::ComPtr<ID3D11Buffer> getVertexBuffer() const;
	std::string getName() const;

	void draw();
	void draw_noMaterial(float3 color = float3(1, 1, 1));
	void draw_BoundingBox();
	void draw_forShadowMap();
	void draw_withoutBinding();

	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;

	bool load(std::string filename, bool combineParts = true);

	float castRayOnMesh(float3 rayPos, float3 rayDir);

	Mesh(std::string OBJFile = "");
	Mesh& operator=(const Mesh& other) = delete;
	~Mesh();
};
