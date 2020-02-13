#pragma once
#include "GlobalNamespaces.h"
#include "Mesh.h"
#include "Animated.h"
#include "EntityCollision.h"

#define MODEL_MATRIX_BUFFER_SLOT 0

class Entity {
private:
	float3 m_position;
	float3 m_rotation;
	float3 m_scale;
	bool m_matrixChanged = false; // if position, rotation or scale is changed then the model matrix
								  // is updated when it is fetched

	Mesh m_mesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_modelMatrixBuffer;

	EntityCollision m_collisionData;

	struct MatrixBuffer {
		float4x4 matWorld, matInvTraWorld;
	} m_matrixBufferData; // matrix for translation, rotation and scale

	void updateMatrix();
	void bindModelMatrixBuffer();
	void createBuffers();

	// Collision related
	bool collisionSphere_Sphere(Entity& other);
	bool collisionSphere_OBB(Entity& other);
	bool collisionOBB_OBB(Entity& other);

protected:
	Animated m_meshAnim;

	bool onGround(float height) const;


public:
	float4x4 getModelMatrix();
	float3 getPosition() const;
	float3 getRotation() const;
	float3 getScale() const;

	// Transformations
	void setPosition(float3 position);
	void move(float3 movement);
	void setRotation(float3 rotation);
	void rotate(float3 rotate);
	void rotateX(float val);
	void rotateY(float val);
	void rotateZ(float val);
	void setScale(float3 scale);
	void setScale(float scale);

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
	void setCollisionData(EntityCollision data);
	void setCollisionPosition(float3 pos);

	Entity(string meshName = "", float3 position = float3(0, 0, 0),
		float3 rotation = float3(0, 0, 0), float3 scale = float3(1, 1, 1));
	~Entity();
};
