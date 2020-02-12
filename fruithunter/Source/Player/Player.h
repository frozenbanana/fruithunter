#pragma once
#include "Input.h"
#include "Camera.h"
#include "Terrain.h"

#include "Bow.h"

class Player {
public:
	Player();
	~Player();
	void initialize();
	void update(float td, Terrain* terrain);
	void bowUpdate();
	void rotatePlayer();
	void draw();

	float3 getPosition() const;
	float3 getCameraPosition() const;
	float3 getForward() const;
	float3 getVelocity() const;
	void setPosition(float3 position);

private:
	const float FACTORSTEPS = 0.1f;
	const float3 DEFAULTFORWARD = float3(0.0f, 0.0f, 1.0f);
	const float3 DEFAULTRIGHT = float3(1.0f, 0.0f, 0.0f);
	const float3 DEFAULTUP = float3(0.0f, 1.0f, 0.0f);
	const float PLAYERHEIGHT = 1.5f;

	Camera m_camera;
	Bow m_bow;
	float3 m_position;
	float3 m_velocity;

	bool m_onGround;
	bool m_bouncing;
	bool m_sliding;
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
	void groundCheck();
	void bounceCheck(Vector3 normal);
	void slideCheck(Vector3 normal);
	void slide(float td, Vector3 normal, float l);
	void dash();
	void bounce(Vector3 normal, float dt);
	void movement(Vector3 normal, float dt, Vector3 collisionPoint);
	float clamp(float x, float high, float low);
};
