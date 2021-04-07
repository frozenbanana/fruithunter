#include "Apple.h"
#include "Input.h"
#include "SceneManager.h"
#include "PathFindingThread.h"
#include "AudioController.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Apple", 3);

	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_fruitType = APPLE;

	m_activeRadius = 12.f;
	m_passiveRadius = 18.f;

	m_passive_speed = 6.f;
	m_active_speed = 15.f;
	m_caught_speed = 15.f;
	m_groundFriction = 5.f;
	m_maxSteps = 30;
	m_direction = float3(1.f, 0.f, 0.f);
	setCollisionDataOBB();
}

void Apple::behaviorPassive() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	float terrainHeight = SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition());
	// Check if not at home
	if (getPosition().y <= 1.f) {
		float3 target = m_worldHome - getPosition();
		target.Normalize();
		target.y = 1.f;
		jump(target, 10.f);
		return;
	}
	if (m_onGround) {
		

		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			// stopMovement();
			changeState(ACTIVE);
		}
		else {
			if (!withinDistanceTo(m_worldHome, ARRIVAL_RADIUS) && m_onGround) {
				// Check if there is no other path on going
				if (m_availablePath.empty()) {
					if (++m_nrOfTriesGoHome < 10) {
						makeReadyForPath(m_worldHome); // go home
					}
					else {
						setWorldHome(getPosition() + float3(.0001f, 0, 0.001f));
						m_nrOfTriesGoHome = 0;
					}
					m_speed = m_passive_speed;
				}
				else {
					float3 jumpTo = (m_availablePath.back() - getPosition());
					jumpTo.Normalize();
					jumpTo.y = 1.f;
					jump(jumpTo, 1.f);
				}
			}
			else { // Just jump when home

				// m_speed = 0.f;
				jump(float3(0.0f, 1.0f, 0.0), 7.f);
				SoundID sID = AudioController::getInstance()->play("jump2");
				AudioController::getInstance()->scaleVolumeByDistance(sID, (SceneManager::getScene()->m_camera.getPosition()-getPosition()).Length(), 1, 40);
				AudioController::getInstance()->setPitch(sID, RandomFloat(-1, 1) * 0.5);

				m_nrOfJumps++;
				if (m_nrOfJumps >= MAXNROFJUMPS) {
					float3 newHome = m_worldHome;
					newHome += float3(RandomFloat(-10.f, 10.f), 0.f, RandomFloat(-10.f, 10.f));
					newHome.y = SceneManager::getScene()->m_terrains.getHeightFromPosition(newHome);
					auto pft = PathFindingThread::getInstance();
					if (isValid(newHome)) {
						m_worldHome = newHome;
						m_nrOfJumps = 0;
					}
					
				}
			}
		}
	}
	lookTo(m_velocity * float3(1, 0, 1));
}

void Apple::behaviorActive() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
		stopMovement();
	}
	else {
		flee(playerPosition);
		m_speed = m_active_speed;
		lookTo(m_velocity * float3(1, 0, 1));
	}
}

void Apple::behaviorCaught() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	if (m_onGround) {
		if (!m_hit) {
			jump(float3(0.f, 1.f, 0.f), 15.f);
			m_hit = true;
		}
		m_direction = playerPosition - getPosition();
		lookTo(m_direction * float3(1, 0, 1));
		
		m_speed = m_caught_speed;
	}
	lookTo(playerPosition);
}

bool Apple::isValid(float3 point) { 
	float3 normal = SceneManager::getScene()->m_terrains.getNormalFromPosition(point);
	normal.Normalize();
	// Don't you climb no walls
	if (point.y < 1.f)
		return false;


	if (abs(float3(0.0f, 1.0f, 0.0f).Dot(normal)) < 0.87f)
		return false;

	vector<shared_ptr<Entity>*> objects = SceneManager::getScene()->m_entities.getElementsByPosition(point);
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

void Apple::updateAnimated(float dt) {
	m_startAnimationPosition = getPosition();
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes

	float frameSpeedOrder[] = { 1/4.f, 1/5.f, 1/2.0f, 1/1.9f, 1/4.f, 1/2.f };

	// calc total animation time
	float sum = 0;
	for (size_t i = 0; i < 6; i++)
		sum += frameSpeedOrder[i];

	// increment time
	m_frameTime += dt;

	// find frame
	float time = fmod(m_frameTime, sum);
	int frame = 0;
	while (time >= frameSpeedOrder[frame]) {
		time -= frameSpeedOrder[frame];
		// try next frame
		frame = (frame + 1) % m_nrOfFramePhases;
	}
	m_currentFramePhase = frame; // found frame

	// set frame
	m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
		frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);

	// Update lerp value between morphing
	float animationLerp = time / frameSpeedOrder[frame];
	m_meshAnim.updateSpecific(animationLerp);
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