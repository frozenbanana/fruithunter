#pragma once
#include "Input.h"
#include "Camera.h"
#include "Terrain.h"
#include "Bow.h"

class Player {
public:
	Player();
	~Player();

	void update(float dt);

	void draw();

	void bindMatrix();

	void collideObject(Entity& obj);
	bool checkAnimal(float3 animalPos, float range, float throwStrength);

	// GETS
	float3 getPosition() const;
	float3 getCameraPosition() const;
	float3 getForward() const;
	float3 getVelocity() const;
	vector<FrustumPlane> getFrustumPlanes() const;
	CubeBoundingBox getCameraBoundingBox() const;
	vector<float3> getFrustumPoints(float scaleBetweenNearAndFarPlane) const;
	Bow& getBow();
	float getStamina() const;
	void getStaminaBySkillshot(Skillshot skillShot);

	// SETS
	void setPosition(float3 position);

	//States
	bool inHuntermode() const;

	void standsOnObject();
	void activateHunterMode();

private:
	// Keys
	const Keyboard::Keys KEY_FORWARD = Keyboard::W;
	const Keyboard::Keys KEY_BACKWARD = Keyboard::S;
	const Keyboard::Keys KEY_LEFT = Keyboard::A;
	const Keyboard::Keys KEY_RIGHT = Keyboard::D;
	const Keyboard::Keys KEY_DASH = Keyboard::Space;
	const Keyboard::Keys KEY_SPRINT = Keyboard::LeftShift;
	const Keyboard::Keys KEY_HM = Keyboard::F;

	const float3 DEFAULTFORWARD = float3(0.0f, 0.0f, 1.0f);
	const float3 DEFAULTRIGHT = float3(1.0f, 0.0f, 0.0f);
	const float3 DEFAULTUP = float3(0.0f, 1.0f, 0.0f);

	const float PLAYER_HEIGHT = 1.5f; // units above ground
	const float GROUND_FRICTION =
		0.5f; // friction on flat terrain, reduces velocity by percentage per seconds, 0-60.
	const float GROUND_FRICTION_WEAK = 1.0f; // friction on steep terrain, 0-60.
	const float STEEPNESS_BORDER =
		0.6f; // value of dot product when flat terrain goes to steep terrain
	const float ONGROUND_THRESHOLD =
		0.03f; // extra height over terrain until player is not grounded

	float3 m_position = float3(0, 0, 0);
	float3 m_velocity = float3(0, 0, 0);
	float3 m_lastSafePosition = float3(0, 0, 0); // Latest position where player was on ground

	Camera m_camera;
	Bow m_bow;

	// Player behavior
	bool m_godMode = false;
	bool m_chargingBow = false;
	bool m_onGround = false;				 // if player is grounded
	bool m_onSteepGround = false;			//if player is on steep terrain				
	bool m_onEntity = false;							// if player is standing on an object
	float3 m_gravity = float3(0, -1, 0) * 15.f; // direction * strength
	float m_seaHeight = 1.f;					// Height where player falls in sea
	float m_resetTimer = 0.f;					// Timer for respawn time
	float m_resetDelay = 0.5f;					// Number of seconds before respawn
	// movement speed
	float m_speed = 20.f;				// player movement strength
	float m_speedSprintMultiplier = 2.f;// player movement multiplier when sprinting
	float m_speedOnChargingDash = 10.f; // player movement when charging dash
	float m_speedInAir = 2.5f;			// player movement in air
	float m_godModeSpeed = 20.f;		// player movement in godmode
	// stamina
	const float STAMINA_MAX = 1.f;	// max value of sprint
	float m_stamina = 0.0f;	// stamina available
	bool m_staminaConsumed = false; // stamina consumed this frame update
	// sprint
	const float STAMINA_SPRINT_THRESHOLD = 0.0f;   // threshold when sprinting is available
	const float STAMINA_SPRINT_CONSUMPTION = 0.0f; // stamina consumed per seconds
	bool m_sprinting = false;					   // is the player sprinting
	// dash
	float m_dashForce = 11.f;
	const float STAMINA_DASH_COST = 0.0f; // stamina cost of full charged dash
	const float DASHMAXCHARGE = 1.f;	  // Max charge of dash charge in seconds
	const float DASHMINCHARGE = 0.4f;	  // Min charge of dash charge in seconds
	float m_dashCharge = 0.f;			  // charge of dash in seconds
	bool m_chargingDash = false;		  // is the player charging

	// hunter mode
	const float STAMINA_HM_COST = 0.1f;
	bool m_hunterMode = false;

	// Orientation
	float3 m_playerForward = DEFAULTFORWARD;
	float3 m_playerRight = DEFAULTRIGHT;
	float3 m_playerUp = DEFAULTUP;
	float m_cameraPitch = 0, m_cameraYaw = 0;

	//- - - Functions - - -
	void updateBow(float dt, Terrain* terrain);
	void updateCamera();
	void rotatePlayer(float dt);
	void updateGodMode(float dt);

	float3 getMovementForce();
	void checkGround(Terrain* terrain);
	void checkSteepTerrain(Terrain* terrain);
	void calculateTerrainCollision(Terrain* terrain, float dt);

	void checkSprint(float dt);
	void checkDash(float dt);
	void checkPlayerReset(float dt); // Resets player if below sea level
	void checkHunterMode();

	/*
	 * Modifies m_velocity to have a sliding effect
	 */
	void slide(float td, float3 normal, float l);
	float clamp(float x, float high, float low);
	float getPlayerMovementSpeed() const;

	/*
	 * consumes stamina and next time restoreStamina() is called it will fail. Doing so that the
	 * stamina only restores when not being used.
	 */
	void consumeStamina(float amount);
	void restoreStamina(float amount);

	void updateVelocity_inAir(float3 playerForce, float dt);
	void updateVelocity_onFlatGround(float3 playerForce, float dt);
	void updateVelocity_onSteepGround(float dt);

	// Ability
	void updateHunterMode(float);
};
