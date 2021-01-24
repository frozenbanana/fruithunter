#pragma once
#include "GlobalNamespaces.h"
#include "Mesh.h"
#include "Animated.h"
#include "EntityCollision.h"
#include "MeshRepository.h"
#include "Transformation.h"
#include "Fragment.h"

#define MODEL_MATRIX_BUFFER_SLOT 0

class Entity : public Transformation, public Fragment {
private:
	shared_ptr<Mesh> m_mesh;
	bool m_visible = true;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_modelMatrixBuffer;

	EntityCollision m_collisionData;

	//-- Private Functions --

	bool isMeshInitialized() const;
	void setCollisionForMesh(string meshName);

protected:
	Animated m_meshAnim;
	int m_currentMaterial;
	bool atOrUnder(float terrainHeight) const;
	void setMaterial(int materialIndex);

public:
	string getModelName() const;
	float3 getLocalBoundingBoxPosition() const;
	float3 getLocalBoundingBoxSize() const;

	// Transformations
	void setPosition(float3 position);
	void setRotation(float3 rotation);
	void setScale(float3 scale);
	void setScale(float scale);
	void isVisible(bool state);

	// Drawing
	virtual void draw(float3 color = float3(1.));
	void draw_onlyMesh(float3 color);
	void draw_boundingBox();

	void draw_animate(float3 color = float3(1.));
	void draw_animate_onlyMesh(float3 color = float3(1.f));

	virtual void updateAnimated(float dt);
	virtual void updateAnimatedSpecific(float frameTime);
	void setFrameTargets(int first, int second);
	bool load(string filename);
	bool loadAnimated(string filename, int nrOfFrames);
	void setCurrentMaterial(int materialIndex);
	void loadMaterials(std::vector<string> fileNames);

	float castRay(float3 point, float3 direction);
	bool castRayEx(float3 point, float3 direction, float3& intersection, float3& normal);
	bool castRayEx_limitDistance(
		float3 point, float3 direction, float3& intersection, float3& normal);

	// Collisions
	bool checkCollision(float3 point);
	bool checkCollision(Entity& other);
	bool checkCollision(EntityCollision& other);
	void setCollisionData(float3 point, float3 posOffset, float3 scale, float radius);
	void setCollisionData(float3 point, float3 posOffset, float3 scale, float3 halfSizes);
	void setCollisionDataOBB();
	void setCollisionDataSphere(); // Sets with radius as boundingbox size Y
	void setCollisionDataTree();
	void setCollidable(bool collidable = false);

	float3 getHalfSizes() const;
	float3 getBoundingBoxPos() const;
	int getCollisionType() const;
	float3 getPointOnOBB(float3 point) const;
	bool getIsCollidable() const;

	Entity(string meshName = "", float3 position = float3(0, 0, 0), float3 scale = float3(1, 1, 1));
	~Entity();
};
