#include "Melon.h"
#include "PathFindingThread.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;
	vector<string> names{ "Melon.mtl", "Melon2bronze.mtl", "Melon2silver.mtl", "Melon2gold.mtl",
		"Melon3.mtl" };
	loadMaterials(names);

	m_meshAnim.setFrameTargets(0, 0);
	m_rollSpeed = 5.f;
	m_fruitType = MELON;

	setScale(0.5);
	changeState(AI::State::PASSIVE);
	setStartPosition(pos);
	// enforce that homes are on terrain
	setWorldHome(m_position);
	m_secondWorldHome = m_worldHome + float3(3.f, 0.0, 3.0f);
	m_secondWorldHome.y = TerrainManager::getInstance()->getHeightFromPosition(m_secondWorldHome);
	m_direction = m_position - m_secondWorldHome;

	m_rollAnimationSpeed = 2.0f;
	setCollisionDataOBB();


	m_passiveRadius = 15.f;
	m_activeRadius = 15.f;
	m_maxSteps = 10;

	m_passive_speed = 8.f;
	m_active_speed = 15.f;
	m_caught_speed = 15.f;

	int rand = 0;
	rand = std::rand();
	if (rand % 2 == 0) {
		m_angleDirection = 1;
	}
	else {
		m_angleDirection = -1;
	}
	m_triesToGoHome = 0;
	m_maxSteps = 16;
}

void Melon::behaviorPassive(float3 playerPosition) {

	if (m_position.y <= 1.f) {
		float3 target = m_worldHome - m_position;
		target.Normalize();
		target.y = 1.f;
		jump(target, 10.f);
		return;
	}


	if (m_onGround) {
		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			changeState(ACTIVE);
			stopMovement();
			return;
		}

		if (withinDistanceTo(m_worldHome, 0.75f)) {
			m_destination = m_secondWorldHome - m_position;
			lookTo(m_secondWorldHome);
		}
		else if (withinDistanceTo(m_secondWorldHome, 0.75f)) {
			m_destination = m_worldHome - m_position;
			lookTo(m_worldHome);
		}
		else if (!withinDistanceTo(m_worldHome, 5.f) && !withinDistanceTo(m_secondWorldHome, 5.f)) {
			m_destination = m_worldHome - m_position;
			lookTo(m_worldHome);
		}
		m_speed = m_passive_speed;
		makeReadyForPath(m_destination);
		
	}
}

void Melon::behaviorActive(float3 playerPosition) {
	if (m_onGround) {
		if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
			stopMovement();
			changeState(PASSIVE);
			return;
		}
		if (m_availablePath.empty()) {
			float3 target = circulateAround(playerPosition);
			makeReadyForPath(target);
		}

		lookTo(playerPosition);
		m_speed = m_active_speed;

		
	}
}

void Melon::behaviorCaught(float3 playerPosition) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_direction = playerPosition - m_position; // run to player

		m_speed = m_caught_speed;
		//makeReadyForPath(playerPosition);
		
	}
	lookTo(playerPosition);
}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

float3 Melon::circulateAround(float3 playerPosition) {

	float3 toMelon = m_position - playerPosition;
	toMelon.y = playerPosition.y;
	if (m_nrOfTriesGoHome == 100) {
		m_angleDirection *= -1;
		m_nrOfTriesGoHome = 0;
	}
	float angle = XM_PI / 8;
	angle *= m_angleDirection;
	Matrix rotate = Matrix(cos(angle), 0.f, -sin(angle), 0.f, 0.f, 1.f, 0.f, 0.f, sin(angle), 0.f,
		cos(angle), 0.f, 0.f, 0.f, 0.f, 1.f);


	float3 target = target.Transform(toMelon, rotate);
	target.Normalize();
	target *= 10.f;

	target += playerPosition;
	target.y = playerPosition.y;

	return target;
}

void Melon::updateAnimated(float dt) {
	m_frameTime += dt;
	if (m_frameTime > 2) {
		m_frameTime = 0.f;
		setAnimationDestination();
		lookTo(m_destinationAnimationPosition);
	}
	float3 tempDir(m_destinationAnimationPosition - getPosition());
	tempDir.Normalize();
	roll(dt);
}

void Melon::setRollSpeed(float rollSpeed) { m_rollAnimationSpeed = rollSpeed; }

void Melon::pathfinding(float3 start, std::vector<float4>* animals) {
	// ErrorLogger::log("thread starting for pathfinding");
	auto pft = PathFindingThread::getInstance();

	if ((start - m_destination).LengthSquared() < 0.5f)
		return;
	if (m_readyForPath) {
		
			TerrainManager* tm = TerrainManager::getInstance();
			// enforce start and m_destination to terrain
			float3 startCopy = float3(start.x, tm->getHeightFromPosition(start), start.z);
			float3 m_destinationCopy =
				float3(m_destination.x, tm->getHeightFromPosition(m_destination), m_destination.z);

			shared_ptr<AI::Node> currentNode = make_shared<AI::Node>(
				shared_ptr<AI::Node>(), startCopy, startCopy, m_destinationCopy);
			bool collidedWithSomething = false;
			size_t counter = 0;
			std::vector<shared_ptr<AI::Node>> open;
			std::vector<shared_ptr<AI::Node>> closed;
			std::list<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f),
				float3(0.f, 0.f, -1.f), float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f),
				float3(1.f, 0.f, 0.f), float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f),
				float3(1.f, 0.f, 1.f) };


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

					shared_ptr<AI::Node> child = make_shared<AI::Node>(
						currentNode, childPosition, startCopy, m_destinationCopy);


					// Check if node is in open or closed.
					if (!beingUsed(child, open, closed)) {
						continue;
					}
					if (!checkAnimals(*animals, childPosition)) {
						continue;
					}

					if (!isValid(
							child->position, currentNode->position, *pft->m_collidables, 0.7f)) {
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
