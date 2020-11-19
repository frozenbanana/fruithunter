#pragma once
#include "MeshHandler.h"
#include "ShaderSet.h"

#define COLOR_BUFFER_SLOT 2
#define MATERIAL_BUFFER_SLOT 2
#define PLANE_BUFFER_SLOT 7

class Mesh {
private:
	static ShaderSet m_shaderObject;
	static ShaderSet m_shaderObject_onlyMesh;
	static ShaderSet m_shaderObject_clip;

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
	// clipping plane
	struct PlaneBuffer {
		float4 point;
		float4 normal;
		float4 color;
		PlaneBuffer(float3 _point, float3 _normal, float3 _color, float time) { 
			point = float4(_point.x, _point.y, _point.z, time);
			normal = float4(_normal.x, _normal.y, _normal.z, 1);
			color = float4(_color.x, _color.y, _color.z, 1);
		}
	};
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_planeBuffer;

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
	static float obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale);

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

	void draw();
	void draw_clippingPlane(float3 plane_point, float3 plane_normal, float3 plane_color, float time);
	void draw_noMaterial(float3 color = float3(1, 1, 1));
	void draw_BoundingBox();

	// -- ANALYSE --
	float castRayOnMesh(float3 rayPos, float3 rayDir);

	// -- INITILIZE --
	bool load(std::string filename, bool combineParts = true);
	void loadOtherMaterials(std::vector<string> fileNames);

	Mesh(std::string OBJFile = "");
	Mesh& operator=(const Mesh& other) = delete;
	~Mesh();
};
