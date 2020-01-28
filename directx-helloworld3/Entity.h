#pragma once
#include "Mesh.h"
class Entity {
private:
	float3 m_position;
	float3 m_rotation;
	float3 m_scale;
	bool m_matrixChanged = false;//if position, rotation or scale is changed then the model matrix is updated when it is fetched
	float4x4 m_modelMatrix;//matrix for translation, rotation and scale

	Mesh m_mesh;

	ID3D11Buffer* m_modelMatrixBuffer = nullptr;

	void updateMatrix();

	void createBuffers();
	void release();

public:
	float4x4 getModelMatrix();
	float3 getPosition() const;
	float3 getRotation() const;
	float3 getScale() const;

	//Transformations
	void setPosition(float3 position);
	void move(float3 movement);
	void setRotation(float3 rotation);
	void rotate(float3 rotate);
	void rotateX(float val);
	void rotateY(float val);
	void rotateZ(float val);
	void setScale(float3 scale);
	void setScale(float scale);

	//Drawing
	void draw();
	void draw_onlyMesh(float3 color);

	bool load(string filename);

	Entity(string meshName = "", float3 position = float3(0,0,0), float3 rotation = float3(0,0,0), float3 scale = float3(1,1,1));
	~Entity();

};
