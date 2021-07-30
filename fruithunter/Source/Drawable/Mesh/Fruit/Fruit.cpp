#include "Fruit.h"
#include "Input.h"
#include "SceneManager.h"
#include "AudioController.h"

void Fruit::draw_fruit() {
	if (m_isVisible) {
		Renderer::getInstance()->setBlendState_NonPremultiplied();
		draw_animate(m_baseColor);
		Renderer::getInstance()->setBlendState_Opaque();
	}
}

void Fruit::draw_fruit_shadow() { draw_animate_onlyMesh(float3(0, 0, 0)); }

Skillshot Fruit::hit(float3 playerPos) {
	const float LONGSHOT = 25.f;
	const float MEDIUMSHOT = 15.f;
	const float FASTMOVING_VELOCITY = 11.f;

	Skillshot hitType = Skillshot::SS_BRONZE;
	float dist = (playerPos - getPosition()).Length();
	if (dist > LONGSHOT) {
		if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
			// gold
			hitType = SS_GOLD;
		}
		else {
			// gold
			hitType = SS_GOLD;
		}
	}
	else if (dist > MEDIUMSHOT) {
		if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
			// case 2: Medium shot
			// in air or fast moving -> gold
			// Gold
			hitType = SS_GOLD;
		}
		else {
			// silver
			hitType = SS_SILVER;
		}
	}
	return hitType;
}

FruitType Fruit::getFruitType() { return m_fruitType; }

void Fruit::enforceOverTerrain() {
	float height = SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition());
	if (atOrUnder(height)) {
		float feet = getBoundingBoxPos().y - getHalfSizes().y;
		float newY = height + (getPosition().y - feet) + 0.01f;
		setPosition(float3(getPosition().x, newY, getPosition().z));
	}
}

void Fruit::checkOnGroundStatus() {
	m_onGround =
		atOrUnder(SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()));
}

void Fruit::bindToEnvironment(Environment* terrain) { m_boundTerrain = terrain; }

void Fruit::onHit() {
	Player* player = SceneManager::getScene()->m_player.get();
	Skillshot skillshot = hit(player->getPosition()); // calculate skillshot
	onHit(skillshot);
}

void Fruit::onHit(Skillshot skillshot) {
	Player* player = SceneManager::getScene()->m_player.get();
	player->getStaminaBySkillshot(skillshot); // give stamina
	onDeath(skillshot);
}

void Fruit::move(float dt) {
	setPosition(getPosition() + m_velocity * dt);
}

Fruit::Fruit(FruitType type, float3 pos) : Entity() {
	m_fruitType = type;
	setPosition(pos);
}

void Fruit::updateVelocity(float dt) {
	float friction = m_onGround ? m_groundFriction : m_airFriction;
	m_velocity += m_gravity * dt;
	m_velocity *= pow((friction / 60.f), dt);
}

void Fruit::respawn() {
	if (!m_respawning) {
		// on respawn
		m_respawning = true;
		m_respawn_timer = m_respawn_timeMax;
		m_startScale = getScale().x;
	}
}

void Fruit::updateRespawn() {
	// -- FRUIT RESPAWNING -- //
	// Change into passive mode to automaticly respawn melon

	if (m_respawning) {
		// update
		float dt = SceneManager::getScene()->getDeltaTime();

		float th = m_respawn_timeMax / 2;
		if (m_respawn_timer >= th && m_respawn_timer - dt < th) {
			// find new respawn point
			if (m_boundTerrain != nullptr) {
				// spawn on bound terrain
				float3 sp = m_boundTerrain->getRandomSpawnPoint();
				setPosition(sp + float3(0.f, 1.f, 0.f) * (getHalfSizes().y + 0.1f));
			}
			else {
				int tIndex =
					SceneManager::getScene()->m_terrains.getTerrainIndexFromPosition(getPosition());
				if (tIndex == -1) {
					// pick random terrain if not on a terrain (Plan B)
					tIndex = rand() % SceneManager::getScene()->m_terrains.size();
				}
				if (tIndex != -1) {
					float3 sp = SceneManager::getScene()->m_terrains[tIndex]->getRandomSpawnPoint();
					setPosition(sp + float3(0.f, 1.f, 0.f) * (getHalfSizes().y + 0.1f));
				}
				else {
					// this should never happen as fruits only can spawn if there is a terrain to
					// spawn from
					ErrorLogger::logError(
						"(Fruit) Fruit cant respawn. No terrains exists!", HRESULT());
				}
			}
			m_velocity *= 0;
		}
		m_respawn_timer = Clamp<float>(m_respawn_timer - dt, 0, m_respawn_timeMax);

		// scaling
		float factor = abs((m_respawn_timeMax / 2) - m_respawn_timer) / (m_respawn_timeMax / 2);
		setScale(m_startScale * factor);

		if (m_respawn_timer == 0) {
			// end of respawn
			m_respawning = false;
		}
	}
}

bool Fruit::rayCastWorld(float3 point, float3 forward, float3& intersection, float3& normal) {
	Scene* scene = SceneManager::getScene();

	float t = -1;
	// find terrain collision
	t = scene->m_terrains.castRay(point, forward);
	if (t > -1) {
		intersection = point + forward * t;
		normal = scene->m_terrains.getNormalFromPosition(intersection);
	}
	// object collisions
	CubeBoundingBox bb(vector<float3>({ point, point + forward }));
	vector<shared_ptr<Entity>*> entities = scene->m_entities.cullElements(bb);
	for (size_t i = 0; i < entities.size(); i++) {
		float3 cast_target, cast_normal;
		if ((*entities[i])->getIsCollidable()) {
			if ((*entities[i])->castRayEx_limitDistance(point, forward, cast_target, cast_normal)) {
				float tt = (cast_target - point).Length() / forward.Length();
				if (tt <= 1 && (t == -1 || tt < t)) {
					t = tt;
					intersection = cast_target;
					normal = cast_normal;
				}
			}
		}
	}
	return (t > -1);
}

