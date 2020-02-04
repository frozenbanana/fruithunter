#pragma once
#include "Input.hpp"
#include "Camera.hpp"

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
	void update(float td);
	void movePlayer();
	void rotatePlayer();
	void draw();

private:
	Camera m_camera;
	Vector3 m_position;
	Vector3 m_velocity;
	Vector3 m_playerForward;
	float m_gravity;
	float m_speed;
	float m_velocityFactorFrontBack;
	float m_velocityFactorStrafe;

	Vector3 m_playerRight;
	Vector3 m_playerUp;
	float m_cameraPitch, m_cameraYaw;
	
//- - - Functions - - -
	void jump();
	bool onGround();
};
