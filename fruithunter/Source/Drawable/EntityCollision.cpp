#include "EntityCollision.h"
#include "ErrorLogger.h"

bool EntityCollision::collisionSphereSphere(SphereData* sphere1, SphereData* sphere2) {
	float dist = (sphere1->m_point - sphere2->m_point).LengthSquared();
	float radSum =
		pow(sphere1->m_radius * sphere1->m_scale.y + sphere2->m_radius * sphere2->m_scale.y,
			2); // Assume scale is uniform
	return dist < radSum;
}

void vecToArray(float arr[], float3 vec) {
	arr[0] = vec.x;
	arr[1] = vec.y;
	arr[2] = vec.z;
}

bool EntityCollision::collisionOBBOBB(ObbData& a, ObbData& b) {
	// "Borrowed" from ch 4.4.1 "real-time collision detecton" - Christer Ericson

	// TODO: Sätt om float3 i Obbdata till float[3]
	// och lägg till vector functioner som Dot istället.
	float ra, rb;
	XMFLOAT3X3 R, AbsR;

	float ac[3]; // c = center
	float bc[3];
	float ae[3]; // e = halfsizes
	float be[3];
	vecToArray(ac, a.m_point);
	vecToArray(bc, b.m_point);
	vecToArray(ae, a.m_halfSize * a.m_scale); // element-wise multiplication
	vecToArray(be, b.m_halfSize * b.m_scale);

	// Compute rotation matrix expressing b in a’s coordinate frame
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			R.m[i][j] = a.m_axis[i].Dot(b.m_axis[j]);

	// Compute translation vector t
	float3 tv = b.m_point - a.m_point;
	float t[3];

	// Bring translation into a’s coordinate frame
	tv = float3(tv.Dot(a.m_axis[0]), tv.Dot(a.m_axis[1]), tv.Dot(a.m_axis[2]));
	vecToArray(t, tv);

	// Compute common subexpressions. Add in an epsilon term to
	// counteract arithmetic errors when two edges are parallel and
	// their cross product is (near) null (see text for details)
	const float EPSILON = 0.00001f;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			AbsR.m[i][j] = abs(R.m[i][j]) + EPSILON;

	// Test axes L = A0, L = A1, L = A2
	for (int i = 0; i < 3; i++) {
		ra = ae[i];
		rb = be[0] * AbsR.m[i][0] + be[1] * AbsR.m[i][1] + be[2] * AbsR.m[i][2];
		if (abs(t[i]) > ra + rb)
			return 0;
	}

	// Test axes L = B0, L = B1, L = B2
	for (int i = 0; i < 3; i++) {
		ra = ae[0] * AbsR.m[0][i] + ae[1] * AbsR.m[1][i] + ae[2] * AbsR.m[2][i];
		rb = be[i];
		if (abs(t[0] * R.m[0][i] + t[1] * R.m[1][i] + t[2] * R.m[2][i]) > ra + rb)
			return 0;
	}

	// Test axis L = A0 x B0
	ra = ae[1] * AbsR.m[2][0] + ae[2] * AbsR.m[1][0];
	rb = be[1] * AbsR.m[0][2] + be[2] * AbsR.m[0][1];
	if (abs(t[2] * R.m[1][0] - t[1] * R.m[2][0]) > ra + rb)
		return 0;

	// Test axis L = A0 x B1
	ra = ae[1] * AbsR.m[2][1] + ae[2] * AbsR.m[1][1];
	rb = be[0] * AbsR.m[0][2] + be[2] * AbsR.m[0][0];
	if (abs(t[2] * R.m[1][1] - t[1] * R.m[2][1]) > ra + rb)
		return 0;

	// Test axis L = A0 x B2
	ra = ae[1] * AbsR.m[2][2] + ae[2] * AbsR.m[1][2];
	rb = be[0] * AbsR.m[0][1] + be[1] * AbsR.m[0][0];
	if (abs(t[2] * R.m[1][2] - t[1] * R.m[2][2]) > ra + rb)
		return 0;

	// Test axis L = A1 x B0
	ra = ae[0] * AbsR.m[2][0] + ae[2] * AbsR.m[0][0];
	rb = be[1] * AbsR.m[1][2] + be[2] * AbsR.m[1][1];
	if (abs(t[0] * R.m[2][0] - t[2] * R.m[0][0]) > ra + rb)
		return 0;

	// Test axis L = A1 x B1
	ra = ae[0] * AbsR.m[2][1] + ae[2] * AbsR.m[0][1];
	rb = be[0] * AbsR.m[1][2] + be[2] * AbsR.m[1][0];
	if (abs(t[0] * R.m[2][1] - t[2] * R.m[0][1]) > ra + rb)
		return 0;

	// Test axis L = A1 x B2
	ra = ae[0] * AbsR.m[2][2] + ae[2] * AbsR.m[0][2];
	rb = be[0] * AbsR.m[1][1] + be[1] * AbsR.m[1][0];
	if (abs(t[0] * R.m[2][2] - t[2] * R.m[0][2]) > ra + rb)
		return 0;

	// Test axis L = A2 x B0
	ra = ae[0] * AbsR.m[1][0] + ae[1] * AbsR.m[0][0];
	rb = be[1] * AbsR.m[2][2] + be[2] * AbsR.m[2][1];
	if (abs(t[1] * R.m[0][0] - t[0] * R.m[1][0]) > ra + rb)
		return 0;

	// Test axis L = A2 x B1
	ra = ae[0] * AbsR.m[1][1] + ae[1] * AbsR.m[0][1];
	rb = be[0] * AbsR.m[2][2] + be[2] * AbsR.m[2][0];
	if (abs(t[1] * R.m[0][1] - t[0] * R.m[1][1]) > ra + rb)
		return 0;

	// Test axis L = A2 x B2
	ra = ae[0] * AbsR.m[1][2] + ae[1] * AbsR.m[0][2];
	rb = be[0] * AbsR.m[2][1] + be[1] * AbsR.m[2][0];
	if (abs(t[1] * R.m[0][2] - t[0] * R.m[1][2]) > ra + rb)
		return 0;

	// Since no separating axis is found, the OBBs must be intersecting
	return 1;
}

