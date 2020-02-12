#include "Player.h"
#include "Renderer.h"
#include "Errorlogger.h"

Player::Player() {}

Player::~Player() {}

void Player::initialize() {
	m_position = float3(0.0f, 0.0f, -4.0f);
	m_velocity = float3(0.0f, 0.0f, 0.0f);
	m_gravity = -9.82f;
	m_playerForward = DEFAULTFORWARD;

	m_speed = 2.10f;
	m_velocityFactorFrontBack = 0.0f;
	m_velocityFactorStrafe = 0.0f;

	m_camera.setView(m_position, m_playerForward, float3(0.0, 1.0, 0.0));

	m_playerRight = DEFAULTFORWARD;
	m_playerUp = DEFAULTUP;
	m_cameraPitch = m_cameraYaw = 0.0f;
	m_aimZoom = 0.0f;
	m_releasing = false;
}

void Player::update(float td, Terrain* terrain) {

	// player rotation
	rotatePlayer();

	// modify velocity vector to match terrain
	do {
		Vector3 movement = m_velocity * 0.017f;
		float l = terrain->castRay(m_position, movement);
		if (l == -1)
			break;
		float3 collisionPoint = m_position + movement * l;
		float3 collisionNormal = terrain->getNormalFromPosition(collisionPoint.x, collisionPoint.z);
		slide(td, collisionNormal, l);
	} while (1);

	// player movement
	float forceStrength = 10;
	float3 force;
	float3 playerStraightForward = float3(0, 1, 0).Cross(m_playerForward).Cross(float3(0, 1, 0));
	force += playerStraightForward * (Input::getInstance()->keyDown(Keyboard::W) -
										 Input::getInstance()->keyDown(Keyboard::S));
	force += m_playerRight * (Input::getInstance()->keyDown(Keyboard::D) -
								 Input::getInstance()->keyDown(Keyboard::A));

	// movement
	m_position += m_velocity * td;

	// onground
	float3 normal = terrain->getNormalFromPosition(m_position.x, m_position.z);
	float height = terrain->getHeightFromPosition(m_position.x, m_position.z);
	float terrainSteepness = float3(0, 1, 0).Dot(normal);
	m_position.y =
		clamp(m_position.y, m_position.y, height); // clamp position to never go under terrain!
	if (abs(m_position.y - height) < 0.025) {
		// on ground
		m_onGround = true;

		if (terrainSteepness < 0.6) {
			// STEEP terrain
			m_velocity.y -= 5 * td; // gravity if steep terrain
			m_velocity *= 0.99;		// weak ground friction

			ErrorLogger::log("Steep terrain!");
		}
		else {
			// FLAT terrian
			m_velocity *= 0.9; // ground friction

			// jump
			if (Input::getInstance()->keyPressed(Keyboard::Space)) {
				m_velocity.y = 2;
			}

			// add player forces
			m_velocity += force * forceStrength * td;
		}
	}
	else {
		// in air
		m_onGround = false;

		m_velocity.y -= 5 * td; // gravity if not on ground

		// add forces
		m_velocity += force * forceStrength * 0.2f * td;
	}



	// update camera properties
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position + float3(0, PLAYERHEIGHT, 0));
	m_camera.setTarget(m_position + float3(0, PLAYERHEIGHT, 0) + m_playerForward);
	m_camera.updateBuffer();

	// Update bow
	bowUpdate(td);
	m_bow.rotate(m_cameraPitch, m_cameraYaw);
	m_bow.update(td, getCameraPosition(), m_playerForward, m_playerRight);
}

void Player::bowUpdate(float dt) {
	Input* input = Input::getInstance();

	if (input->mouseDown(Input::MouseButton::RIGHT)) {
		m_aimZoom = min(1.0f, m_aimZoom + dt * 2.0f);
		m_camera.setFov(XM_PI / (3.0f * (1.0f + m_aimZoom)));
		m_bow.aim();
	}
	if (m_releasing || input->mouseReleased(Input::MouseButton::RIGHT)) {
		m_releasing = true;
		m_bow.release();

		if (m_aimZoom > 0.0f)
			m_aimZoom -= dt * 2.0f;
		else
			m_releasing = false;

		m_camera.setFov(XM_PI / (3.0f * (1.0f + m_aimZoom)));
	}
	if (input->mouseDown(Input::MouseButton::LEFT)) {
		m_bow.charge();
	}
	if (input->mouseUp(Input::MouseButton::LEFT)) {
		m_bow.shoot(m_playerForward);
	}
}

void Player::rotatePlayer() {
	Input* ip = Input::getInstance();

	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		deltaX = (float)ip->mouseX();
		deltaY = (float)ip->mouseY();
	}

	float rotationSpeed = 0.01f;

	if (deltaX != 0.0f) {
		m_cameraYaw += deltaX * rotationSpeed;
	}
	if (deltaY != 0.0f) {
		m_cameraPitch += deltaY * rotationSpeed;
		m_cameraPitch = min(max(m_cameraPitch, -1.5f), 1.5f);
	}

	if (ip->keyDown(Keyboard::Keys::Right))
		m_cameraYaw += 0.01f;
	if (ip->keyDown(Keyboard::Keys::Left))
		m_cameraYaw -= 0.01f;
	if (ip->keyDown(Keyboard::Keys::Up))
		m_cameraPitch -= 0.01f;
	if (ip->keyDown(Keyboard::Keys::Down))
		m_cameraPitch += 0.01f;

	Matrix cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.f);
	Vector3 cameraTarget = XMVector3TransformCoord(m_playerForward, cameraRotationMatrix);
	cameraTarget = XMVector3Normalize(cameraTarget);

	Matrix rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

	m_playerForward = XMVector3TransformCoord(DEFAULTFORWARD, cameraRotationMatrix);
	m_playerUp = XMVector3TransformCoord(m_playerUp, rotateYTempMatrix);
	m_playerRight = XMVector3TransformCoord(DEFAULTRIGHT, cameraRotationMatrix);
}

