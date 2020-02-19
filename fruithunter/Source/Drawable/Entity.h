#pragma once
#include "GlobalNamespaces.h"
#include "Mesh.h"
#include "Animated.h"
#include "EntityCollision.h"
#include "MeshRepository.h"

#define MODEL_MATRIX_BUFFER_SLOT 0

class Entity {
private:
	float3 m_rotation;
	float3 m_scale;
	float4x4 m_matRotation;
	bool m_matrixChanged = false; // if position, rotation or scale is changed then the model matrix
								  // is updated when it is fetched

	shared_ptr<Mesh> m_mesh;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_modelMatrixBuffer;

	EntityCollision m_collisionData;

	struct MatrixBuffer {
		float4x4 matWorld, matInvTraWorld;
	} m_matrixBufferData; // matrix for translation, rotation and scale

	void updateMatrix();
	void bindModelMatrixBuffer();
	void createBuffers();
	bool isMeshInitialized() const;

protected:
	float3 m_position;
	Animated m_meshAnim;

	bool atOrUnder(float terrainHeight) const;


public:
	float4x4 getModelMatrix();
	float3 getPosition() const;
	float3 getRotation() const;
	float3 getScale() const;

	// Transformations
	void setPosition(float3 position);
	void move(float3 movement);
	void setRotationByAxis(float3 axis, float angle);
	void setRotation(float3 rotation);
	void rotate(float3 rotate);
	void rotateX(float val);
	void rotateY(float val);
	void rotateZ(float val);
	void setScale(float3 scale);
	void setScale(float scale);
	void scaleBoundingBoxHalfSizes(float3 scale);

	// Drawing
	void draw();
	void draw_onlyMesh(float3 color);
	void draw_boundingBox();

	void draw_animate();
	virtual void updateAnimated(float dt);
	virtual void updateAnimatedSpecific(float frameTime);
	void setFrameTargets(int first, int second);
	bool load(string filename);
	bool loadAnimated(string filename, int nrOfFrames);

	float castRay(float3 point, float3 direction);

	// Collisions
	bool checkCollision(Entity& other);
	bool checkCollision(EntityCollision& other);
	void setCollisionData(float3 point, float3 posOffset, float3 scale, float radius);
	void setCollisionData(float3 point, float3 posOffset, float3 scale, float3 halfSizes);
	void setCollisionDataOBB();
	void setCollisionDataSphere(); // Sets with radius as boundingbox size Y
	float3 getHalfSizes() const;
	float3 getBoundingBoxPos() const;
	int getCollisionType() const;

	Entity(string meshName = "", float3 position = float3(0, 0, 0),
		float3 rotation = float3(0, 0, 0), float3 scale = float3(1, 1, 1));
	~Entity();
};
