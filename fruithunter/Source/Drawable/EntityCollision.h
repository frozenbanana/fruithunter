#pragma once
#include "GlobalNamespaces.h"

class EntityCollision {
public:
	enum CollisionType { ctSphere, ctOBB, ctSimpleMesh };

private:
	struct CollisionData {
		float3 m_point;
	};
	struct SphereData : CollisionData {
		float m_radius;
		SphereData(float3 point, float radius) {
			m_point = point;
			m_radius = radius;
		}
	};
	struct ObbData : CollisionData {
		// TODO:
	};

	CollisionType m_collisionType;
	unique_ptr<CollisionData> m_collisionData;
	bool collisionSphereSphere(SphereData* sphere1, SphereData* sphere2);

public:
	EntityCollision(float3 point = float3(0.f), float radius = 0.5f); // default is sphere
	~EntityCollision();
	void operator=(EntityCollision& other);
	void setCollisionData(CollisionType type, float3 point, float radius); // sphere
	bool collide(EntityCollision& other);
	void setCollisionPosition(float3 pos);
};