#include "Player.hpp"
#include "Renderer.hpp"

Player::Player() {}

Player::~Player() {}

void Player::initialize() {
	m_position = Vector3(0.0f, 0.0f, -4.0f);
	m_velocity = Vector3(0.0f, 0.0f, 0.0f);
	m_gravity = -9.82f;
	m_playerForward = DEFAULTFORWARD;

	m_speed = .10f;
	m_velocityFactorFrontBack = 0.0f;
	m_velocityFactorStrafe = 0.0f;

	m_camera.setView(m_position, m_playerForward, Vector3(0.0, 1.0, 0.0));


	m_playerRight = DEFAULTFORWARD;
	m_playerUp = DEFAULTUP;
	m_cameraPitch = m_cameraYaw = 0.0f;
}

void Player::update(float td) {
	m_groundHeight = 0.0f; // update m_groundHeight
	rotatePlayer();
	movePlayer();
	m_position += m_speed * m_velocity * td;
	if (!onGround()) { // Movement along the Y-axis. a.k.a Gravity. According to one dimensional
					   // physics.
		m_position.y = m_position.y + m_velocity.y * td +
					   (m_gravity * td * td) * 0.5; // Pos2 = Pos1 + v1 * t + (a * t^2)/2
		m_velocity.y += m_gravity * td;				// Update old velocity
	}
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position);
	m_camera.setTarget(m_position + m_playerForward);

	m_camera.updateBuffer();
}

void Player::movePlayer() {
	Input* input = Input::getInstance();
	if (input->keyDown(Keyboard::Keys::W)) {

		m_velocityFactorFrontBack < 1.0f ? m_velocityFactorFrontBack += FACTORSTEPS
										 : m_velocityFactorFrontBack = 1.0f;
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

	// STOPPED IT FROM MOVING
	if (m_velocityFactorStrafe <= 0.1 && m_velocityFactorStrafe >= -0.1)
		m_velocityFactorStrafe = 0;
	if (m_velocityFactorFrontBack <= 0.1 && m_velocityFactorFrontBack >= -0.1)
		m_velocityFactorFrontBack = 0;
	// To avoid "skipping" - Position along the Y-axis is avoided here under.
	m_position.x += m_speed * m_velocityFactorFrontBack * m_playerForward.x;
	m_position.z += m_speed * m_velocityFactorFrontBack * m_playerForward.z;
	m_position.x += m_speed * m_velocityFactorStrafe * m_playerRight.x;
	m_position.z += m_speed * m_velocityFactorStrafe * m_playerRight.z;
}

void Player::rotatePlayer() {
	Input* ip = Input::getInstance();

	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		deltaX = ip->mouseX();
		deltaY = ip->mouseY();
	}

	float rotationSpeed = 0.01;

	if (deltaX != 0.0f) {
		m_cameraYaw += deltaX * rotationSpeed;
	}
	if (deltaY != 0.0f) {
		m_cameraPitch += deltaY * rotationSpeed;
		m_cameraPitch = min(max(m_cameraPitch, -1.5), 1.5);
	}

	if (ip->keyDown(Keyboard::Keys::Right))
		m_cameraYaw += 0.1f;
	if (ip->keyDown(Keyboard::Keys::Left))
		m_cameraYaw -= 0.10f;
	if (ip->keyDown(Keyboard::Keys::Up))
		m_cameraPitch -= 0.1f;
	if (ip->keyDown(Keyboard::Keys::Down))
		m_cameraPitch += 0.1f;

	Matrix cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0);
	Vector3 cameraTarget = XMVector3TransformCoord(m_playerForward, cameraRotationMatrix);
	cameraTarget = XMVector3Normalize(cameraTarget);

	Matrix rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

	m_playerForward = XMVector3TransformCoord(DEFAULTFORWARD, cameraRotationMatrix);
	m_playerUp = XMVector3TransformCoord(m_playerUp, rotateYTempMatrix);
	m_playerRight = XMVector3TransformCoord(DEFAULTRIGHT, cameraRotationMatrix);
}

void Player::draw() { m_camera.bindMatrix(); }

void Player::jump() {

	if (onGround()) {
		m_velocity.y =
			3.0f; // If you are on the ground you may jump, giving yourself 3 m/s along the Y-axis.
	}
}

bool Player::onGround() { // Check if you are on the ground
	bool _onGround = false;
	if (m_position.y <= m_groundHeight) {
		_onGround = true;
		m_velocity.y = 0.0f; // Stop gravity if you are on the ground.
		m_position.y = m_groundHeight;
	}
	return _onGround;
}