bool EntityCollision::collisionSphereOBB(SphereData& sphere, ObbData& obb) {
	float3 closestOnOBB = obb.closestPtPointOBB(sphere.m_point);
	float distSq = (closestOnOBB - sphere.m_point).LengthSquared();

	return distSq < sphere.m_radius * sphere.m_radius * sphere.m_scale.y * sphere.m_scale.y;
}

EntityCollision::EntityCollision(float3 point, float3 posOffset, float3 scale, float radius) {
	setCollisionData(point, posOffset, scale, radius);
	m_collidable = true;
}

EntityCollision::EntityCollision(float3 point, float3 posOffset, float3 scale, float3 halfSizes) {
	setCollisionData(point, posOffset, scale, halfSizes);
	m_collidable = true;
}

EntityCollision::~EntityCollision() {}


void EntityCollision::setCollisionData(float3 point, float3 posOffset, float3 scale, float radius) {
	m_collisionType = ctSphere;
	m_collisionData = make_unique<SphereData>(point, posOffset, scale, radius);
}

void EntityCollision::setCollisionData(
	float3 point, float3 posOffset, float3 scale, float3 halfSize) {
	m_collisionType = ctOBB;
	m_collisionData = make_unique<ObbData>(point, posOffset, scale, halfSize);
}

void EntityCollision::rotateObbAxis(float4x4 matRotation) {
	if (m_collisionType == ctOBB) {
		((ObbData*)m_collisionData.get())->m_axis[0] =
			float3::Transform(float3::Right, matRotation);
		((ObbData*)m_collisionData.get())->m_axis[1] = float3::Transform(float3::Up, matRotation);
		((ObbData*)m_collisionData.get())->m_axis[2] =
			float3::Transform(float3::Forward, matRotation);

		m_collisionData.get()->m_point =
			m_collisionData->m_origin +
			float3::Transform(m_collisionData->m_posOffset * m_collisionData->m_scale, matRotation);
	}
}

bool EntityCollision::collide(EntityCollision& other) {
	if (!m_collidable || !other.m_collidable)
		return false;

	bool collides = false;
	switch (m_collisionType) {
	case ctSphere:
		switch (other.m_collisionType) {
		case ctSphere:
			collides = collisionSphereSphere(
				(SphereData*)m_collisionData.get(), (SphereData*)other.m_collisionData.get());
			break;
		case ctOBB:
			collides = collisionSphereOBB(
				*(SphereData*)m_collisionData.get(), *(ObbData*)other.m_collisionData.get());
			break;
		}
		break;
	case ctOBB:
		switch (other.m_collisionType) {
		case ctSphere:
			collides = collisionSphereOBB(
				*(SphereData*)other.m_collisionData.get(), *(ObbData*)m_collisionData.get());
			break;
		case ctOBB:
			collides = collisionOBBOBB(
				*(ObbData*)m_collisionData.get(), *(ObbData*)other.m_collisionData.get());
			break;
		}
		break;
	}
	return collides;
}

void EntityCollision::setCollisionPosition(float3 pos) {
	if (m_collisionData->m_origin != pos) {
		float3 diff = pos - m_collisionData->m_origin;
		m_collisionData->m_point += diff;
		m_collisionData->m_origin = pos;
	}
}

void EntityCollision::setCollisionScale(float3 scale) {
	if (m_collisionData->m_scale != scale) {
		float3 scaleFactor = scale / m_collisionData->m_scale;
		float3 diffPos = m_collisionData->m_point - m_collisionData->m_origin;

		m_collisionData->m_point = m_collisionData->m_origin + diffPos * scaleFactor;
		m_collisionData->m_scale = scale;
	}
}

void EntityCollision::setCollidable(bool collidable) { m_collidable = collidable; }

int EntityCollision::getCollisionType() const { return m_collisionType; }

// Returns point on OBB that is closest to a point
float3 EntityCollision::ObbData::closestPtPointOBB(float3 point) const {
	// Theory from ch 5.1.4 "real-time collision detecton" - Christer Ericson
	// x = (P-C).dot(axis[0])
	float3 vec = point - m_point;
	float halfSize[3];
	vecToArray(halfSize, m_halfSize * m_scale);
	float3 pointOnObb = m_point;

	for (size_t i = 0; i < 3; ++i) {
		// get composite distance along axis
		float dist = vec.Dot(m_axis[i]);

		// clamp distance to box size
		dist = max(min(halfSize[i], dist), -halfSize[i]);

		pointOnObb += dist * m_axis[i];
	}

	return pointOnObb;
}

float3 EntityCollision::getClosestPointOnBox(float3 point) const {
	if (m_collisionType == ctOBB) {
		return ((ObbData*)m_collisionData.get())->closestPtPointOBB(point);
	}
	else {
		ErrorLogger::log("Called getClosestPointOnBox in a sphere!");
		return float3(-1.f);
	}
}

bool EntityCollision::getIsCollidable() const { return m_collidable; }

float3 EntityCollision::getOBBHalfsize() { 
	return m_collisionType == ctOBB ? ((ObbData*)m_collisionData.get())->m_halfSize : float3(0.f); }
