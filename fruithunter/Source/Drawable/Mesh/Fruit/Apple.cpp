#include "Apple.h"
#include "Input.h"
#include "SceneManager.h"
#include "PathFindingThread.h"
#include "AudioController.h"

Apple::Apple(float3 pos) : Fruit(FruitType::APPLE, pos) {
	loadAnimated("Apple", 3);

	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_activeRadius = 12.f;
	m_passiveRadius = 18.f;

	m_groundFriction = 60.f; // 5
	m_airFriction = 60.f;

	m_passive_speed = 6.f;
	m_active_speed = 15.f;
	m_caught_speed = 15.f;
	m_maxSteps = 30;
	m_direction = float3(1.f, 0.f, 0.f);
	setCollisionDataOBB();
}

void Apple::behaviorPassive() {
	changeState(AI::State::ACTIVE);

	// float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	// float terrainHeight =
	// SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition());

	// if (m_onGround) {
	//

	//	if (withinDistanceTo(playerPosition, m_activeRadius)) {
	//		// stopMovement();
	//		changeState(ACTIVE);
	//	}
	//	else {
	//		if (!withinDistanceTo(m_worldHome, ARRIVAL_RADIUS) && m_onGround) {
	//			// Check if there is no other path on going
	//			if (m_availablePath.empty()) {
	//				if (++m_nrOfTriesGoHome < 10) {
	//					makeReadyForPath(m_worldHome); // go home
	//				}
	//				else {
	//					setWorldHome(getPosition() + float3(.0001f, 0, 0.001f));
	//					m_nrOfTriesGoHome = 0;
	//				}
	//				m_speed = m_passive_speed;
	//			}
	//			else {
	//				float3 jumpTo = (m_availablePath.back() - getPosition());
	//				jumpTo.Normalize();
	//				jumpTo.y = 1.f;
	//				jump(jumpTo, 1.f);
	//			}
	//		}
	//		else { // Just jump when home

	//			// m_speed = 0.f;
	//			jump(float3(0.0f, 1.0f, 0.0), 7.f);
	//			SoundID sID = AudioController::getInstance()->play("jump2");
	//			AudioController::getInstance()->scaleVolumeByDistance(sID,
	//(SceneManager::getScene()->m_camera.getPosition()-getPosition()).Length(), 1, 40);
	//			AudioController::getInstance()->setPitch(sID, RandomFloat(-1.f, 1.f) * 0.5f);

	//			m_nrOfJumps++;
	//			if (m_nrOfJumps >= MAXNROFJUMPS) {
	//				float3 newHome = m_worldHome;
	//				newHome += float3(RandomFloat(-10.f, 10.f), 0.f, RandomFloat(-10.f, 10.f));
	//				newHome.y = SceneManager::getScene()->m_terrains.getHeightFromPosition(newHome);
	//				auto pft = PathFindingThread::getInstance();
	//				if (isValid(newHome)) {
	//					m_worldHome = newHome;
	//					m_nrOfJumps = 0;
	//				}
	//
	//			}
	//		}
	//	}
	//}
	// lookTo(m_velocity * float3(1, 0, 1));

	//// respawn if fall into water
	// if (getPosition().y < 1) {
	//	respawn();
	//}
}

void Apple::behaviorActive() {
	float dt = SceneManager::getScene()->getDeltaTime();

	// collision
	float3 ray_point = getBoundingBoxPos() - float3(0, 1, 0) * getHalfSizes();
	float3 ray_distance = m_velocity * dt;
	float3 intersection_position, intersection_normal;
	if (rayCastWorld(ray_point, ray_distance, intersection_position, intersection_normal)) {
		intersection_normal.Normalize();
		float3 up(0, 1, 0);
		if (intersection_normal.Dot(up) >= 0.7f) {
			// flat
			// jump
			float3 target = findJumpLocation(5, 3, 16, 10);
			jumpToLocation(target);
			playSound_bounce();
			// set looking direction
			float3 flatVel = m_velocity * float3(1, 0, 1);
			if (flatVel.Length() > 0) {
				m_desiredLookDirection = Normalize(flatVel);
				// lookTo(flatVel);
			}
			// reset animation
			m_frameTime = 0;
		}
		else {
			// steep
			// slide downhill
			m_velocity -= m_velocity.Dot(intersection_normal) *
						  intersection_normal; // remove force against normal
		}
	}

	// look towards desired direction
	m_currentLookDirection += (m_desiredLookDirection - m_currentLookDirection) *
							 Clamp<float>(1 - pow(rotationSpeed, dt), 0.f, 1.f);
	m_currentLookDirection.Normalize();
	lookTo(m_currentLookDirection);
}

