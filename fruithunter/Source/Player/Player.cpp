#include "Player.h"
#include "Renderer.h"
#include "Errorlogger.h"
#include "VariableSyncer.h"
#include "AudioHandler.h"

Player::Player() {}

Player::~Player() {}

void Player::initialize() {
	m_position = float3(1.0f, 2.0f, 3.0f);
	m_lastSafePosition = m_position;
	m_velocity = float3(0.0f, 0.0f, 0.0f);
	m_playerForward = DEFAULTFORWARD;
	VariableSyncer::getInstance()->create("Player.txt", nullptr);
	VariableSyncer::getInstance()->bind("Player.txt", "speed walk:f", &m_speed);
	VariableSyncer::getInstance()->bind("Player.txt", "speed sprint:f", &m_speedSprint);
	VariableSyncer::getInstance()->bind("Player.txt", "speed in air:f", &m_speedInAir);
	VariableSyncer::getInstance()->bind("Player.txt", "jump force:f", &m_jumpForce);
	VariableSyncer::getInstance()->bind("Player.txt", "dash force:f", &m_dashForce);
}

void Player::update(float dt, Terrain* terrain) {
	// Movement force
	float3 force = getMovementForce();

	checkJump();
	checkSprint(dt);
	checkDash(dt);

	rotatePlayer(dt);

	if (m_onEntity) {
		// On an object, behave like ground
		m_position += m_velocity * dt;
		updateVelocity_onFlatGround(force, dt);

		// reset value
		m_onEntity = false;
	}
	else if (terrain != nullptr) {
		// In terrain
		calculateTerrainCollision(terrain, dt);

		// Move player;
		m_position += m_velocity * dt;

		// Update velocity for next frame
		if (onGround(terrain)) {
			if (getSteepness(terrain) < STEEPNESS_BORDER) {
				// Steep ground
				updateVelocity_onSteepGround(dt);
			}
			else {
				// Flat ground
				updateVelocity_onFlatGround(force, dt);
			}
		}
		else {
			// In air
			updateVelocity_inAir(force, dt);
		}
	}
	else {
		// Outside of terrain, falling
		m_position += m_velocity * dt;
		updateVelocity_inAir(force, dt);
	}

	// Reset player if below sea level
	checkPlayerReset(dt);

	restoreStamina(dt);

	updateGodMode(dt);
	updateCamera();

	updateBow(dt, terrain);
}

void Player::draw() {
	m_camera.bindMatrix();
	m_bow.draw();
}

void Player::collideObject(Entity& obj) {
	// Check
	float radius = 0.2f;
	float stepHeight = 0.65f; // height able to simply step over
	EntityCollision feet(
		m_position + float3(0.f, radius, 0.f), float3(0.f), float3(1.f), float(radius));
	EntityCollision hip(m_position + float3(0.f, stepHeight + radius * 2, 0.f), float3(0.f),
		float3(1.f), float(radius * 2.f));
	if (obj.checkCollision(feet)) { // walk on/to
		if (m_velocity.y <= 0.f) {
			m_velocity.y = 0.f;
			m_onGround = true;
			m_onEntity = true;
			if (obj.getCollisionType() == EntityCollision::ctOBB)
				m_position.y +=
					(obj.getPointOnOBB(m_position + float3(0.f, 2.f, 0.f)).y - m_position.y) * 0.2f;
			else {
				// Doesn't work great. Made all objects obbs
				float3 down(0.01f, -1.f, 0.01f);
				down.Normalize();
				float cast = obj.castRay(m_position + float3(0.f, stepHeight, 0.f), down);
				if (cast != -1.f && cast < stepHeight) {
					m_position.y = m_position.y + stepHeight - cast;
					// m_position.y += (stepHeight - cast) * 1.f;
				}
			}
		}
	}
	else if (obj.checkCollision(hip)) { // bump into
		float3 objToPlayer;
		if (obj.getCollisionType() == EntityCollision::ctOBB) {
			float3 pointOnOBBClosestToPlayer = obj.getPointOnOBB(m_position);
			objToPlayer = m_position - pointOnOBBClosestToPlayer;
		}
		else {
			objToPlayer = m_position - (obj.getBoundingBoxPos() + obj.getPosition());
		}
		if (objToPlayer.Dot(m_velocity) < 0.f) {
			objToPlayer.Normalize();
			float3 tangentObj = float3::Up.Cross(objToPlayer);
			float3 reflection = float3::Reflect(m_velocity, objToPlayer);
			reflection.y = m_velocity.y;
			m_velocity += reflection;
			m_velocity *= 0.5f;
		}
	}
}

