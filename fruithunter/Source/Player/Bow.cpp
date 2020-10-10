#include "Bow.h"
#include "AudioHandler.h"
#include "ErrorLogger.h"
#include "SceneManager.h"
#include "GlobalNamespaces.h"

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.2f);
}

Bow::~Bow() {}

void Bow::draw() {
	m_bow.draw_animate();
	if (!m_waitingForArrowRecovery)
		arrow.draw();
}

void Bow::update_rotation(float pitch, float yaw) {
	float dt = SceneManager::getScene()->getDeltaTime();

	float3 rotation_holstered = float3(pitch, yaw, 0) + m_bowPositioning_angle0;
	float3 rotation_aiming = float3(pitch, yaw, 0) + m_bowPositioning_angle1;
	m_rotation_desired = (rotation_holstered * (1 - m_drawFactor) + rotation_aiming * m_drawFactor);
	m_rotation += (m_rotation_desired - m_rotation) * Clamp<float>(dt * m_rotationCatchup, 0, 1);
	m_bow.setRotation(m_rotation);
	arrow.setRotation(m_bow.getRotation());
}

void Bow::atPull() {
	m_charging = true; 
}

void Bow::pull(float dt) {

	AudioHandler::getInstance()->playInstance(AudioHandler::STRETCH_BOW, m_drawFactor);
	m_bowWindup = Clamp<float>(m_bowWindup + dt / m_bowPositioning_timeUntilTense, 0, 1);
	m_drawFactor = 1.f - pow(m_bowWindup - 1, 2);
	m_stringFactor = Clamp<float>(m_drawFactor,0,0.999); // bow animation is fixed to the draw factor
	m_bow.updateAnimatedSpecific(m_stringFactor);
	m_bow.setFrameTargets(0, 1);
}

shared_ptr<Arrow> Bow::atLoosening() { 
	AudioHandler::getInstance()->playOnce(AudioHandler::HEAVY_ARROW);
	// release
	m_charging = false; // stop charging
	// start arrow recovery
	m_waitingForArrowRecovery = true;
	m_arrowReturnTimer = m_arrowTimeBeforeReturn;
	// spawn arrow in world
	float bowEfficiencyConstant = 400.0f*3;
	float bowMaterialConstant = 0.05f;
	float force = pow(
		(bowEfficiencyConstant * m_drawFactor) / (m_arrowMass + m_bowMass * bowMaterialConstant),
		0.5f);
	float3 direction = getForward();
	float3 vel = direction * force;

	//set bow position and rotation to desired stats.
	m_bow.setRotation(m_rotation_desired);
	arrow.setRotation(m_bow.getRotation());
	m_bow.setPosition(m_position_desired + m_sourcePosition);
	arrow.setPosition(m_bow.getPosition() + m_sourceForward * 0.3f * (1.0f - 1.6f * m_drawFactor));

	shared_ptr<Arrow> ret = make_shared<Arrow>();
	ret->initilize(arrow.getPosition_front(), vel);
	return ret;
}

void Bow::loosen(float dt) {

	// update bow factors
	AudioHandler::getInstance()->pauseInstance(AudioHandler::STRETCH_BOW);
	m_drawFactor += (0.f - m_drawFactor) * m_bowPositioning_bowDrag *
					dt; // update draw factor to move towards 0
	m_bowWindup = m_drawFactor;
	m_stringVelocity += (0.f - m_stringFactor) * m_bowPositioning_stringSpringConstant *
						dt; // update spring velocity
	m_stringVelocity *=
		pow(m_bowPositioning_stringFriction, dt); // spring friction, reduce spring velocity
	m_stringFactor += m_stringVelocity * dt;	  // update spring position
	// Update Animation
	m_bow.updateAnimatedSpecific(m_stringFactor);
	m_bow.setFrameTargets(0, 1);
}

void Bow::update_positioning(float dt, float3 position, float3 forward, float3 right) {
	float3 bowForward = getForward();
	m_sourceForward = bowForward;
	m_sourcePosition = position;
	// Set bow position based on player position and direction.
	float3 up = forward.Cross(right);

	float3 position_holstered =
		forward * m_bowPositioning_offset0.z + right * m_bowPositioning_offset0.x +
		up * m_bowPositioning_offset0.y + bowForward * m_drawFactor * m_bowPositioning_drawForward;
	float3 position_aiming =
		(forward * m_bowPositioning_offset1.z + right * m_bowPositioning_offset1.x +
			up * m_bowPositioning_offset1.y) +
		bowForward * m_drawFactor * m_bowPositioning_drawForward;

	float3 drawPosition =
		(position_holstered * (1 - m_drawFactor) + position_aiming * m_drawFactor);
	m_position_desired = drawPosition;
	m_position_current += (m_position_desired - m_position_current) * Clamp<float>(dt * m_positionCatchup,0,1);
	m_bow.setPosition(m_position_current + position);

	// update arrow transformation (is invisible when it should be)
	arrow.setPosition(m_bow.getPosition() + bowForward * 0.3f * (1.0f - 1.6f * m_drawFactor));
}

shared_ptr<Arrow> Bow::update_bow(float dt, bool pulling) { 
	bool debug = false;
	shared_ptr<Arrow> spawnedArrow; // empty at start!
	if (m_charging) {
		if (pulling) {
			if (debug)
				cout << "pulling" << endl;
			pull(dt);
		}
		else {
			if (debug)
				cout << "atRelease" << endl;
			spawnedArrow = atLoosening();
		}
	}
	else {
		if (pulling && !m_waitingForArrowRecovery) {
			if (debug)
				cout << "atPull" << endl;
			// start to pull if started to pull this call and not waiting for arrow.
			atPull();
		}
		else {
			if (debug)
				cout << "release" << endl;
			loosen(dt);
		}
	}
	update_recovery(dt);
	return spawnedArrow;
}

float3 Bow::getForward() const {
	float4x4 mRot = float4x4::CreateRotationZ(m_rotation.z) *
					float4x4::CreateRotationX(m_rotation.x) *
					float4x4::CreateRotationY(m_rotation.y);
	return float3::Transform(float3(0, 0, 1), mRot);
}

void Bow::recovery_recover() { m_waitingForArrowRecovery = false; }

void Bow::recovery_reduce(float dt) { 
	m_arrowReturnTimer = max(m_arrowReturnTimer - dt, 0); 
}

float Bow::recovery() const { return m_arrowReturnTimer; }

void Bow::setRecoveryTime(float timeInSeconds) { m_arrowTimeBeforeReturn = timeInSeconds; }

bool Bow::update_recovery(float dt) {
	if (m_waitingForArrowRecovery) {
		// waiting for arrow recovery
		if (m_arrowReturnTimer <= 0) {
			recovery_recover();
			return true;
		}
		else {
			recovery_reduce(dt);
		}
	}
	return false;
}

bool Bow::isReady() const { return !m_waitingForArrowRecovery; }
