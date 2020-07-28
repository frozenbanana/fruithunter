#include "RPYCamera.h"

float3 RPYCamera::getRotation() const { return m_rotation; }

void RPYCamera::setRotation(float3 rotation) { 
	m_rotation = rotation;
	m_rotation.x = Clamp(m_rotation.x, -3.1415f / 2.f, 3.1415f / 2.f);
	float4x4 rotMat = float4x4::CreateRotationZ(m_rotation.z) *
					  float4x4::CreateRotationX(m_rotation.x) *
					  float4x4::CreateRotationY(m_rotation.y);
	float3 dir = float3::Transform(float3(0,0,1), rotMat);
	dir.Normalize();
	setTarget(getPosition() + dir);
}

void RPYCamera::rotate(float3 rotation) { setRotation(m_rotation+rotation); }
