#pragma once
#include "Entity.h"
#include "Arrow.h"

class Bow {
private:
	Entity m_bow;	// bow mesh
	Arrow arrow;	// arrow mesh (not used as an arrow, only visual)
	float3 m_rotation; // bow adn arrow rotation (Entity class can't handle rotation)

	// bow positioning
	float3 m_bowPositioning_offset0 = float3(0.05f, -0.3f, 0.5f);//holstered
	float3 m_bowPositioning_angle0 = float3(0.4f, 0, -0.8f);
	float3 m_bowPositioning_offset1 = float3(0.1f, -0.05f, 0.55f);//aiming
	float3 m_bowPositioning_angle1 = float3(0, 0, -0.2f);
	float m_bowPositioning_drawForward = 0.2f;
	float m_bowPositioning_rotationVelocityClamp = 0.5f;
	float m_bowPositioning_rotationSpringConstant = 30.f;
	float m_bowPositioning_bowDrag = 3.5f;
	float m_bowPositioning_stringFriction = 0.0001f / 60.f;
	float m_bowPositioning_stringSpringConstant = 800.f;

	// bow properties
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

	//-- Private Functions --

	/* return forward vector of bow mesh. */
	float3 getForward() const;

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

public:
	Bow();
	~Bow();

	/* Draw bow mesh. Draw arrow mesh (if not recovering) */
	void draw();

	/* Update rotation to bow and arrow according to drawFactor. Base rotation in the parameters. */
	void update_rotation(float pitch, float yaw);
	/* Updated bow and arrow position according to the drawFactor. Center around parameters. */
	void update_positioning(float dt, float3 position, float3 forward, float3 right);
	/* Pull or loosen bow according to parameter, returns arrow pointer if an arrow was shot! */
	shared_ptr<Arrow> update_bow(float dt, bool pulling);

	/* Returns float value on seconds until recovery */
	float recovery() const;
	/* Set recovery time for how long until the arrow will return after being shot. */
	void setRecoveryTime(float timeInSeconds);
	/* Return true if bow is ready to shoot arrow. */
	bool isReady() const;

};