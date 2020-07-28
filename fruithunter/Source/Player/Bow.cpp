#include "Bow.h"
#include "AudioHandler.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

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
	float3 rotation_holstered = float3(pitch, yaw, 0) + m_bowPositioning_angle0;
	float3 rotation_aiming = float3(pitch, yaw, 0) + m_bowPositioning_angle1;
	m_rotation = (rotation_holstered * (1 - m_drawFactor) + rotation_aiming * m_drawFactor);
	m_bow.setRotation(m_rotation);
	//arrow.setRotationMatrix(m_bow.getRotationMatrix());
	arrow.setRotation(m_bow.getRotation());
}

void Bow::atPull() {
	m_charging = true; 
}

void Bow::pull(float dt) {
	AudioHandler::getInstance()->playInstance(AudioHandler::STRETCH_BOW, m_drawFactor);
	m_drawFactor += (1.f - m_drawFactor) * m_bowPositioning_bowDrag * dt;// update draw factor to move towards 1
	m_stringFactor = m_drawFactor;// bow animation is fixed to the draw factor
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
	float bowEfficiencyConstant = 400.0f;
	float bowMaterialConstant = 0.05f;
	float force = pow(
		(bowEfficiencyConstant * m_drawFactor) / (m_arrowMass + m_bowMass * bowMaterialConstant),
		0.5f);
	// float3 direction = SceneManager::getScene()->m_player->getForward();
	float3 direction = getForward();
	float3 vel = direction * force;

	shared_ptr<Arrow> ret = make_shared<Arrow>();
	ret->initilize(arrow.getPosition_front(), vel);
	return ret;

	// OLD SHOT CODE
	//if (m_charging && 0) {
	//	m_charging = false;
	//	m_shooting = true;
	//	m_arrowReturnTimer = m_arrowTimeBeforeReturn;
	//	m_arrowHitObject = false;

	//	float bowEfficiencyConstant = 400.0f;
	//	float bowMaterialConstant = 0.05f;

	//	float velocity = pow((bowEfficiencyConstant * m_drawFactor) /
	//							 (m_arrowMass + m_bowMass * bowMaterialConstant),
	//		0.5f);

	//	direction.Normalize();

	//	m_arrowPitch = pitch;
	//	m_arrowYaw = yaw;

	//	float3 arrowStartVelocity =
	//		float3(abs(direction.x), 0.0f, abs(direction.z)) * startVelocity;

	//	m_arrowVelocity =
	//		arrowStartVelocity + direction * velocity; // adds player velocity and it looks okay
	//	m_oldArrowVelocity = m_arrowVelocity;		   // Required to calc rotation
	//	if (m_drawFactor > 0.5) {
	//		AudioHandler::getInstance()->playOnce(AudioHandler::HEAVY_ARROW);
	//	}
	//	else {
	//		AudioHandler::getInstance()->playOnce(AudioHandler::LIGHT_ARROW);
	//	}
	//}
}

void Bow::loosen(float dt) {
	// update bow factors
	AudioHandler::getInstance()->pauseInstance(AudioHandler::STRETCH_BOW);
	m_drawFactor += (0.f - m_drawFactor) * m_bowPositioning_bowDrag * dt;// update draw factor to move towards 0
	m_stringVelocity += (0.f - m_stringFactor) * m_bowPositioning_stringSpringConstant * dt;// update spring velocity
	m_stringVelocity *= pow(m_bowPositioning_stringFriction, dt);// spring friction, reduce spring velocity
	m_stringFactor += m_stringVelocity * dt;// update spring position
	// Update Animation
	m_bow.updateAnimatedSpecific(m_stringFactor);
	m_bow.setFrameTargets(0, 1);
}

void Bow::update_positioning(float dt, float3 position, float3 forward, float3 right) {
	float3 bowForward = getForward();
	// Set bow position based on player position and direction.
	float3 up = forward.Cross(right);
	float3 position_holstered =
		position + 
		forward * m_bowPositioning_offset0.z + 
		right * m_bowPositioning_offset0.x +
		up * m_bowPositioning_offset0.y + 
		bowForward * m_drawFactor * m_bowPositioning_drawForward;
	float3 position_aiming =
		position +
		(forward * m_bowPositioning_offset1.z + 
			right * m_bowPositioning_offset1.x +
			up * m_bowPositioning_offset1.y) +
		bowForward * m_drawFactor * m_bowPositioning_drawForward;

	float3 drawPosition =
		(position_holstered * (1 - m_drawFactor) + position_aiming * m_drawFactor);
	m_bow.setPosition(drawPosition);

	// update arrow transformation (is invisible when it should be)
	arrow.setPosition(m_bow.getPosition() + bowForward * 0.3f * (1.0f - 1.6f * m_drawFactor));
}

shared_ptr<Arrow> Bow::update_bow(float dt, bool pulling) { 
	bool deb = false;
	shared_ptr<Arrow> spawnedArrow; // empty at start!
	if (m_charging) {
		if (pulling) {
			if (deb)
				cout << "pulling" << endl;
			pull(dt);
		}
		else {
			if (deb)
				cout << "atRelease" << endl;
			spawnedArrow = atLoosening();
		}
	}
	else {
		if (pulling && !m_waitingForArrowRecovery) {
			if (deb)
				cout << "atPull" << endl;
			// start to pull if started to pull this call and not waiting for arrow.
			atPull();
		}
		else {
			if (deb)
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
