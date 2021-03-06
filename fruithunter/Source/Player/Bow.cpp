#include "Bow.h"
#include "TerrainManager.h"
#include "AudioHandler.h"
#include "ErrorLogger.h"
#include "VariableSyncer.h"

void onLoad() { ErrorLogger::log("BowLoaded"); }

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.2f);
	m_arrow.load("arrowV3");
	// m_arrow.setScale(float3(0.2f, 0.2f, m_arrowLength));
	m_arrow.setScale(float3(0.5f, 0.5f, m_arrowLength));
	m_arrow.setPosition(float3(-10.f)); // To make sure that arrow doesn't spawn in fruits.
	m_arrow.setCollisionDataOBB();
	m_trailEffect = ParticleSystem(ParticleSystem::ARROW_GLITTER);

	FileSyncer* file = VariableSyncer::getInstance()->create(
		"Bow.txt", FileSyncer::SyncType::state_liveFile, onLoad);
	file->bind("offset0:v3", &m_bowPositioning_offset0);
	file->bind("angle0:v3", &m_bowPositioning_angle0);
	file->bind("offset1:v3", &m_bowPositioning_offset1);
	file->bind("angle1:v3", &m_bowPositioning_angle1);
	file->bind("drawForward:f", &m_bowPositioning_drawForward);
	file->bind("rotation drag:f", &m_bowPositioning_rotationSpringConstant);
	file->bind("bow drag:f", &m_bowPositioning_bowDrag);
	file->bind("string friction:f", &m_bowPositioning_stringFriction);
	file->bind("string drag:f", &m_bowPositioning_stringSpringConstant);
}

Bow::~Bow() {}