void Player::draw() {
	m_camera.bindMatrix();
	m_bow.draw();
}

float3 Player::getPosition() const { return m_position; }

float3 Player::getCameraPosition() const { return m_camera.getPosition(); }

float3 Player::getForward() const { return m_playerForward; }

float3 Player::getVelocity() const { return m_velocity; }

void Player::setPosition(float3 position) { m_position = position; }

void Player::groundCheck() { // Check if you are on the ground
	if (m_position.y <= m_groundHeight + 0.01f) {
		m_onGround = true;
		m_velocity.y = 0.0f; // Stop gravity if you are on the ground.
		m_position.y = m_groundHeight;
	}
	else {
		m_onGround = false;
	}
}

void Player::bounceCheck(Vector3 normal) {
	if (DEFAULTUP.Dot(normal) < 0.5f) {
		m_bouncing = true;
	}
	else {
		m_bouncing = false;
	}
}

void Player::slideCheck(Vector3 normal) {
	if (DEFAULTUP.Dot(normal) < 0.4f || m_velocity.Length() >= 1.5f) {
		m_sliding = true;
	}
	else {
		m_sliding = false;
	}
}

void Player::slide(float td, Vector3 normal, float l) {
	if (l != -1) {
		float friction = 0.0;
		Vector3 longVel = m_velocity * td;
		Vector3 shortVel = longVel * l;
		Vector3 longVel_n = longVel.Dot(normal) * normal;
		Vector3 shortVel_n = shortVel.Dot(normal) * normal;
		Vector3 diffVel_n = longVel_n - shortVel_n;
		Vector3 longVelocityOnPlane = longVel - longVel_n;
		Vector3 shortVelocityOnPlane = shortVel - shortVel_n;
		Vector3 diffVelocityOnPlane = longVelocityOnPlane - shortVelocityOnPlane;
		Vector3 velOnPlaneNorm = longVelocityOnPlane;
		velOnPlaneNorm.Normalize();
		Vector3 frictionOnPlane = -velOnPlaneNorm * diffVel_n.Length() * friction;
		if (frictionOnPlane.Length() > diffVelocityOnPlane.Length()) {
			frictionOnPlane *= 0;
			diffVelocityOnPlane *= 0;
		}
		m_velocity = (shortVelocityOnPlane + shortVel_n + normal * 0.0001 + diffVelocityOnPlane +
						 frictionOnPlane) /
					 td;
		return;
		Vector3 slideVector =
			longVel - ((longVel.Dot(normal) - shortVel.Dot(normal) - 0.001) * normal);
		m_velocity = slideVector / td; // normalized
	}
}

void Player::dash() {
	if (m_dashCooldown > 0.0f) {
		m_dashCooldown = 0.0f;
		// m_velocityFactorFrontBack = 5.0f;
		m_velocity += (m_playerForward * 10);
		m_onGround = false;
	}
	else {
		ErrorLogger::log(std::to_string(1 - m_dashCooldown));
	}
}

void Player::bounce(Vector3 normal, float dt) {
	// Reflection = InVector - 2(Invector * normal) * normal;
	Vector3 bounceVector = 10 * (m_velocity - ((2 * m_velocity.Dot(normal) * normal)));
	bounceVector.x = clamp(bounceVector.x, 0.5, -0.5);
	bounceVector.z = clamp(bounceVector.z, 0.5, -0.5);
	bounceVector.y = clamp(bounceVector.y, 0.5, -0.5);
	bounceVector.y += m_gravity * dt;
	m_velocity = bounceVector;
	ErrorLogger::log("Boing " /* + std::to_string(m_velocity.y)*/);
}

void Player::movement(Vector3 normal, float dt, Vector3 collisionPoint) {
	if (m_onGround) {
		if (m_sliding) {
			// slide(normal, collisionPoint);
			m_velocity.y += (m_gravity * dt);
		}
		else if (m_bouncing) {
			bounce(normal, dt);
		}
		else { // Running
			   // To avoid "skipping" - Position along the Y-axis is avoided here under.
			Vector3 flatForward =
				XMVector3Normalize(Vector3(m_playerForward.x, 0.0f, m_playerForward.z));
			m_velocity.x = m_velocityFactorFrontBack * flatForward.x;
			m_velocity.z = m_velocityFactorFrontBack * flatForward.z;
			m_velocity.x += m_velocityFactorStrafe * m_playerRight.x;
			m_velocity.z += m_velocityFactorStrafe * m_playerRight.z;
		}
	}
	else { // Flying
		m_position.y = m_position.y + m_velocity.y * dt +
					   (m_gravity * dt * dt) * 0.5f; // Pos2 = Pos1 + v1 * t + (a * t^2)/2
		m_velocity.y += m_gravity * dt;
	}
}

float Player::clamp(float x, float high, float low) {
	if (x > high) {
		x = high;
	}
	else if (x < low) {
		x = low;
	}
	return x;
}
