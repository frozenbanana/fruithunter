#pragma once
#include "MeshHandler.h"
#include "ShaderSet.h"

class Mesh {
public:
	enum BoxIntersection { OutsideMiss = 0, OutsideHit = 1, InsideHit = 2 };

private:
	const int COLOR_SLOT = 8;
	const int MATERIAL_SLOT = 2;

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
	std::vector<std::vector<Material>> m_materials;
	int m_currentMaterial;
	// bounding box
	std::vector<Vertex> m_boxVertices;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer_BoundingBox;
	// vertex buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	// color buffer
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_colorBuffer;

	// FUNCTIONS

	bool findMinMaxValues();
	// bounding box functions
	void updateBoundingBoxBuffer();
	void loadBoundingBox();
	// buffer functions
	void createBuffers(bool instancing = false);
	// intersection
	static float triangleTest(
		float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2);
	static BoxIntersection obbTest(
		float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale, float& t);

public:
	// -- GETS --
	int getVertexCount() const;
	const std::vector<Vertex>& getVertexPoints() const;
	const Microsoft::WRL::ComPtr<ID3D11Buffer> getVertexBuffer() const;
	std::string getName() const;
	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;
	float3 getBoundingBoxHalfSizes() const; // for collision detection

	// -- SETS --
	void setMaterialIndex(int material);

	// -- BINDS --
	void bindMaterial(int index);
	void bindColorBuffer(float3 color);
	void bindMesh() const;

	// -- DRAW CALLS
	void drawCall_all();
	void drawCall_perMaterial();

	void draw(float3 color = float3(1.));
	void draw_noMaterial(float3 color = float3(1, 1, 1));
	void draw_BoundingBox();

	// -- ANALYSE --
	float castRayOnMesh(float3 rayPos, float3 rayDir);
	bool castRayOnMeshEx(float3 rayPos, float3 rayDir, float3& intersection, float3& normal);
	bool castRayOnMeshEx_limitDistance(float3 rayPos, float3 rayDir, float3& intersection, float3& normal);

	// -- INITILIZE --
	bool load(std::string filename, bool combineParts = true);
	void loadOtherMaterials(std::vector<string> fileNames);

	Mesh(std::string OBJFile = "");
	Mesh& operator=(const Mesh& other) = delete;
	~Mesh();
};
