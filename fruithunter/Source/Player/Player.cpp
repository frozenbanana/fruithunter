#include "Player.h"
#include "Renderer.h"
#include "Errorlogger.h"

Player::Player() { initialize(); }

Player::~Player() {}

void Player::initialize() { m_camera.setView(m_position, m_playerForward, float3(0.0, 1.0, 0.0)); }

void Player::update(float dt, Terrain* terrain) {
	Input* ip = Input::getInstance();

	// player rotation
	rotatePlayer();

	// modify velocity vector to match terrain
	int loopCount = 100;
	while (1) {
		float3 movement = m_velocity * dt;
		float l = terrain->castRay(m_position, movement);
		if (l == -1)
			break; // break of no intersection was found!
		float3 collisionPoint = m_position + movement * l;
		float3 collisionNormal = terrain->getNormalFromPosition(collisionPoint.x, collisionPoint.z);
		slide(dt, collisionNormal, l);
		loopCount--;
		if (loopCount <= 0) {
			ErrorLogger::logWarning(HRESULT(), "WARNING! Player collision with terrain calculated "
											   "ALOT of iterations in the update function!");
			break;
		}
	};

	// player movement
	float3 force;
	float3 playerStraightForward = m_playerRight.Cross(float3(0, 1, 0));
	force += playerStraightForward * (ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD));
	force += m_playerRight * (ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT));

	// movement
	m_position += m_velocity * dt;

	// onground
	float3 normal =
		terrain->getNormalFromPosition(m_position.x, m_position.z); // normal on current position
	float height = terrain->getHeightFromPosition(
		m_position.x, m_position.z); // height of terrain on current position
	float terrainSteepness = float3(0, 1, 0).Dot(normal);
	m_position.y =
		clamp(m_position.y, m_position.y, height); // clamp position to never go under terrain!
	if (abs(m_position.y - height) < ONGROUND_THRESHOLD) {
		// on ground
		m_onGround = true;

		if (terrainSteepness < STEEPNESS_BORDER) {
			// STEEP terrain
			m_velocity += m_gravity * dt;		// gravity if steep terrain
			m_velocity *= GROUND_FRICTION_WEAK; // weak ground friction
		}
		else {
			// FLAT terrian
			m_velocity *= GROUND_FRICTION; // ground friction

			// jump
			if (ip->keyPressed(KEY_JUMP)) {
				m_velocity.y = m_jumpForce;
			}

			// add player forces
			m_velocity += force * getPlayerMovementSpeed() * dt;
		}
	}
	else {
		// in air
		m_onGround = false;

		m_velocity += m_gravity * dt; // gravity if in air

		// add forces
		m_velocity += force * m_speedInAir * dt;
	}

	// dash
	if (m_stamina >= STAMINA_DASH_COST && !m_sprinting && m_onGround) {
		if (ip->keyDown(KEY_DASH)) {
			m_dashCharge = clamp(m_dashCharge + dt, DASHMAXCHARGE, 0);
		}
		else if (ip->keyReleased(KEY_DASH)) {
			m_velocity += m_playerForward * m_dashForce * ((float)m_dashCharge / DASHMAXCHARGE);
			consumeStamina(STAMINA_DASH_COST);
			// reset
			m_dashCharge = 0;
		}
	}
	else {
		//return to original state
		m_dashCharge = clamp(m_dashCharge - 2*dt, DASHMAXCHARGE, 0);
	}

	// sprint
	if (ip->keyPressed(KEY_SPRINT) && m_stamina > STAMINA_SPRINT_THRESHOLD && !m_chargingDash) {
		// activate sprint
		m_sprinting = true;
	}
	if (ip->keyDown(KEY_SPRINT) && m_sprinting && m_stamina > 0) {
		// consume stamina
		consumeStamina(STAMINA_SPRINT_CONSUMPTION * dt);
	}
	else {
		m_sprinting = false;
	}

	// restore stamina
	restoreStamina(dt); // should be called only once!

	// update camera properties
	updateCamera();

	// Update bow
	updateBow(dt);
}

