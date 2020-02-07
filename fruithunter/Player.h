#pragma once
#include "Input.h"
#include "Camera.h"

using namespace DirectX;
using namespace SimpleMath;

const float FACTORSTEPS = 0.1f;
const Vector3 DEFAULTFORWARD = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 DEFAULTRIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 DEFAULTUP = Vector3(0.0f, 1.0f, 0.0f);


class Player {
public:
	Player();
	~Player();
	void initialize();
	void update(float td, float height, float3 normal = float3(0,0,0));
	void movePlayer();
	void rotatePlayer();
	void draw();

	float3 getPosition() const;
	float3 getForward() const;
	void setPosition(float3 position);

private:
	Camera m_camera;
	Vector3 m_position;
	Vector3 m_velocity;
	
	float m_gravity;
	float m_speed;
	float m_velocityFactorFrontBack;
	float m_velocityFactorStrafe;
	float m_groundHeight;
	float m_dashCooldown;

	Vector3 m_playerForward;
	Vector3 m_playerRight;
	Vector3 m_playerUp;
	float m_cameraPitch, m_cameraYaw;
	
//- - - Functions - - -
	void jump();
	bool onGround();
	void dash();
};
