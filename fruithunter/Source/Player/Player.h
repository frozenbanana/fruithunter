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
	void update(float dt, Terrain* terrain);

	void draw();
	void collideObject(Entity& obj);
	bool checkAnimal(float3 animalPos, float range, float throwStrength);

	float3 getPosition() const;
	float3 getCameraPosition() const;
	float3 getForward() const;
	float3 getVelocity() const;
	Entity& getArrow() { return m_bow.getArrow(); };
	float getStamina() const;
	bool isShooting() const;
	void setPosition(float3 position);
	void standsOnObject();
	Bow& getBow();

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
		0.5f; // friction on flat terrain, reduces velocity by percentage per seconds, 0-60.
	const float GROUND_FRICTION_WEAK = 1.0f; // friction on steep terrain, 0-60.
	const float STEEPNESS_BORDER =
		0.6f; // value of dot product when flat terrain goes to steep terrain
	const float ONGROUND_THRESHOLD = 0.1f; // extra height over terrain until player is not grounded

	float3 m_position;
	float3 m_velocity;

	Camera m_camera;
	Bow m_bow;

	// Player behavior
	bool m_godMode = false;
	bool m_onGround;							// if player is grounded
	bool m_onEntity;							// if player is standing on an object
	float3 m_gravity = float3(0, -1, 0) * 15.f; // direction * strength
	float3 m_lastSafePosition;					// Latest position where player was on ground
	float m_seaHeight = 1.f;					// Height where player falls in sea
	float m_resetTimer = 0.f;					// Timer for respawn time
	float m_resetDelay = 0.5f;					// Number of seconds before respawn
	// movement speed
	float m_speed = 20.f;				// player movement strength
	float m_speedSprint = 40.f;			// player movement strength when sprinting
	float m_speedOnChargingDash = 10.f; // player movement when charging dash
	float m_speedInAir = 5.f;			// player movement in air
	float m_godModeSpeed = 20.f;		// player movement in godmode
	// jump
	float m_jumpForce = 5.f; // strength of jump force
	bool m_jumpReset = true;
	// stamina
	const float STAMINA_MAX = 1.f;	// max value of sprint
	float m_stamina = STAMINA_MAX;	// stamina available
	bool m_staminaConsumed = false; // stamina consumed this frame update
	// sprint
	const float STAMINA_SPRINT_THRESHOLD = 0.5f;   // threshold when sprinting is available
	const float STAMINA_SPRINT_CONSUMPTION = 0.2f; // stamina consumed per seconds
	bool m_sprinting = false;					   // is the player sprinting
	// dash
	float m_dashForce = 10.f;
	const float STAMINA_DASH_COST = 0.9f; // stamina cost of full charged dash
	const float DASHMAXCHARGE = 1.f;	  // Max charge of dash charge in seconds
	float m_dashCharge = 0.f;			  // charge of dash in seconds
	bool m_chargingDash = false;		  // is the player charging

	// Orientation
	float3 m_playerForward = DEFAULTFORWARD;
	float3 m_playerRight = DEFAULTRIGHT;
	float3 m_playerUp = DEFAULTUP;
	float m_cameraPitch, m_cameraYaw;
	float m_aimZoom = 1.f;
	bool m_releasing = false;

	//- - - Functions - - -
	void updateBow(float dt, Terrain* terrain);
	void updateCamera();
	void rotatePlayer(float dt);
	void updateGodMode(float dt);

	float3 getMovementForce();
	bool onGround(Terrain* terrain);
	float getSteepness(Terrain* terrain);
	void calculateTerrainCollision(Terrain* terrain, float dt);

	void checkJump();
	void checkSprint(float dt);
	void checkDash(float dt);
	void checkPlayerReset(float dt); // Resets player if below sea level

	/*
	 * Modifies m_velocity to have a sliding effect
	 */
	void slide(float td, float3 normal, float l);
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
