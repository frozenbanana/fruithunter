#include "Player.h"
#include "Renderer.h"
#include "Errorlogger.h"

Player::Player() {}

Player::~Player() {}

void Player::initialize() {
	m_position = Vector3(0.0f, 0.0f, -4.0f);
	m_velocity = Vector3(0.0f, 0.0f, 0.0f);
	m_gravity = -9.82f;
	m_playerForward = DEFAULTFORWARD;

	m_speed = 2.10f;
	m_velocityFactorFrontBack = 0.0f;
	m_velocityFactorStrafe = 0.0f;

	m_camera.setView(m_position, m_playerForward, Vector3(0.0, 1.0, 0.0));


	m_playerRight = DEFAULTFORWARD;
	m_playerUp = DEFAULTUP;
	m_cameraPitch = m_cameraYaw = 0.0f;
}

void Player::update(float td, float height, float3 normal, Vector3 collisionPoint) {
	m_groundHeight = height; // update m_groundHeight
	groundCheck();
	slideCheck(normal);
	bounceCheck(normal);
	rotatePlayer();
	movePlayer();
	movement(normal, td, collisionPoint);
	m_position.x += m_speed * m_velocity.x * td;
	m_position.z += m_speed * m_velocity.z * td;
	m_dashCooldown += td; // Cooldown for dashing
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position);
	m_camera.setTarget(m_position + m_playerForward);

	m_camera.updateBuffer();


	// ErrorLogger::log(std::to_string(m_velocityFactorFrontBack));
}

void Player::movePlayer() {
	Input* input = Input::getInstance();
	if (input->keyDown(Keyboard::Keys::W)) {

		/*m_velocityFactorFrontBack < 1.0f ? m_velocityFactorFrontBack += FACTORSTEPS
										 : m_velocityFactorFrontBack -= FACTORSTEPS;*/

		if (m_velocityFactorFrontBack <= 1.0f) {
			m_velocityFactorFrontBack += FACTORSTEPS;
		}
		else {
			if (m_onGround) {
				m_velocityFactorFrontBack -= FACTORSTEPS;
			}
		}
		// ErrorLogger::log("pressing W ");
	}
	else {
		if (m_velocityFactorFrontBack > 0.0f)
			m_velocityFactorFrontBack -= FACTORSTEPS;
	}

	if (input->keyDown(Keyboard::Keys::S)) {
		m_velocityFactorFrontBack >= -1.0f ? m_velocityFactorFrontBack -= FACTORSTEPS
										   : m_velocityFactorFrontBack = -1.0f;
		// ErrorLogger::log("pressing S ");
	}
	else {
		if (m_velocityFactorFrontBack < (0.0f))
			m_velocityFactorFrontBack += FACTORSTEPS;
	}

	if (input->keyDown(Keyboard::Keys::D)) {
		m_velocityFactorStrafe < 1.0f ? m_velocityFactorStrafe += FACTORSTEPS
									  : m_velocityFactorStrafe = 1.0f;
	}
	else {
		if (m_velocityFactorStrafe > 0.0f) {
			m_velocityFactorStrafe -= FACTORSTEPS;
		}
	}

	if (input->keyDown(Keyboard::Keys::A)) {

		m_velocityFactorStrafe > -1.0f ? m_velocityFactorStrafe -= FACTORSTEPS
									   : m_velocityFactorStrafe = -1.0f;
	}
	else {
		if (m_velocityFactorStrafe < 0.0f)
			m_velocityFactorStrafe += FACTORSTEPS;
	}

	if (input->keyPressed(Keyboard::Keys::Space)) {
		jump();
	}

	if (input->keyPressed(Keyboard::Keys::LeftShift)) {
		dash();
	}

	/*if (input->keyDown(Keyboard::Keys::LeftControl)) {
		m_sliding = true;
	}*/


	// STOPPED IT FROM MOVING
	if (m_velocityFactorStrafe <= 0.1 && m_velocityFactorStrafe >= -0.1)
		m_velocityFactorStrafe = 0;
	if (m_velocityFactorFrontBack <= 0.1 && m_velocityFactorFrontBack >= -0.1)
		m_velocityFactorFrontBack = 0;
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
		m_cameraYaw += 0.1f;
	if (ip->keyDown(Keyboard::Keys::Left))
		m_cameraYaw -= 0.10f;
	if (ip->keyDown(Keyboard::Keys::Up))
		m_cameraPitch -= 0.1f;
	if (ip->keyDown(Keyboard::Keys::Down))
		m_cameraPitch += 0.1f;

	Matrix cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.f);
	Vector3 cameraTarget = XMVector3TransformCoord(m_playerForward, cameraRotationMatrix);
	cameraTarget = XMVector3Normalize(cameraTarget);

	Matrix rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

	m_playerForward = XMVector3TransformCoord(DEFAULTFORWARD, cameraRotationMatrix);
	m_playerUp = XMVector3TransformCoord(m_playerUp, rotateYTempMatrix);
	m_playerRight = XMVector3TransformCoord(DEFAULTRIGHT, cameraRotationMatrix);
}

void Player::draw() { m_camera.bindMatrix(); }

float3 Player::getPosition() const { return m_position; }

float3 Player::getForward() const { return m_playerForward; }

float3 Player::getVelocity() const { return m_velocity; }

void Player::setPosition(float3 position) { m_position = position; }

void Player::jump() {

	if (m_onGround) {
		m_velocity.y =
			3.0f; // If you are on the ground you may jump, giving yourself 3 m/s along the Y-axis.
		m_onGround = false;
	}
}

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

void Player::slide(Vector3 normal, Vector3 collisionPoint) {
	// Vektordelen
	Vector3 collisionVector = collisionPoint - m_position;
	Vector3 slideVector = m_velocity - ((m_velocity.Dot(normal) - collisionVector.Dot(normal)) * normal);
	//					negativ - stor					negativ - liten
	m_velocity = Vector3(
		clamp(slideVector.x, 5, -5), clamp(slideVector.y, 5, -5), clamp(slideVector.z, 5, -5));
	ErrorLogger::log("Swosh "/* + std::to_string(m_velocity.y)*/);
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
	ErrorLogger::log("Boing "/* + std::to_string(m_velocity.y)*/);
}

void Player::movement(Vector3 normal, float dt, Vector3 collisionPoint) {
	if (m_onGround) {
		if (m_sliding) {
			slide(normal, collisionPoint);
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