bool Fruit::isOnGround(float3 position, float heightThreshold) {
	Scene* scene = SceneManager::getScene();
	float3 forward = float3(0, -1, 0) * heightThreshold;
	float t = -1;
	// find terrain height
	float h = scene->m_terrains.getHeightFromPosition(position);
	if (abs(position.y - h) < heightThreshold)
		return true;
	// object collisions
	vector<shared_ptr<Entity>*> entities = scene->m_entities.getElementsByPosition(position);
	for (size_t i = 0; i < entities.size(); i++) {
		float3 cast_target, cast_normal;
		if ((*entities[i])->getIsCollidable()) {
			if ((*entities[i])
					->castRayEx_limitDistance(position, forward, cast_target, cast_normal)) {
				// hit
				return true;
			}
		}
	}
	return false;
}

void Fruit::spawnCollectionPoint(Skillshot skillshot) {
	shared_ptr<CollectionPoint> cp = make_shared<CollectionPoint>();
	cp->load(getPosition(), float3(0, 1, 0), getFruitType(), skillshot);
	SceneManager::getScene()->m_collectionPoint.push_back(cp);
}

void Fruit::jumpToLocation(float3 target, float2 baseHeight) {
	float3 source = getPosition();
	float p_start = source.y; // start height
	float p_end = target.y;
	float g = m_gravity.y;	   // gravity
	float H = p_end - p_start; // jump difference
	float errorJump = RandomFloat(baseHeight.x, baseHeight.y);
	H = max(H, 0) + errorJump;	// jump height (always positive)
	float v = sqrt(-2 * H * g); // vertical velocity

	float sqrtPart = pow(v / g, 2) - 2 * (p_start - p_end) / g;
	if (sqrtPart < 0)
		return; // impossible jump (do nothing)
	float t0 = -v / g + sqrt(sqrtPart);
	float t1 = -v / g - sqrt(sqrtPart);
	float t = max(t0, t1);

	float3 horizontalMovement = (target - source) * float3(1, 0, 1);
	float3 horizontalVelocity = horizontalMovement / t;
	m_velocity = float3(horizontalVelocity.x, v, horizontalVelocity.z);
}

float3 Fruit::findJumpLocation(
	float range, float maxHeight, size_t samples, float playerAvoidRange) {
	// search best jump target
	Environment* terrain = m_boundTerrain;
	if (terrain == nullptr)
		return getPosition(); // cant analyse terrain

	float score_best = 0;
	float3 finalJumpTarget = getPosition();
	for (size_t i = 0; i < samples; i++) {
		// get point
		float r = RandomFloat(0, XM_PI * 2);
		float horLength = (1 - (float)pow(RandomFloat(0.1f, 1.f), 2)) * range;
		float3 direction = float3(cos(r), 0, sin(r)) * horLength;
		float3 jumpTarget = getPosition() + direction;
		jumpTarget.y = terrain->getHeightFromPosition(jumpTarget.x, jumpTarget.z);

		// check jump location if valid
		float jumpHeightElevation = abs(jumpTarget.y - getPosition().y);
		if (jumpHeightElevation > maxHeight)
			continue; // too high/low jump target
		if (!validJumpTarget(jumpTarget))
			continue;

		// score target
		float score = 0;
		// avoid player
		float distanceToPlayer =
			(SceneManager::getScene()->m_player->getPosition() - jumpTarget).Length();
		if (distanceToPlayer < playerAvoidRange) {
			float withinPlayerFactor = 1 - (distanceToPlayer / playerAvoidRange);
			score -= withinPlayerFactor * 20;
		}
		// keep approx direction
		float3 targetDirection = Normalize((jumpTarget - getPosition()) * float3(1, 0, 1));
		float3 movementDirection = Normalize(m_velocity * float3(1, 0, 1));
		float directionFactor = max(targetDirection.Dot(movementDirection), 0);
		float approxDirectionFactor = 1 - (abs(0.5f - directionFactor) / 0.5f);
		score += approxDirectionFactor * 5;

		// keep best
		if (i == 0 || score > score_best) {
			score_best = score;
			finalJumpTarget = jumpTarget;
		}
	}

	return finalJumpTarget;
}

bool Fruit::validJumpTarget(float3 target) {
	Environment* terrain = m_boundTerrain;
	// check if valid point
	if (!terrain->validPosition(target))
		return false;
	// check if secure
	const float secureDistance = 0.25f;
	for (size_t i = 0; i < 4; i++) {
		float r = ((float)i / 4) * XM_PI * 2;
		float3 adjacentPointFlat = target + float3(cos(r), 0, sin(r) * secureDistance);
		// float3 adjacentPointTilt = adjacentPointFlat;
		// adjacentPointTilt.y =
		//	terrain->getHeightFromPosition(adjacentPointTilt.x, adjacentPointTilt.z);
		// float3 toFlat = Normalize(adjacentPointFlat - target);
		// float3 toTilt = Normalize(adjacentPointFlat - target);
		// float3 normal =
		//	Normalize(Normalize(toFlat.Cross(toTilt)).Cross(toTilt));
		// if (abs(normal.Dot(float3(0, 1, 0))) > 0.7f)
		//	return false; // too much tilt (unsecure)
		if (!terrain->validPosition(adjacentPointFlat))
			return false; // invalid adjacent point (unsecure)
	}
	return true;
}

bool Fruit::isVisible() const { return m_isVisible; }

void Fruit::onDeath(Skillshot skillshot) {
	_onDeath(skillshot);
	markForDeletion();
}
