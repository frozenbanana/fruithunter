#pragma once
#include "Camera.h"
class RPYCamera : public Camera {
private:
	float3 m_rotation;

public:
	float3 getRotation() const;

	void setRotation(float3 rotation);
	void rotate(float3 rotation);

	void setTarget(float3 target);

};
