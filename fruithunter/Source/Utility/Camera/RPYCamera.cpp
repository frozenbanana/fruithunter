#include "RPYCamera.h"

float3 RPYCamera::getRotation() const { return m_rotation; }

void RPYCamera::setRotation(float3 rotation) { 
	m_rotation = rotation;
	m_rotation.x = Clamp(m_rotation.x, -XM_PI / 2.f, XM_PI / 2.f);
	float4x4 rotMat = float4x4::CreateRotationZ(m_rotation.z) *
					  float4x4::CreateRotationX(m_rotation.x) *
					  float4x4::CreateRotationY(m_rotation.y);
	float3 dir = float3::Transform(float3(0,0,1), rotMat);
	dir.Normalize();
	Camera::setTarget(getPosition() + dir);
}

void RPYCamera::rotate(float3 rotation) { setRotation(m_rotation + rotation); }

void RPYCamera::setTarget(float3 target) { 
	float3 dir = target - m_positionEye;
	dir.Normalize();

	float2 mapY(dir.z, dir.x);
	float2 mapX(float2(dir.x, dir.z).Length(), dir.y);
	mapY.Normalize();
	mapX.Normalize();
	float rotY = (mapY.y >= 0) ? (acos(mapY.x)) : (-acos(mapY.x));
	float rotX = -((mapX.y >= 0) ? (acos(mapX.x)) : (-acos(mapX.x)));
	setRotation(float3(rotX, rotY, 0));
}
