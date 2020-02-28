#include "Bow.h"
#include "TerrainManager.h"
#include "AudioHandler.h"
#define ARM_LENGTH 0.55f
#define OFFSET_RIGHT 0.37f
#define OFFSET_UP -0.1f
#define BOW_ANGLE XM_PI / 8.0f

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.2f);
	m_arrow.load("Arrow");
	m_arrow.setScale(float3(0.2f, 0.2f, m_arrowLength));
	m_arrow.setPosition(float3(-10.f)); // To make sure that arrow doesn't spawn in fruits.
	m_arrow.setCollisionDataOBB();
}

Bow::~Bow() {}

void Bow::update(
	float dt, float3 playerPos, float3 playerForward, float3 playerRight, float3 wind) {
	// m_bow.setRotationByAxis(playerForward, BOW_ANGLE * m_aimMovement);

	// Set bow position based on player position and direction.
	float3 playerUp = playerForward.Cross(playerRight);
	m_bow.setPosition(playerPos + playerForward * ARM_LENGTH +
					  playerRight * OFFSET_RIGHT * m_aimMovement +
					  playerUp * OFFSET_UP * m_aimMovement);


	// Update m_arrowReturnTimer
	m_arrowReturnTimer -= dt;
	// Bow animation.
	if (m_charging) {
		AudioHandler::getInstance()->playInstance(AudioHandler::STRETCH_BOW, m_drawFactor);
		m_drawFactor = min(0.99f, m_drawFactor + dt);
		m_bow.updateAnimatedSpecific(m_drawFactor);
		m_bow.setFrameTargets(0, 1);
	}
	else {
		AudioHandler::getInstance()->pauseInstance(AudioHandler::STRETCH_BOW);
		m_drawFactor = max(0.0f, m_drawFactor - 5.0f * dt);
		m_bow.updateAnimatedSpecific(m_drawFactor);
		m_bow.setFrameTargets(0, 1);
	}

	// Update arrow.
	if (m_shooting) {
		if (!m_arrowHitObject) {
			float castray =
				TerrainManager::getInstance()->castRay(m_arrow.getPosition(), m_arrowVelocity * dt);
			if (castray != -1) {
				// Arrow is hitting terrain
				m_arrowReturnTimer = 0.5f;
				m_arrowHitObject = true;
				float3 target = m_arrow.getPosition() + m_arrowVelocity * castray * dt;
				m_arrow.setPosition(target);
				AudioHandler::getInstance()->playOnceByDistance(
					AudioHandler::HIT_WOOD, m_bow.getPosition(), target);
			}
			else {
				arrowPhysics(dt, wind); // Updates arrow in flight, wind is no longer hard coded.
				m_arrow.setPosition(m_arrow.getPosition() + m_arrowVelocity * dt);
				m_arrow.setRotation(float3(m_arrowPitch, m_arrowYaw, 0));
			}
		}
		if (m_arrowReturnTimer < 0.0f ||
			(m_arrow.getPosition() - playerPos).LengthSquared() >
				m_maxTravelLengthSquared) { // replace with collision later
			m_shooting = false;
		}
	}
	else {
		if (m_charging) {
			// Move arrow with bowstring. Hardcoded values determined by experimentation.
			m_arrow.setPosition(
				m_bow.getPosition() + playerForward * 0.3f * (1.0f - 1.6f * m_drawFactor));
		}
		else {
			m_arrow.setPosition(m_bow.getPosition() + playerForward * 0.3f);
		}
		m_arrow.setRotationMatrix(m_bow.getRotationMatrix());
	}

	// Move bow towards the center while aiming.
	if (m_aiming) {
		m_aimMovement = max(0.2f, m_aimMovement - dt * 4.0f);
	}
	else {
		m_aimMovement = min(1.0f, m_aimMovement + dt * 4.0f);
	}

	m_aiming = false;
}

void Bow::draw() {
	m_bow.draw_animate();
	m_arrow.draw();
}

void Bow::rotate(float pitch, float yaw) {
	m_bow.setRotation(float3(pitch, yaw, -BOW_ANGLE * m_aimMovement));
}

void Bow::aim() { m_aiming = true; }

void Bow::release() { // Stops charging
	m_charging = false;
	m_chargeReset = false;
}

void Bow::charge() { // Draws the arrow back on the bow
	if (!m_shooting && m_chargeReset) {
		m_charging = true;
	}
}

void Bow::shoot(
	float3 direction, float3 startVelocity, float pitch, float yaw) { // Shoots/fires the arrow
	m_chargeReset = true;

	if (m_charging) {
		m_charging = false;
		m_shooting = true;
		m_arrowReturnTimer = m_arrowTimeBeforeReturn;
		m_arrowHitObject = false;

		float bowEfficiencyConstant = 400.0f;
		float bowMaterialConstant = 0.05f;

		float velocity = pow((bowEfficiencyConstant * m_drawFactor) /
								 (m_arrowMass + m_bowMass * bowMaterialConstant),
			0.5f);

		direction.Normalize();

		m_arrowPitch = pitch;
		m_arrowYaw = yaw;

		float3 arrowStartVelocity =
			float3(abs(direction.x), 0.0f, abs(direction.z)) * startVelocity;

		m_arrowVelocity =
			arrowStartVelocity + direction * velocity; // adds player velocity and it looks okay
		m_oldArrowVelocity = m_arrowVelocity;		   // Required to calc rotation
		if (m_drawFactor > 0.5) {
			AudioHandler::getInstance()->playOnce(AudioHandler::HEAVY_ARROW);
		}
		else {
			AudioHandler::getInstance()->playOnce(AudioHandler::LIGHT_ARROW);
		}
	}
}

bool Bow::isShooting() const { return m_shooting; }

void Bow::arrowPhysics(float dt, float3 windVector) { // Updates arrow in flight
	// Update acceleration

	float3 relativeVelocity;

	if (windVector.Length() < 0.0001f) {
		relativeVelocity = m_arrowVelocity;
		m_arrowArea = 0.0001f;
	}
	else {
		relativeVelocity = m_arrowVelocity - windVector;
		calcArea(relativeVelocity);
	}

	float totalDragTimesLength = -m_arrowArea * relativeVelocity.Length() / m_arrowMass;

	float3 acceleration = float3(totalDragTimesLength * relativeVelocity.x,
		(totalDragTimesLength * relativeVelocity.y) - 9.82f,
		totalDragTimesLength * relativeVelocity.z);

	m_arrowVelocity += acceleration * dt;

	float angle = calcAngle(m_arrowVelocity, m_oldArrowVelocity);
	m_arrowPitch += angle;

	m_oldArrowVelocity = m_arrowVelocity;
}

void Bow::calcArea(float3 relativeWindVector) {
	float angle = calcAngle(relativeWindVector, m_arrowVelocity);
	m_arrowArea = ((1 - sin(angle)) * 0.0001f) + (sin(angle) * 0.005f);
	ErrorLogger::logFloat3("Rel. wind", relativeWindVector);
}

float Bow::calcAngle(float3 vec1, float3 vec2) {
	float3 normalisedVec1 = vec1;
	normalisedVec1.Normalize();

	float3 normalisedVec2 = vec2;
	normalisedVec2.Normalize();

	return acos(normalisedVec1.Dot(normalisedVec2));
}