void Apple::behaviorCaught() { changeState(ACTIVE); }

bool Apple::isValid(float3 point) {
	float3 normal = SceneManager::getScene()->m_terrains.getNormalFromPosition(point);
	normal.Normalize();
	// Don't you climb no walls
	if (point.y < 1.f)
		return false;


	if (abs(float3(0.0f, 1.0f, 0.0f).Dot(normal)) < 0.87f)
		return false;

	vector<shared_ptr<Entity>*> objects =
		SceneManager::getScene()->m_entities.getElementsByPosition(point);
	for (size_t i = 0; i < objects.size(); ++i) {
		if (!(*objects[i])->getIsCollidable())
			continue;

		float3 newPoint = (*objects[i])->getPosition() - point;
		newPoint.Normalize();
		newPoint.y = (*objects[i])->getPosition().y;
		newPoint *= 0.7f;
		newPoint += point;

		if ((*objects[i])->checkCollision(newPoint))
			return false;
	}


	return true;
}

void Apple::jumpToLocation(float3 target) {
	float3 source = getPosition();
	float p_start = source.y; // start height
	float p_end = target.y;
	float g = m_gravity.y;	   // gravity
	float H = p_end - p_start; // jump difference
	float errorJump = RandomFloat(1.f, 1.5f);
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

float3 Apple::findJumpLocation(
	float range, float maxHeight, size_t samples, float playerAvoidRange) {
	// search best jump target
	Environment* terrain = m_boundTerrain;

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

bool Apple::validJumpTarget(float3 target) {
	Environment* terrain = m_boundTerrain;
	// check if valid point
	if (!terrain->validPosition(target))
		return false;
	// check if secure
	const float secureDistance = 0.25f;
	for (size_t i = 0; i < 4; i++) {
		float r = ((float)i / 4) * XM_PI * 2;
		float3 adjacentPointFlat = target + float3(cos(r), 0, sin(r) * secureDistance);
		//float3 adjacentPointTilt = adjacentPointFlat;
		//adjacentPointTilt.y =
		//	terrain->getHeightFromPosition(adjacentPointTilt.x, adjacentPointTilt.z);
		//float3 toFlat = Normalize(adjacentPointFlat - target);
		//float3 toTilt = Normalize(adjacentPointFlat - target);
		//float3 normal =
		//	Normalize(Normalize(toFlat.Cross(toTilt)).Cross(toTilt));
		//if (abs(normal.Dot(float3(0, 1, 0))) > 0.7f)
		//	return false; // too much tilt (unsecure)
		if (!terrain->validPosition(adjacentPointFlat))
			return false; // invalid adjacent point (unsecure)
	}
	return true;
}

void Apple::playSound_bounce() {
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 0.2f, 25.f);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1.f, -0.25f));
}

void Apple::updateAnimated(float dt) {

	vector<int> frameOrder = { 0, 2, 0 }; // Order of using keyframes
	vector<float> frameTime = { 0.1f, 0.4f };

	// vector<int> frameOrder = { 1, 2, 0, 1 }; // Order of using keyframes
	// vector<float> frameTime = { 0.1f, 0.2f, 0.2f };

	float animLoopTime = 0;
	for (size_t i = 0; i < frameTime.size(); i++)
		animLoopTime += frameTime[i];

	m_frameTime = Clamp<float>(m_frameTime + dt, 0, animLoopTime); // increment animation

	float time = frameTime[0];
	int frame = 0;
	while (frame + 1 < frameTime.size() && m_frameTime > time) {
		frame++;
		time += frameTime[frame];
	}
	float rest = (m_frameTime - (time - frameTime[frame])) / frameTime[frame];
	float smoothRest = -cos(rest * XM_PI) * 0.5f + 0.5f;
	m_meshAnim.updateSpecific(smoothRest);
	m_meshAnim.setFrameTargets(frameOrder[frame], frameOrder[frame + 1]);
}