bool Player::checkAnimal(float3 animalPos, float range, float throwStrength) {
	float3 diff = m_position - animalPos;
	float rangeSq = range * range;
	float diffLenSq = diff.LengthSquared();
	bool inRange = diffLenSq < rangeSq;
	if (inRange) {
		float strength = (rangeSq - diffLenSq) / rangeSq;
		diff.Normalize();
		float3 throwVec = (diff + float3(0.f, 1.f, 0.f)) * throwStrength;
		m_velocity = throwVec;
		return true;
	}
	return false;
}

float3 Player::getPosition() const { return m_position; }

float3 Player::getCameraPosition() const { return m_camera.getPosition(); }

float3 Player::getForward() const { return m_playerForward; }

float3 Player::getVelocity() const { return m_velocity; }

float Player::getStamina() const { return m_stamina; }

bool Player::isShooting() const { return m_bow.isShooting(); }

void Player::setPosition(float3 position) {
	m_position = position;
	m_lastSafePosition = position;
}

void Player::standsOnObject() { m_onEntity = true; }

void Player::updateBow(float dt, Terrain* terrain) {
	Input* input = Input::getInstance();

	if (input->mouseDown(Input::MouseButton::RIGHT)) {
		m_aimZoom = max(0.4f, m_aimZoom - dt * 1.5f);
		m_camera.setFov(m_camera.getDefaultFov() * m_aimZoom);
		m_bow.aim();
	}
	else if (m_releasing || input->mouseReleased(Input::MouseButton::RIGHT)) {
		m_releasing = true;

		if (m_aimZoom < 1.0f) {
			m_aimZoom += dt * 1.5f;
		}
		else {
			m_bow.release();
			m_aimZoom = 1.0f;
			m_releasing = false;
		}

		m_camera.setFov(m_camera.getDefaultFov() * m_aimZoom);
	}
	if (input->mousePressed(Input::MouseButton::LEFT)) {
		m_chargingBow = true;
	}
	if (input->mouseDown(Input::MouseButton::LEFT) && m_chargingBow) {
		m_bow.charge();
	}
	else if (input->mouseUp(Input::MouseButton::LEFT)) {
		m_chargingBow = false;
		m_bow.shoot(m_playerForward, m_velocity, m_cameraPitch, m_cameraYaw);
	}

	float3 wind;
	terrain != nullptr ? wind = terrain->getWind() : wind = float3(0.f);

	m_bow.rotate(m_cameraPitch, m_cameraYaw);
	m_bow.update(dt, getCameraPosition(), m_playerForward, m_playerRight, wind);
}

void Player::updateCamera() {
	float playerHeight = PLAYER_HEIGHT - 0.5f * (m_dashCharge / DASHMAXCHARGE);
	m_camera.setUp(m_playerUp);
	m_camera.setEye(m_position + float3(0, playerHeight, 0));
	m_camera.setTarget(m_position + float3(0, playerHeight, 0) + m_playerForward);
}

void Player::rotatePlayer(float dt) {
	Input* ip = Input::getInstance();

	float deltaX = 0.0f;
	float deltaY = 0.0f;

	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		deltaX = (float)ip->mouseX();
		deltaY = (float)ip->mouseY();
	}

	float rotationSpeed = m_aimZoom * 0.6f * dt;

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

void Player::updateGodMode(float dt) {
	Input* ip = Input::getInstance();

	if (ip->keyPressed(Keyboard::Keys::G))
		m_godMode = !m_godMode;

	if (m_godMode) {
		m_velocity = float3(0.f);
		m_position += m_playerForward *
					  (float)(ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD)) *
					  m_godModeSpeed * dt;
		m_position += m_playerRight * (float)(ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT)) *
					  m_godModeSpeed * dt;
	}
}

float3 Player::getMovementForce() {
	Input* ip = Input::getInstance();
	float3 force;
	float3 playerStraightForward = m_playerRight.Cross(float3(0, 1, 0));
	force += playerStraightForward * (float)(ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD));
	force += m_playerRight * (float)(ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT));

	return force;
}