void Bow::update(
	float dt, float3 playerPos, float3 playerForward, float3 playerRight, Terrain* terrain) {

	/* Smooth rotation is temporary removed because of the stutters it gives*/

	// rotate to desired rotation
	//float3 rotationChange = m_desiredRotation - m_rotation;
	//(m_desiredRotation - m_rotation) * m_bowPositioning_rotationSpringConstant * dt;
	//	THIS WORKS BUT GIVES STUTTERS
	////clamp rotation velocity
	// float3 maxRot = float3(1.f, 1.f, 1.f) * m_bowPositioning_rotationVelocityClamp;
	// float3 forceRot(0, 0, 0);
	// if (abs(rotationChange.x) > maxRot.x)
	//	forceRot.x = Frac(abs(rotationChange.x) / maxRot.x) * rotationChange.x;
	// if (abs(rotationChange.y) > maxRot.y)
	//	forceRot.y = Frac(abs(rotationChange.y) / maxRot.y) * rotationChange.y;
	// if (abs(rotationChange.z) > maxRot.z)
	//	forceRot.z = Frac(abs(rotationChange.z) / maxRot.z) * rotationChange.z;
	// rotationChange -= forceRot;
	// set rotation
	// m_rotation =
	//	forceRot + rotationChange * m_bowPositioning_rotationSpringConstant * dt + m_rotation;
	//m_rotation += rotationChange * Clamp(m_bowPositioning_rotationSpringConstant * dt, 0, 1.f);

	float3 forward = getForward();
	// Set bow position based on player position and direction.
	float3 playerUp = playerForward.Cross(playerRight);
	float3 position_holstered = playerPos + playerForward * m_bowPositioning_offset0.z +
								playerRight * m_bowPositioning_offset0.x +
								playerUp * m_bowPositioning_offset0.y +
								forward * m_drawFactor * m_bowPositioning_drawForward;
	float3 position_aiming =
		playerPos +
		(playerForward * m_bowPositioning_offset1.z + playerRight * m_bowPositioning_offset1.x +
			playerUp * m_bowPositioning_offset1.y) +
		forward * m_drawFactor * m_bowPositioning_drawForward;

	float3 drawPosition = (position_holstered * (1 - m_drawFactor) + position_aiming * m_drawFactor);
	m_bow.setPosition(drawPosition);

	// Update m_arrowReturnTimer
	m_arrowReturnTimer -= dt;
	// Bow animation.
	if (m_charging) {
		AudioHandler::getInstance()->playInstance(AudioHandler::STRETCH_BOW, m_drawFactor);
		m_drawFactor += (1.f - m_drawFactor) * m_bowPositioning_bowDrag * dt;
		m_stringFactor = m_drawFactor;
		m_bow.updateAnimatedSpecific(m_stringFactor);
		m_bow.setFrameTargets(0, 1);
	}
	else {
		AudioHandler::getInstance()->pauseInstance(AudioHandler::STRETCH_BOW);
		m_drawFactor += (0.f - m_drawFactor) * m_bowPositioning_bowDrag * dt;
		m_stringVelocity += (0.f - m_stringFactor) * m_bowPositioning_stringSpringConstant * dt;
		m_stringVelocity *= pow(m_bowPositioning_stringFriction, dt);
		m_stringFactor += m_stringVelocity * dt;
		m_bow.updateAnimatedSpecific(m_stringFactor);
		m_bow.setFrameTargets(0, 1);
	}

	// m_trailEffect.update(dt);

	// Update arrow.
	if (m_shooting) {
		if (!m_arrowHitObject) {
			float castray =
				TerrainManager::getInstance()->castRay(m_arrow.getPosition(), m_arrowVelocity * dt);
			if (castray != -1) {
				m_trailEffect.setEmitState(false);
				// Arrow is hitting terrain
				float3 target = m_arrow.getPosition() + m_arrowVelocity * castray * dt;
				arrowHitObject(target);
			}
			else if(terrain != nullptr){
				arrowPhysics(dt, terrain->getWindStatic()); // Updates arrow in flight, wind is no
															// longer hard coded.
				// update Particle System
				m_trailEffect.setPosition(m_arrow.getPosition());
				m_trailEffect.setEmitState(true);

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
		m_trailEffect.setEmitState(false);
		if (m_charging) {
			// Move arrow with bowstring. Hardcoded values determined by experimentation.
			m_arrow.setPosition(
				m_bow.getPosition() + forward * 0.3f * (1.0f - 1.6f * m_drawFactor));
		}
		else {
			m_arrow.setPosition(m_bow.getPosition() + forward * 0.3f);
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
	float3 rotation_holstered = float3(pitch, yaw, 0) + m_bowPositioning_angle0;
	float3 rotation_aiming = float3(pitch, yaw, 0) + m_bowPositioning_angle1;
	m_rotation = (rotation_holstered * (1 - m_drawFactor) + rotation_aiming * m_drawFactor);
	m_bow.setRotation(m_rotation);
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

ParticleSystem& Bow::getTrailEffect() { return m_trailEffect; }

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

float3 Bow::getArrowVelocity() const { return m_arrowVelocity; }

bool Bow::isShooting() const { return m_shooting; }

bool Bow::getArrowHitObject() const { return m_arrowHitObject; }

void Bow::arrowHitObject(float3 target) {
	m_arrowReturnTimer = 0.5f;
	m_arrowHitObject = true;
	// float3 target = m_arrow.getPosition() + m_arrowVelocity * castray * dt;
	m_arrow.setPosition(target);
	AudioHandler::getInstance()->playOnceByDistance(
		AudioHandler::HIT_WOOD, m_bow.getPosition(), target);
}

void Bow::arrowPhysics(float dt, float3 windVector) { // Updates arrow in flight
	// Update acceleration

	float3 relativeVelocity;

	if (windVector.Length() < 0.0001f) {
		relativeVelocity = m_arrowVelocity;
		m_arrowArea = 0.0001f;
	}
	else {
		relativeVelocity = m_arrowVelocity + windVector;
		calcArea(relativeVelocity);
	}

	float totalDragTimesLength = -m_arrowArea * relativeVelocity.Length() / m_arrowMass;

	float3 acceleration = float3(totalDragTimesLength * relativeVelocity.x,
		(totalDragTimesLength * relativeVelocity.y) - 9.82f,
		totalDragTimesLength * relativeVelocity.z);

	m_arrowVelocity += (acceleration + windVector) * dt;

	float angle = calcAngle(m_arrowVelocity, m_oldArrowVelocity);
	m_arrowPitch += angle;

	m_oldArrowVelocity = m_arrowVelocity;
}

void Bow::calcArea(float3 relativeWindVector) {
	float angle = calcAngle(relativeWindVector, m_arrowVelocity);
	m_arrowArea = ((1 - sin(angle)) * 0.0001f) + (sin(angle) * 0.005f);
}

float Bow::calcAngle(float3 vec1, float3 vec2) {
	float3 normalisedVec1 = vec1;
	normalisedVec1.Normalize();

	float3 normalisedVec2 = vec2;
	normalisedVec2.Normalize();
	float soonAngle = normalisedVec1.Dot(normalisedVec2);
	soonAngle = min(1.f, max(-1.f, soonAngle)); // clamped to avoid NaN in cos
	return acos(soonAngle);
}

float3 Bow::getForward() const {
	float4x4 mRot = float4x4::CreateRotationZ(m_rotation.z) *
					float4x4::CreateRotationX(m_rotation.x) *
					float4x4::CreateRotationY(m_rotation.y);
	return float3::Transform(float3(0, 0, 1), mRot);
}