void Apple::flee(float3 playerPos) {
	// Update fleeing path if ther is none
	if (m_availablePath.empty()) {
		float3 runTo = getPosition() - playerPos;
		runTo.Normalize();
		runTo *= m_passiveRadius;
		runTo += getPosition();
		makeReadyForPath(runTo);
	}
	// set new velocity from path
}
void Apple::pathfinding(float3 start) {
	// ErrorLogger::log("thread starting for pathfinding");
	auto pft = PathFindingThread::getInstance();

	if (m_readyForPath) {

		m_availablePath.clear();
		if (!isValid(start)) {
			float3 newUnstuck = m_destination - start;
			newUnstuck.Normalize();
			newUnstuck += start;
			if (AI::isValid(newUnstuck, newUnstuck, 0.7f)) {
				m_availablePath.push_back(newUnstuck);
				m_readyForPath = false;
				return;
			}
		}
		TerrainBatch* tm = &SceneManager::getScene()->m_terrains;
		// enforce start and m_destination to terrain
		float3 startCopy = float3(start.x, tm->getHeightFromPosition(start), start.z);
		float3 m_destinationCopy =
			float3(m_destination.x, tm->getHeightFromPosition(m_destination), m_destination.z);

		shared_ptr<AI::Node> currentNode =
			make_shared<AI::Node>(shared_ptr<AI::Node>(), startCopy, startCopy, m_destinationCopy);
		bool collidedWithSomething = false;
		size_t counter = 0;
		std::vector<shared_ptr<AI::Node>> open;
		std::vector<shared_ptr<AI::Node>> closed;
		std::list<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f), float3(0.f, 0.f, -1.f),
			float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f), float3(1.f, 0.f, 0.f),
			float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f), float3(1.f, 0.f, 1.f) };


		open.push_back(currentNode);
		while (!open.empty() && counter++ < m_maxSteps) {
			quickSort(open, 0, (int)open.size() - 1);
			closed.push_back(open.back());
			open.pop_back();

			// Check to see if we're inside a certain radius of m_destinationCopy location
			shared_ptr<AI::Node> currentNode = closed.back();

			if ((currentNode->position - m_destinationCopy).LengthSquared() < ARRIVAL_RADIUS ||
				counter == m_maxSteps - 1) {
				m_availablePath.clear(); // Reset path

				// Add path steps
				while (currentNode->parent != nullptr) {
					m_availablePath.push_back(currentNode->position);
					currentNode = currentNode->parent;
				}


				if (m_availablePath.size() > 2) {
					m_availablePath.pop_back(); // remove first position because it is the same
												// as startCopy.
				}
				m_readyForPath = false;

				return;
			}

			for (auto childOffset : childPositionOffsets) {

				// Create child AI::Node
				float3 childPosition = currentNode->position + STEP_SCALE * childOffset;
				childPosition.y = tm->getHeightFromPosition(childPosition);

				shared_ptr<AI::Node> child =
					make_shared<AI::Node>(currentNode, childPosition, startCopy, m_destinationCopy);


				// Check if node is in open or closed.
				if (!beingUsed(child, open, closed)) {
					continue;
				}

				if (!AI::isValid(child->position, currentNode->position, 0.7f)) {
					continue;
				}

				// Add child to open
				open.push_back(child);
			}
		}
		while (currentNode->parent != nullptr) {
			m_availablePath.push_back(currentNode->position);
			currentNode = currentNode->parent;
		}
		m_readyForPath = false;
	}
}

void Apple::restartAnimation() { m_frameTime = 0; }

void Apple::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	behaviorActive();
	updateAnimated(dt); // animation stuff
	updateVelocity(dt);
	move(dt);
	updateRespawn();
	enforceOverTerrain(); // force fruit above ground

	// respawn
	if (getPosition().y <= 1)
		respawn();
}

bool Apple::isRespawning() const { return m_respawn_timer != 0; }

void Apple::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }
