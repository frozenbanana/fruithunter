#pragma once
#include <SimpleMath.h>
#include <WICTextureLoader.h>
#include "MeshHandler.h"
#include "ShaderSet.h"
#include "Renderer.hpp"

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
	std::vector<Part> parts;
	std::vector<Vertex> mesh;
	std::vector<Vertex_Material> mesh_materials;
	std::vector<MaterialPart> materials;
	//bounding box
	static std::vector<Vertex> box;
	//buffers
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* vertexMaterialBuffer = nullptr;
	static ID3D11Buffer* materialBuffer;
	static ID3D11Buffer* vertexBuffer_BoundingBox;
	ID3D11ShaderResourceView*** maps = nullptr;
	//functions
	std::wstring s2ws(const std::string& s)
	{

		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	bool findMinMaxValues();

	void updateBoundingBoxBuffer();
	void loadBoundingBox();
	void createBuffers(bool instancing = false);
	void freeBuffers();
	int findMaterial(std::string name) const;
public:
	const std::vector<Vertex>& getVertexPoints() const;
	std::string getName()const;

	void draw();
	void draw_noMaterial(float3 color = float3(1,1,1));
	void draw_BoundingBox();
	void draw_forShadowMap();
	void bindMesh()const;

	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;

	bool load(std::string filename, bool combineParts = true, bool instancing = false);

	float castRayOnMesh(float3 rayPos, float3 rayDir);
	static float triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2);
	static float obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale);

	Mesh(std::string OBJFile = "");
	Mesh& operator=(const Mesh& other);
	~Mesh();
};
