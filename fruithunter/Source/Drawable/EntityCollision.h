#pragma once
#include "GlobalNamespaces.h"

class EntityCollision {
public:
	enum CollisionType { ctSphere, ctOBB, ctSimpleMesh };

private:
	class CollisionData {
	public:
		float3 m_origin; // The position of the entity
		float3 m_posOffset;
		float3 m_point; // The point to use in calculations (origin + rotated offset)
		float3 m_scale;
		virtual ~CollisionData() {}
	};
	class SphereData : public CollisionData {
	public:
		float m_radius;
		SphereData(float3 point, float3 posOffset, float3 scale, float radius) {
			m_origin = point;
			m_posOffset = posOffset;
			m_scale = scale;
			m_point = point + posOffset * scale;

			m_radius = radius;
		}
	};
	class ObbData : public CollisionData {
	public:
		float3 m_axis[3];
		float3 m_halfSize;
		ObbData(float3 point, float3 posOffset, float3 scale, float3 halfSize) {
			m_origin = point;
			m_posOffset = posOffset;
			m_scale = scale;
			m_point = point + posOffset * scale;

			m_halfSize = halfSize;
			m_axis[0] = float3::Right;
			m_axis[1] = float3::Up;
			m_axis[2] = float3::Forward;
		}
		float3 closestPtPointOBB(float3 point) const;
	};

	CollisionType m_collisionType;
	unique_ptr<CollisionData> m_collisionData;
	bool m_collidable;

	bool collisionSphereSphere(SphereData* sphere1, SphereData* sphere2);
	bool collisionOBBOBB(ObbData& obb1, ObbData& obb2);
	bool collisionSphereOBB(SphereData& sphere, ObbData& obb);

public:
	EntityCollision(const EntityCollision& other);
	EntityCollision(float3 point = float3(0.f), float3 posOffset = float3(0.f),
		float3 scale = float3(1.f),
		float radius = 0.5f); // default is sphere
	EntityCollision(float3 point, float3 posOffset, float3 scale, float3 halfSizes);
	~EntityCollision();

	EntityCollision& operator=(const EntityCollision& other);

	void setCollisionData(float3 point, float3 posOffset, float3 scale, float radius);	  // sphere
	void setCollisionData(float3 point, float3 posOffset, float3 scale, float3 halfSize); // OBB
	void rotateObbAxis(float4x4 matRotation);
	bool collide(EntityCollision& other);
	void setCollisionPosition(float3 pos);
	void setCollisionScale(float3 scale);
	void setCollidable(bool collidable = false);

	int getCollisionType() const;
	float3 getClosestPointOnBox(float3 point) const;
	bool getIsCollidable() const;
};