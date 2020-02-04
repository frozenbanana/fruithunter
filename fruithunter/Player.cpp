#include "Player.hpp"
#include "Renderer.hpp"

Player::Player() {}

Player::~Player() {}

void Player::initialize() {
	m_position = Vector3(0.0f, 0.0f, -4.0f);
	m_movement = Vector3(0.0f, 0.0f, 0.0f);
	m_playerForward = DEFAULTFORWARD;

	m_velocity = .10f;
	m_velocityFactorFrontBack = 0.0f;
	m_velocityFactorStrafe = 0.0f;

	m_camera.setView(m_position, m_playerForward, Vector3(0.0, 1.0, 0.0));


	m_playerRight = DEFAULTFORWARD;
	m_playerUp = DEFAULTUP;
	m_cameraPitch = m_cameraYaw = 0.0f;
}

void Player::update(float td) {
	rotatePlayer();
	movePlayer();
	m_position += m_velocity * m_movement * td;
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
		if (m_velocityFactorFrontBack >= 0.0f)
			m_velocityFactorFrontBack -= FACTORSTEPS;
	}

	if (input->keyDown(Keyboard::Keys::S)) {
		m_velocityFactorFrontBack >= -1.0f ? m_velocityFactorFrontBack -= FACTORSTEPS
										   : m_velocityFactorFrontBack = -1.0f;
		// ErrorLogger::log("pressing S ");
	}
	else {
		if (m_velocityFactorFrontBack <= (0.0f))
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

	// STOPPED IT FROM MOVING
	if (m_velocityFactorStrafe <= 0.1 && m_velocityFactorStrafe >= -0.1)
		m_velocityFactorStrafe = 0;
	if (m_velocityFactorFrontBack <= 0.1 && m_velocityFactorFrontBack >= -0.1)
		m_velocityFactorFrontBack = 0;
	m_position += m_velocity * m_velocityFactorFrontBack * m_playerForward;
	m_position += m_velocity * m_velocityFactorStrafe * m_playerRight;
}

void Player::rotatePlayer() {
	Input* ip = Input::getInstance();


	float deltaX = ip->mouseX();
	float deltaY = ip->mouseY();

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
	if (ip->keyDown(Keyboard::Keys::Space))
		m_cameraYaw = 0;



	Matrix cameraRotationMatrix = XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0);
	Vector3 cameraTarget = XMVector3TransformCoord(m_playerForward, cameraRotationMatrix);
	cameraTarget = XMVector3Normalize(cameraTarget);


	Matrix rotateYTempMatrix = XMMatrixRotationY(m_cameraYaw);

	m_playerForward = XMVector3TransformCoord(DEFAULTFORWARD, cameraRotationMatrix);
	m_playerUp = XMVector3TransformCoord(m_playerUp, rotateYTempMatrix);
	m_playerRight = XMVector3TransformCoord(DEFAULTRIGHT, cameraRotationMatrix);
}

void Player::draw() { m_camera.bindMatrix(); }
