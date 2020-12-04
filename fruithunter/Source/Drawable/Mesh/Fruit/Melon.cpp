#include "Melon.h"
#include "PathFindingThread.h"
#include "SceneManager.h"

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
	setWorldHome(getPosition());
	m_secondWorldHome = m_worldHome + float3(3.f, 0.0, 3.0f);
	m_secondWorldHome.y =
		SceneManager::getScene()->m_terrains.getHeightFromPosition(m_secondWorldHome);
	m_direction = getPosition() - m_secondWorldHome;

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

	m_ball.load("Sphere");
	m_ball.setScale(0.1);
}

void Melon::behaviorPassive() {
	if (ImGui::Begin("TestingWindow")) {
		ImGui::InputFloat("speed", &m_topSpeed);
		ImGui::InputFloat("acc", &m_acceleration);
		ImGui::InputFloat("back acc", &m_deacceleration);
		ImGui::InputFloat("heightTrigger", &m_sensorHeightTrigger);
		ImGui::End();
	}

	float dt = SceneManager::getScene()->getDeltaTime();
	TerrainBatch* tr = &SceneManager::getScene()->m_terrains;

	float3 sensorAvg;
	size_t counter = 0;
	float3 feet = getPosition() - getHalfSizes();
	for (size_t i = 0; i < 8; i++) {
		float rot = (3.1415 * 2 / 8) * i;
		m_sensors[i] = float3(cos(rot), 0, sin(rot)) * m_velocity.Length() + getPosition();
		float height = tr->getHeightFromPosition(m_sensors[i]);
		m_sensorState[i] = (abs(feet.y - height) > m_sensorHeightTrigger) || (height < 0.5);
		if (!m_sensorState[i]) {
			sensorAvg += m_sensors[i];
			counter++;
		}
	}
	if (counter)
		sensorAvg /= counter;

	if (!m_onGround) {
		// fall to ground
	}
	else {
		float3 normal = tr->getNormalFromPosition(getPosition());
		float3 direction = float3(m_topSpeed, 0, 0);
		if (m_velocity.Length() != 0)
			direction = Normalize(m_velocity);

		if (counter == 8) {
			m_velocity += direction * (m_topSpeed - m_velocity.Length()) * m_acceleration * dt;
		}
		else if (counter == 0) {
			m_velocity += direction * (0 - m_velocity.Length()) * m_deacceleration * dt;
		}
		else {
			float3 desired = Normalize(sensorAvg - getPosition()) * m_topSpeed;
			m_velocity += (desired - m_velocity) * m_acceleration * dt;
		}



		//float3 futurePointF = getPosition() + direction * m_velocity.Length();
		//float3 futurePointL =
		//	getPosition() + rotatef2Y(direction, sensorWidth) * m_velocity.Length();
		//float3 futurePointR =
		//	getPosition() + rotatef2Y(direction, -sensorWidth) * m_velocity.Length();
		//bool checkF = abs(getPosition().y-tr->getHeightFromPosition(futurePointF)) > 1;
		//bool checkL = abs(getPosition().y-tr->getHeightFromPosition(futurePointL)) > 1;
		//bool checkR = abs(getPosition().y-tr->getHeightFromPosition(futurePointR)) > 1;
		//bool increaseSpeed = true;
		//if (checkF) {
		//	if (checkR && checkL) {
		//		// decrease speed
		//		increaseSpeed = false;
		//	}
		//	else if (checkR) {
		//		// turn left
		//		m_velocity = rotatef2Y(m_velocity, sensorWidth * m_rotSpeed * dt);
		//	}
		//	else if(checkL) {
		//		// turn right
		//		m_velocity = rotatef2Y(m_velocity, -sensorWidth * m_rotSpeed * dt);
		//	}
		//	else {
		//		// turn left
		//		m_velocity = rotatef2Y(m_velocity, sensorWidth * m_rotSpeed * dt);
		//	}
		//}
		//m_velocity += direction * ((int)increaseSpeed * m_topSpeed - m_velocity.Length()) *
		//			  (m_acceleration * increaseSpeed+m_deacceleration*(!increaseSpeed)) * dt;
	}
}

void Melon::behaviorActive() {
	changeState(ACTIVE);
	return;

	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
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

void Melon::behaviorCaught() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	if (atOrUnder(SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()))) {
		m_direction = playerPosition - getPosition(); // run to player

		m_speed = m_caught_speed;
		//makeReadyForPath(playerPosition);
		
	}
	lookTo(playerPosition);
}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

float3 Melon::circulateAround(float3 playerPosition) {

	float3 toMelon = getPosition() - playerPosition;
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
			TerrainBatch* tm = &SceneManager::getScene()->m_terrains;
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

					if (!isValid(child->position, currentNode->position, 0.7f)) {
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

void Melon::draw_sensors() {
	float3 badColor(1, 0, 0), goodColor(1, 1, 1);
	for (size_t i = 0; i < 8; i++) {
		m_ball.setPosition(m_sensors[i]);
		m_ball.draw_onlyMesh(m_sensorState[i]?badColor:goodColor);
	}
}
