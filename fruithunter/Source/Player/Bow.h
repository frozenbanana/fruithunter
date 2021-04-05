#pragma once
#include "Entity.h"
#include "Arrow.h"
#include "AudioController.h"

class Bow {
private:
	Entity m_bow;	// bow mesh
	Arrow arrow;	// arrow mesh (not used as an arrow, only visual)

	// current transformation of bow and arrow
	float3 m_position_current; // local position
	float3 m_rotation_current; // global rotation
	// desired transformation of bow and arrow
	float3 m_sourcePosition; // local position
	float3 m_sourceRotation; // global rotation

	// bow positioning
	float3 m_bowPositioning_offset0 = float3(0.05f, -0.3f, 0.5f);//holstered
	float3 m_bowPositioning_angle0 = float3(0.4f, 0, -0.8f);
	float3 m_bowPositioning_offset1 = float3(0.1f, -0.05f, 0.55f);//aiming
	//float3 m_bowPositioning_angle1 = float3(0, 0, -0.2f); // cannot be used! Need dynamic angle to make sure arrows travels to crosshair
	float m_bowPositioning_drawForward = 0.2f;
	float m_bowPositioning_rotationVelocityClamp = 0.5f;
	float m_bowPositioning_rotationSpringConstant = 30.f;
	float m_bowPositioning_bowDrag = 5.5f;
	float m_bowPositioning_timeUntilTense = 0.5;
	float m_bowPositioning_stringFriction = 0.0001f / 60.f;
	float m_bowPositioning_stringSpringConstant = 800.f;

	// bow properties
	float m_bowWindup = 0;
	bool m_charging = false;
	float m_drawFactor = 0.0f;
	float m_stringVelocity = 0;
	float m_stringFactor = 0.0f;
	float m_bowMass = 1.4f;

	// arrow properties
	float m_arrowMass = 0.1f;
	float m_arrowLength = 0.5f;

	// arrow recovery
	bool m_waitingForArrowRecovery = false;
	float m_arrowReturnTimer = 0.f;
	float m_arrowTimeBeforeReturn = 1.0f;

	// bow catchup, smoother experiance
	float m_rotationCatchup = pow(0.1f, 14.0f);
	float m_positionCatchup = pow(0.1f, 25.0f);

	// sound
	SoundID m_soundID_stretch = 0;

	//-- Private Functions --

	/* return forward vector of bow mesh. */
	float3 getForward() const;
	float4x4 convertPYRtoMatrix(float3 rotation) const;

	/* Pull bowstring, ramping up the force in the bow. */
	void pull(float dt);
	/* Start to pull */
	void atPull();
	/* Loosen the bowstring, decreasing the force in bow. */
	void loosen(float dt);
	/* Start releasing bowstring */
	shared_ptr<Arrow> atLoosening();

	/* Recover arrow to bow, making it ready to shoot again */
	void recovery_recover();
	/* Reduce the recovery timer */
	void recovery_reduce(float dt);
	/* Automatic update of recovery for arrow, return true if recovered arrow this call. */
	bool update_recovery(float dt);

	/* Set the bow and arrows position */
	void setPosition(float3 position);
	/* Set the bow and arrows rotation */
	void setRotation(float3 rotation);
	/* get desired position from draw factor */
	float3 getDesiredLocalPosition();
	/* get desired rotation from draw factor */
	float3 getDesiredRotation();
	/* update current position to move towards desired position */
	void update_position(float dt);
	/* update current rotation to move towards desired rotation */
	void update_rotation(float dt);

public:
	Bow();
	~Bow();

	float getWindup() const;
	float getDrawFactor() const;

	/* Draw bow mesh. Draw arrow mesh (if not recovering) */
	void draw();

	/* Pull or loosen bow according to parameter, returns arrow pointer if an arrow was shot! */
	shared_ptr<Arrow> update(float dt, bool pulling);

	/* Set base orientation for bow and arrow. (player properties) */
	void setOrientation(float3 position, float3 rotation);

	/* Returns float value on seconds until recovery */
	float recovery() const;
	/* Set recovery time for how long until the arrow will return after being shot. */
	void setRecoveryTime(float timeInSeconds);
	/* Return true if bow is ready to shoot arrow. */
	bool isReady() const;

};