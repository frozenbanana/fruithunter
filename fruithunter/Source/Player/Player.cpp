#include "Player.h"
#include "Renderer.h"
#include "Errorlogger.h"

Player::Player() { initialize(); }

Player::~Player() {}

void Player::initialize() { m_camera.setView(m_position, m_playerForward, float3(0.0, 1.0, 0.0)); }

void Player::update(float dt, Terrain* terrain) {
	Input* ip = Input::getInstance();

	// player rotation
	rotatePlayer(dt);

	// modify velocity vector to match terrain
	if (terrain != nullptr) {
		int loopCount = 100;
		while (1) {
			float3 movement = m_velocity * dt;
			float l = terrain->castRay(m_position, movement);
			if (l == -1)
				break; // break of no intersection was found!
			float3 collisionPoint = m_position + movement * l;
			float3 collisionNormal =
				terrain->getNormalFromPosition(collisionPoint.x, collisionPoint.z);
			slide(dt, collisionNormal, l);
			loopCount--;
			if (loopCount <= 0) {
				ErrorLogger::logWarning(HRESULT(),
					"WARNING! Player collision with terrain calculated "
					"ALOT of iterations in the update function!");
				break;
			}
		};
	}

	// player movement
	float3 force;
	float3 playerStraightForward = m_playerRight.Cross(float3(0, 1, 0));
	force += playerStraightForward * (float)(ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD));
	force += m_playerRight * (float)(ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT));

	// movement
	m_position += m_velocity * dt;
	if (m_onEntity) {
		updateVelocity_onFlatGround(force, dt);
	}
	else if (terrain != nullptr) {
		float3 normal = terrain->getNormalFromPosition(
			m_position.x, m_position.z); // normal on current position
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
				updateVelocity_onSteepGround(dt);
			}
			else {
				// FLAT terrian
				updateVelocity_onFlatGround(force, dt);
			}
		}
		else {
			// in air
			updateVelocity_inAir(force, dt);
		}
	}
	else {
		// FALL OF TERRAIN IF NO TERRAIN
		// in air
		updateVelocity_inAir(force, dt);
	}
	// reset value
	m_onEntity = false;

	// dash
	if (m_stamina >= STAMINA_DASH_COST && !m_sprinting && m_onGround) {
		if (ip->keyDown(KEY_DASH)) {
			m_dashCharge = clamp(m_dashCharge + dt, DASHMAXCHARGE, 0);
		}
		else if (ip->keyReleased(KEY_DASH)) {
			m_velocity += m_playerForward * m_dashForce * ((float)m_dashCharge / DASHMAXCHARGE);
			consumeStamina(STAMINA_DASH_COST);
		}
	}
	else {
		// return to original state
		m_dashCharge = clamp(m_dashCharge - 2 * dt, DASHMAXCHARGE, 0);
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
		m_aimZoom = max(0.5f, m_aimZoom - dt);
		m_camera.setFov(m_camera.getDefaultFov() * m_aimZoom);
		m_bow.aim();
	}
	else if (m_releasing || input->mouseReleased(Input::MouseButton::RIGHT)) {
		m_releasing = true;
		m_bow.release();

		if (m_aimZoom < 1.0f) {
			m_aimZoom += dt;
		}
		else {
			m_aimZoom = 1.0f;
			m_releasing = false;
		}

		m_camera.setFov(m_camera.getDefaultFov() * m_aimZoom);
	}
	if (input->mouseDown(Input::MouseButton::LEFT)) {
		m_bow.charge();
	}
	else if (input->mouseUp(Input::MouseButton::LEFT)) {
		m_bow.shoot(m_playerForward);
	}

	m_bow.rotate(m_cameraPitch, m_cameraYaw);
	m_bow.update(dt, getCameraPosition(), m_playerForward, m_playerRight);
}

void Player::updateCamera() {
	float playerHeight = PLAYER_HEIGHT - 0.5f * (m_dashCharge / DASHMAXCHARGE);
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position + float3(0, playerHeight, 0));
	m_camera.setTarget(m_position + float3(0, playerHeight, 0) + m_playerForward);
	m_camera.updateBuffer();
}

void Player::rotatePlayer(float dt) {
	Input* ip = Input::getInstance();

	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		deltaX = (float)ip->mouseX();
		deltaY = (float)ip->mouseY();
	}

	float rotationSpeed = 0.6f * dt;

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

void Player::collideObject(Entity& obj) {
	// Check
	// oeoepeoe
	float radius = 0.2;
	float stepHeight = 0.45f; // height able to simply step over
	EntityCollision feet(m_position + float3(0.f, radius, 0.f), float3(0.f), float3(1.f), radius);
	EntityCollision hip(m_position + float3(0.f, stepHeight + radius * 2, 0.f), float3(0.f),
		float3(1.f), radius * 2);

	if (obj.checkCollision(hip)) { // bump into
		float3 objToPlayer = m_position - obj.getPosition();
		if (objToPlayer.Dot(m_velocity) < 0) {
			objToPlayer.Normalize();
			float3 tangentObj = float3::Up.Cross(objToPlayer);
			float3 reflection = float3::Reflect(m_velocity, objToPlayer);
			reflection.y = m_velocity.y;
			m_velocity += reflection;
			m_velocity *= 0.5;
		}
	}
	else if (obj.checkCollision(feet)) { // walk on/to
		if (m_velocity.y <= 0) {
			m_velocity.y = 0;
			m_onGround = true;
			m_onEntity = true;
			if (obj.getCollisionType() == EntityCollision::ctOBB)
				m_position.y =
					obj.getBoundingBoxPos().y + obj.getHalfSizes().y * obj.getScale().y - 0.01f;


			// I we fix castray
			// float cast =
			//	obj.castRay(m_position + float3(0.f, stepHeight, 0.f), float3(0.01, -1, 0.01));
			// if (cast != -1) {
			//	m_position.y += (stepHeight - cast) * 0.1;
			//}
		}
	}
}

float3 Player::getPosition() const { return m_position; }

float3 Player::getCameraPosition() const { return m_camera.getPosition(); }

float3 Player::getForward() const { return m_playerForward; }

float3 Player::getVelocity() const { return m_velocity; }

void Player::setPosition(float3 position) { m_position = position; }

void Player::standsOnObject() { m_onEntity = true; }

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
		m_velocity =
			(longVel - (longVel.Dot(normal) - shortVel.Dot(normal) - 0.001f) * normal) / dt;
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
		m_stamina =
			clamp(m_stamina + amount, STAMINA_MAX, 0); // restore stamina if no stamina was consumed
	m_staminaConsumed = false; // set to false because this function should be called only once per
							   // frame, fixing the statement to next frame.
}

void Player::updateVelocity_inAir(float3 playerForce, float dt) {
	// in air
	m_onGround = false;

	m_velocity += m_gravity * dt; // gravity if in air

	// add forces
	m_velocity += playerForce * m_speedInAir * dt;
}

void Player::updateVelocity_onFlatGround(float3 playerForce, float dt) {
	m_velocity *= pow(GROUND_FRICTION / 60.f, dt); // ground friction

	// jump
	if (Input::getInstance()->keyPressed(KEY_JUMP)) {
		m_velocity.y = m_jumpForce;
	}

	// add player forces
	m_velocity += playerForce * getPlayerMovementSpeed() * dt;
}

void Player::updateVelocity_onSteepGround(float dt) {
	m_velocity += m_gravity * dt;						// gravity if steep terrain
	m_velocity *= pow(GROUND_FRICTION_WEAK / 60.f, dt); // weak ground friction
}
