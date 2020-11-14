#include "Bow.h"
#include "AudioController.h"
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

void Bow::atPull() {
	m_charging = true; 
	m_soundID_stretch = AudioController::getInstance()->play("stretch-bow", AudioController::SoundType::Effect);
}

void Bow::pull(float dt) {
	m_bowWindup = Clamp<float>(m_bowWindup + dt / m_bowPositioning_timeUntilTense, 0, 1);
	m_drawFactor = 1.f - pow(m_bowWindup - 1, 2);
	m_stringFactor = Clamp<float>(m_drawFactor,0,0.999); // bow animation is fixed to the draw factor
	m_bow.updateAnimatedSpecific(m_stringFactor);
	m_bow.setFrameTargets(0, 1);

	//stop stretch sound if bow fully streched
	if (m_bowWindup == 1)
		AudioController::getInstance()->stop(m_soundID_stretch);
}

shared_ptr<Arrow> Bow::atLoosening() { 
	AudioController::getInstance()->stop(m_soundID_stretch);
	AudioController::getInstance()->play("heavy-arrow-release", AudioController::SoundType::Effect);
	// release
	m_charging = false; // stop charging
	// start arrow recovery
	m_waitingForArrowRecovery = true;
	m_arrowReturnTimer = m_arrowTimeBeforeReturn;
	// calculate arrow velocity
	float bowEfficiencyConstant = 400.0f*3;
	float bowMaterialConstant = 0.05f;
	float force = pow(
		(bowEfficiencyConstant * m_drawFactor) / (m_arrowMass + m_bowMass * bowMaterialConstant),
		0.5f);
	float3 direction = getForward();
	float3 vel = direction * force;

	//set bow position and rotation to desired stats.
	setRotation(getDesiredRotation());
	setPosition(getDesiredLocalPosition()+m_sourcePosition);

	//spawn arrow into world
	shared_ptr<Arrow> ret = make_shared<Arrow>();
	ret->initilize(arrow.getPosition_front(), vel);
	return ret;
}

void Bow::loosen(float dt) {

	// update bow factors
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

shared_ptr<Arrow> Bow::update(float dt, bool pulling) { 
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
	update_rotation(dt);
	update_position(dt);
	return spawnedArrow;
}

void Bow::setOrientation(float3 position, float3 rotation) { 
	m_sourcePosition = position;
	m_sourceRotation = rotation;
}

void Bow::setPosition(float3 position) {
	m_bow.setPosition(position);

	// update arrow transformation (is invisible when it should be)
	arrow.setPosition(m_bow.getPosition() + getForward() * 0.3f * (1.0f - 1.6f * m_drawFactor));
}

void Bow::setRotation(float3 rotation) {
	m_bow.setRotation(rotation);
	arrow.setRotation(rotation);
}

float3 Bow::getDesiredLocalPosition() { 
	float dt = SceneManager::getScene()->getDeltaTime();
	float3 bowForward = getForward();
	// Set bow position based on player position and direction.
	float3 forward = float3::Transform(float3(0, 0, 1), convertPYRtoMatrix(m_sourceRotation));
	float3 right = float3::Transform(float3(1, 0, 0), convertPYRtoMatrix(m_sourceRotation));
	float3 up = float3::Transform(float3(0, 1, 0), convertPYRtoMatrix(m_sourceRotation));

	float3 position_holstered =
		forward * m_bowPositioning_offset0.z + right * m_bowPositioning_offset0.x +
		up * m_bowPositioning_offset0.y + bowForward * m_drawFactor * m_bowPositioning_drawForward;
	float3 position_aiming =
		(forward * m_bowPositioning_offset1.z + right * m_bowPositioning_offset1.x +
			up * m_bowPositioning_offset1.y) +
		bowForward * m_drawFactor * m_bowPositioning_drawForward;

	float3 desiredPosition =
		(position_holstered * (1 - m_drawFactor) + position_aiming * m_drawFactor);
	return desiredPosition;

}

float3 Bow::getDesiredRotation() {
	float dt = SceneManager::getScene()->getDeltaTime();

	float3 rotation_holstered = m_sourceRotation + m_bowPositioning_angle0;
	float3 rotation_aiming = m_sourceRotation + m_bowPositioning_angle1;
	float3 desiredRotation = (rotation_holstered * (1 - m_drawFactor) + rotation_aiming * m_drawFactor);
	return desiredRotation;
}

void Bow::update_position(float dt) {
	float3 desiredLocalPosition = getDesiredLocalPosition();
	m_position_current +=
		(desiredLocalPosition - m_position_current) * Clamp<float>(dt * m_positionCatchup, 0, 1);
	setPosition(m_position_current + m_sourcePosition);
}

void Bow::update_rotation(float dt) {

	float3 desiredRotation = getDesiredRotation();
	m_rotation_current += (desiredRotation - m_rotation_current) * Clamp<float>(dt * m_rotationCatchup, 0, 1);
	setRotation(m_rotation_current);
}

float3 Bow::getForward() const {
	return float3::Transform(float3(0, 0, 1), convertPYRtoMatrix(m_rotation_current));
}

float4x4 Bow::convertPYRtoMatrix(float3 rotation) const {
	return float4x4::CreateRotationZ(rotation.z) * float4x4::CreateRotationX(rotation.x) *
		   float4x4::CreateRotationY(rotation.y);
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
