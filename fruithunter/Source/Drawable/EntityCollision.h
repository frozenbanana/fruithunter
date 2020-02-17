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
		float3 m_axis[3];
		float3 m_halfSize;
		ObbData(float3 point, float3 halfSize) {
			m_point = point;
			m_halfSize = halfSize;
			m_axis[0] = float3::Right;
			m_axis[1] = float3::Up;
			m_axis[2] = float3::Forward;
		}
		float3 closestPtPointOBB(float3 point);
	};

	CollisionType m_collisionType;
	unique_ptr<CollisionData> m_collisionData;
	bool collisionSphereSphere(SphereData* sphere1, SphereData* sphere2);
	bool collisionOBBOBB(ObbData& obb1, ObbData& obb2);
	bool collisionSphereOBB(SphereData& sphere, ObbData& obb);


public:
	EntityCollision(float3 point = float3(0.f), float radius = 0.5f); // default is sphere
	EntityCollision(float3 point, float3 halfSizes);				  // default is sphere
	~EntityCollision();
	void setCollisionData(float3 point, float radius);	  // sphere
	void setCollisionData(float3 point, float3 halfSize); // OBB
	void rotateObbAxis(float4x4 matRotation);
	bool collide(EntityCollision& other);
	void setCollisionPosition(float3 pos);
};