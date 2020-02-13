#pragma once
#include "Input.h"
#include "Camera.h"
#include "Terrain.h"
#include "Bow.h"

class Player {
public:
	Player();
	~Player();
	void initialize();
	void update(float td, Terrain* terrain);
	void updateBow(float dt);
	void updateCamera();
	void rotatePlayer();
	void draw();

	float3 getPosition() const;
	float3 getCameraPosition() const;
	float3 getForward() const;
	float3 getVelocity() const;
	Entity& getArrow() { return m_bow.getArrow(); };
	void setPosition(float3 position);

private:
	// Keys
	const Keyboard::Keys KEY_FORWARD = Keyboard::W;
	const Keyboard::Keys KEY_BACKWARD = Keyboard::S;
	const Keyboard::Keys KEY_LEFT = Keyboard::A;
	const Keyboard::Keys KEY_RIGHT = Keyboard::D;
	const Keyboard::Keys KEY_JUMP = Keyboard::Space;
	const Keyboard::Keys KEY_DASH = Keyboard::LeftControl;
	const Keyboard::Keys KEY_SPRINT = Keyboard::LeftShift;

	const float3 DEFAULTFORWARD = float3(0.0f, 0.0f, 1.0f);
	const float3 DEFAULTRIGHT = float3(1.0f, 0.0f, 0.0f);
	const float3 DEFAULTUP = float3(0.0f, 1.0f, 0.0f);

	const float PLAYER_HEIGHT = 1.5f; // meters above ground
	const float GROUND_FRICTION =
		0.9f; // friction on flat terrain, reduces velocity by percentage per seconds
	const float GROUND_FRICTION_WEAK = 0.99f; // friction on steep terrain, --||--
	const float STEEPNESS_BORDER =
		0.6f; // value of dot product when flat terrain goes to steep terrain
	const float ONGROUND_THRESHOLD =
		0.025f; // extra height over terrain until player is not grounded


	float3 m_position;
	float3 m_velocity;

	Camera m_camera;
	Bow m_bow;

	// Player behavior
	bool m_onGround;						 // if player is grounded
	float3 m_gravity = float3(0, -1, 0) * 5; // direction * strength
	// movement speed
	float m_speed = 10.f;			   // player movement strength
	float m_speedSprint = 25.f;		   // player movement strength when sprinting
	float m_speedOnChargingDash = 2.f; // player movement when charging dash
	float m_speedInAir = 2.f;		   // player movement in air
	// jump
	float m_jumpForce = 2.f; // strength of jump force
	// stamina
	const float STAMINA_MAX = 1.f;	// max value of sprint
	float m_stamina = 1.f;			// stamina available
	bool m_staminaConsumed = false; // stamina consumed this frame update
	// sprint
	const float STAMINA_SPRINT_THRESHOLD = 0.5f;  // threshold when sprinting is available
	const float STAMINA_SPRINT_CONSUMPTION = 1.f; // stamina consumed per seconds
	bool m_sprinting = false;					  // is the player sprinting
	// dash
	float m_dashForce = 10.f;
	const float STAMINA_DASH_COST = 0.9f; // stamina cost of full charged dash
	const float DASHMAXCHARGE = 1.f;	  // Max charge of dash charge in seconds
	float m_dashCharge = 0.f;			  // charge of dash in seconds
	bool m_chargingDash = false;		  // is the player charging

	// Inventory
	size_t m_inventory[FRUITS]; // APPLE 0, BANANA 1, MELON 2

	// Orientation
	float3 m_playerForward = DEFAULTFORWARD;
	float3 m_playerRight = DEFAULTRIGHT;
	float3 m_playerUp = DEFAULTUP;
	float m_cameraPitch, m_cameraYaw;
	float m_aimZoom;
	bool m_releasing;

	//- - - Functions - - -
	/*
	 * Modifies m_velocity to have a sliding effect
	 */
	void slide(float td, Vector3 normal, float l);
	float clamp(float x, float high, float low);
	float getPlayerMovementSpeed() const;

	/*
	 * consumes stamina and next time restoreStamina() is called it will fail. Making so that the
	 * stamina only restores when not being used
	 */
	void consumeStamina(float amount);
	void restoreStamina(float amount);

	void updateVelocity_inAir(float3 playerForce, float dt);
	void updateVelocity_onFlatGround(float3 playerForce, float dt);
	void updateVelocity_onSteepGround(float dt);
};
