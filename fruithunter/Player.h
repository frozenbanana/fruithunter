#pragma once
#include "Input.h"
#include "Camera.h"


class Player {
public:
	Player();
	~Player();
	void initialize();
	void update(float td, float height, float3 normal = float3(0, 0, 0));
	void movePlayer();
	void rotatePlayer();
	void draw();

	float3 getPosition() const;
	float3 getForward() const;
	void setPosition(float3 position);

private:
	const float FACTORSTEPS = 0.1f;
	const float3 DEFAULTFORWARD = float3(0.0f, 0.0f, 1.0f);
	const float3 DEFAULTRIGHT = float3(1.0f, 0.0f, 0.0f);
	const float3 DEFAULTUP = float3(0.0f, 1.0f, 0.0f);

	Camera m_camera;
	float3 m_position;
	float3 m_velocity;

	float m_gravity;
	float m_speed;
	float m_velocityFactorFrontBack;
	float m_velocityFactorStrafe;
	float m_groundHeight;
	float m_dashCooldown;

	float3 m_playerForward;
	float3 m_playerRight;
	float3 m_playerUp;
	float m_cameraPitch, m_cameraYaw;

	//- - - Functions - - -
	void jump();
	bool onGround();
	void dash();
};