void Player::updateBow(float dt) {
	Input* input = Input::getInstance();

	if (input->mouseDown(Input::MouseButton::RIGHT)) {
		m_bow.aim();
	}
	if (input->mouseDown(Input::MouseButton::LEFT)) {
		m_bow.charge();
	}
	if (input->mouseUp(Input::MouseButton::LEFT)) {
		m_bow.shoot(m_playerForward);
	}

	m_bow.rotate(m_cameraPitch, m_cameraYaw);
	m_bow.update(dt, getCameraPosition(), m_playerForward, m_playerRight);
}

void Player::updateCamera() {
	float playerHeight = PLAYER_HEIGHT - 0.5 * (m_dashCharge / DASHMAXCHARGE);
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position + float3(0, playerHeight, 0));
	m_camera.setTarget(m_position + float3(0, playerHeight, 0) + m_playerForward);
	m_camera.updateBuffer();
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
	float3 cameraTarget = XMVector3TransformCoord(m_playerForward, cameraRotationMatrix);
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

void Player::slide(float dt, float3 normal, float l) {
	if (l != -1) {
		/*
		 * Similar equation that applies friction to the sliding effect. Doesn't work nicely, so not
		 * being used.
		 */
		/*
		 float friction = 0.0;
		 float3 longVel = m_velocity * dt;//full velocity this frame (called long velocity)
		 float3 shortVel = longVel * l;//velocity until collision (called short velocity)
		 float3 longVel_n = longVel.Dot(normal) * normal;//long velocity along normal
		 float3 shortVel_n = shortVel.Dot(normal) * normal;// short velocity along normal
		 float3 diffVel_n = longVel_n - shortVel_n;//difference between long and short velocity
		along normal float3 longVelocityOnPlane = longVel - longVel_n;//long velocity along plane
		 float3 shortVelocityOnPlane = shortVel - shortVel_n;// short velocity along plane
		 float3 diffVelocityOnPlane = longVelocityOnPlane - shortVelocityOnPlane;//difference
		between long and short velocity along plane float3 velOnPlaneNorm =
		longVelocityOnPlane;//normalized direction of velocity along plane
		 velOnPlaneNorm.Normalize();
		 float3 frictionOnPlane = -velOnPlaneNorm * diffVel_n.Length() * friction;
		 if (frictionOnPlane.Length() > diffVelocityOnPlane.Length()) {
			frictionOnPlane *= 0;
			diffVelocityOnPlane *= 0;
		}
		 m_velocity = (shortVelocityOnPlane + shortVel_n + normal * 0.0001 + diffVelocityOnPlane +
						 frictionOnPlane) /
					 dt;
		*/

		// standard slide effect with no friction
		float3 longVel = m_velocity * dt; // full velocity this frame (called long velocity)
		float3 shortVel = longVel * l;	  // velocity until collision (called short velocity)
		m_velocity = (longVel - (longVel.Dot(normal) - shortVel.Dot(normal) - 0.001) * normal) / dt;
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

float Player::getPlayerMovementSpeed() const {
	if (m_dashCharge > 0)
		return m_speedOnChargingDash;
	if (Input::getInstance()->keyDown(KEY_SPRINT) && m_sprinting && m_stamina > 0)
		return m_speedSprint;
	else
		return m_speed;
}

void Player::consumeStamina(float amount) {
	m_stamina = clamp(m_stamina - amount, STAMINA_MAX, 0);
	m_staminaConsumed = true;
}

void Player::restoreStamina(float amount) {
	if (!m_staminaConsumed)
		m_stamina = clamp(m_stamina + amount, STAMINA_MAX, 0);//restore stamina if no stamina was consumed
	m_staminaConsumed = false; // set to false because this function should be called only once per
							   // frame, fixing the statement to next frame.
}
