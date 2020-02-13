#include "EntityCollision.h"

bool EntityCollision::collisionSphereSphere(SphereData* sphere1, SphereData* sphere2) {
	float dist = (sphere1->m_point - sphere2->m_point).LengthSquared();
	float radSum = pow(sphere1->m_radius + sphere2->m_radius, 2);
	return dist < radSum;
}


EntityCollision::EntityCollision(float3 point, float radius) {
	setCollisionData(ctSphere, point, radius);
}

EntityCollision::~EntityCollision() {}

void EntityCollision::operator=(EntityCollision& other) {
	m_collisionType = other.m_collisionType;
	m_collisionData.swap(other.m_collisionData);
}

void EntityCollision::setCollisionData(CollisionType type, float3 point, float radius) {
	m_collisionType = type;
	m_collisionData = make_unique<SphereData>(point, radius);
}

bool EntityCollision::collide(EntityCollision& other) {
	bool collides = false;
	switch (m_collisionType) {
	case ctSphere:
		switch (other.m_collisionType) {
		case ctSphere:
			collides = collisionSphereSphere(
				(SphereData*)m_collisionData.get(), (SphereData*)other.m_collisionData.get());
			break;
		case ctOBB:
			// collides = collisionSphere_OBB(other);
			break;
		case ctSimpleMesh:
			break;
		}
		break;
	case ctOBB:
		// switch (other.m_collisionType) {
		// case ctSphere:
		//	// collides = collisionSphere_OBB(other);
		//	break;
		// case ctOBB:
		//	// collides = collisionOBB_OBB(other);
		//	break;
		// case ctSimpleMesh:
		// break;
		//}
		break;
	case ctSimpleMesh:
		// switch (other.m_collisionType) {
		// case ctSphere:
		// case ctOBB:
		// case ctSimpleMesh:
		//}
		break;
	}
	return collides;
}

void EntityCollision::setCollisionPosition(float3 pos) {
	if (m_collisionData->m_point != pos)
		m_collisionData->m_point = pos;
}