bool Player::onGround(Terrain* terrain) {
	float terrainHeight = terrain->getHeightFromPosition(
		m_position.x, m_position.z); // height of terrain on current position
	m_position.y = clamp(
		m_position.y, m_position.y, terrainHeight); // clamp position to never go under terrain!

	m_onGround = abs(m_position.y - terrainHeight) < ONGROUND_THRESHOLD;

	return m_onGround;
}

float Player::getSteepness(Terrain* terrain) {
	float3 normal =
		terrain->getNormalFromPosition(m_position.x, m_position.z); // normal on current position
	float terrainSteepness =
		abs(float3(0, 1, 0).Dot(normal)); // abs() because sometime the dot product becomes negative

	return terrainSteepness;
}

void Player::calculateTerrainCollision(Terrain* terrain, float dt) {
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
}

void Player::checkJump() {
	if (Input::getInstance()->keyPressed(KEY_JUMP) && m_jumpReset) {
		m_jumpReset = false;
		m_velocity.y = m_jumpForce;
	}
}

void Player::checkSprint(float dt) {
	if (Input::getInstance()->keyPressed(KEY_SPRINT) && m_stamina > STAMINA_SPRINT_THRESHOLD &&
		!m_chargingDash && m_onGround) {
		// activate sprint
		m_sprinting = true;
	}
	if (Input::getInstance()->keyDown(KEY_SPRINT) && m_sprinting && m_stamina > 0 &&
		m_velocity.Length() > 0.1f) {
		// consume stamina
		consumeStamina(STAMINA_SPRINT_CONSUMPTION * dt);
	}
	else {
		m_sprinting = false;
	}
}

vector<FrustumPlane> Player::getFrustumPlanes() const { return m_camera.getFrustumPlanes(); }

CubeBoundingBox Player::getCameraBoundingBox() const { return m_camera.getFrustumBoundingBox(); }

void Player::checkDash(float dt) {
	if (Input::getInstance()->keyPressed(KEY_DASH) && m_stamina >= STAMINA_DASH_COST &&
		!m_sprinting && m_onGround) {
		m_chargingDash = true;
	}

	if (Input::getInstance()->keyDown(KEY_DASH) && m_chargingDash) {
		m_dashCharge = clamp(m_dashCharge + dt, DASHMAXCHARGE, 0);
		consumeStamina(STAMINA_DASH_COST * dt);
	}
	else if (Input::getInstance()->keyReleased(KEY_DASH)) {
		m_chargingDash = false;

		float interpolateScale = 0.75f; // 0 = dash forward, 1 = dash up,
		float3 dir =
			float3(0, 1, 0).Cross(m_playerForward.Cross(float3(0, 1, 0))) * (1 - interpolateScale) +
			float3(0, 1, 0) * interpolateScale;
		dir.Normalize();
		m_velocity += dir * m_dashForce * ((float)m_dashCharge / DASHMAXCHARGE);
	}
	else {
		// return to original state
		m_dashCharge = clamp(m_dashCharge - 2 * dt, DASHMAXCHARGE, 0);
	}
}

void Player::checkPlayerReset(float dt) {
	if (m_position.y < m_seaHeight && !m_godMode) {
		m_velocity = float3(0.f);
		m_resetTimer += dt;
		if (m_resetTimer > m_resetDelay) {
			m_position = m_lastSafePosition;
			m_resetTimer = 0.f;
		}
	}
}

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
	if (m_sprinting)
		return m_speedSprint;
	else
		return m_speed;
}

void Player::consumeStamina(float amount) {
	m_stamina = clamp(m_stamina - amount, STAMINA_MAX, 0);
	m_staminaConsumed = true;
}

void Player::restoreStamina(float amount) {
	if (!m_staminaConsumed) {
		m_stamina =
			clamp(m_stamina + amount, STAMINA_MAX, 0); // restore stamina if no stamina was consumed
	}
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

	m_jumpReset = true;

	// add player forces
	m_velocity += playerForce * getPlayerMovementSpeed() * dt;

	m_lastSafePosition = m_position;
}

void Player::updateVelocity_onSteepGround(float dt) {
	m_velocity += m_gravity * dt;						// gravity if steep terrain
	m_velocity *= pow(GROUND_FRICTION_WEAK / 60.f, dt); // weak ground friction
}
