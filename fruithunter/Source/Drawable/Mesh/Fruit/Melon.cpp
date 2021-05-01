#include "Melon.h"
#include "PathFindingThread.h"
#include "SceneManager.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;

	m_meshAnim.setFrameTargets(0, 0);
	m_fruitType = MELON;

	setScale(m_startScale);
	changeState(AI::State::ACTIVE);
	setStartPosition(pos);

	setCollisionDataOBB();

	m_ball.load("Sphere");
	m_ball.setScale(0.1f);

	m_rollTrail.setScale(float3(1.f, 0.1f, 1.f));
	m_rollTrail.setType(ParticleSystem::Type::MELON_TRAIL, false);
	m_rollTrail.setEmitRate(200, true);
}

void Melon::behaviorPassive() { 
	// -- FRUIT RESPAWNING -- // 
	// Change into passive mode to automaticly respawn melon

	if (!isRespawning()) {
		// start of respawn
		// init
		m_respawn_timer = m_respawn_max;
	}
	else {
		// update
		float dt = SceneManager::getScene()->getDeltaTime();

		float th = m_respawn_max / 2;
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
					tIndex = rand() % SceneManager::getScene()->m_terrains.length();
				}
				if (tIndex != -1) {
					float3 sp = SceneManager::getScene()->m_terrains.getSpawnpoint(tIndex);
					setPosition(sp + float3(0.f, 1.f, 0.f) * (getHalfSizes().y + 0.1f));
				}
				else {
					// this should never happen as fruits only can spawn if there is a terrain to
					// spawn from
					ErrorLogger::logError(
						"(Melon) Melon cant respawn. No terrains exists!", HRESULT());
				}
			}
			m_velocity *= 0;
		}
		m_respawn_timer = Clamp<float>(m_respawn_timer-dt, 0, m_respawn_max);

		// scaling
		float factor = abs((m_respawn_max / 2) - m_respawn_timer) / (m_respawn_max / 2);
		setScale(m_startScale * factor);

		if (m_respawn_timer == 0) {
			// end of respawn
			// switch to active mode
			changeState(ACTIVE);
		}
	}
}

void Melon::behaviorActive() {
	float dt = SceneManager::getScene()->getDeltaTime();
	TerrainBatch* tr = &SceneManager::getScene()->m_terrains;

	// init velocity
	if (float2(m_velocity.x, m_velocity.z).Length() == 0) {
		float r = RandomFloat(0, 1) * 2 * 3.1415f;
		m_velocity += float3(cos(r), 0.f, sin(r)) * 0.1f;
	}

	// animation
	float3 lookAtVel = vector2Rotation(m_velocity);
	float3 rot = getRotation();
	rot.y = lookAtVel.y;
	float radie = getHalfSizes().y;
	float rotSpeed = m_velocity.Length() / radie;
	rot.x += rotSpeed * dt;
	setRotation(rot);

	// sensors
	float3 sensorAvg;
	size_t counter = 0;
	float3 feet = getPosition() - getHalfSizes();
	for (size_t i = 0; i < 8 + (int)m_avoidPlayer; i++) {
		if (i < 8) {
			float rot = (3.1415f * 2.f / 8.f) * (float)i;
			m_sensors[i] = float3(cos(rot), 0, sin(rot)) *
							   (m_velocity.Length() * !m_fixedSensors + m_fixedSensors) *
							   m_sensorWidthScale +
						   getPosition();
			float height = tr->getHeightFromPosition(m_sensors[i]);
			m_sensorState[i] = (abs(feet.y - height) > m_sensorHeightTrigger) || (height < 0.5);
		}
		else {
			float3 playerPos = SceneManager::getScene()->m_player->getPosition();
			float3 toPlayer = (playerPos - getPosition()) * float3(1, 0, 1);
			// player sensor
			m_sensors[i] = Normalize(toPlayer) *
							   (m_velocity.Length() * !m_fixedSensors + m_fixedSensors) *
							   m_sensorWidthScale +
						   getPosition();
			m_sensorState[i] = (toPlayer.Length() < (m_sensors[i] - getPosition()).Length());
		}
		if (m_sensorState[i]) {
			sensorAvg += m_sensors[i];
			counter++;
		}
	}
	if (counter)
		sensorAvg /= (float)counter;

	if (!m_onGround) {
		// do nothing
		// fall to ground
	}
	else {
		float3 normal = tr->getNormalFromPosition(getPosition());
		float3 direction = float3(1, 0, 0);
		if (m_velocity.Length() != 0)
			direction = Normalize(m_velocity);

		if (counter == 0) {
			m_accumulatedTime += dt;
			// move straight forward (add a bit randomness)
			m_velocity += rotatef2Y(direction, cos(m_accumulatedTime * m_forwardAngleSpeed) *
												   m_varyingForwardAngle) *
						  (m_topSpeed - m_velocity.Length()) * (1 - pow(m_acceleration, dt));
		}
		else if (counter >= 8) {
			m_velocity += direction * (0 - m_velocity.Length()) * (1 - pow(m_acceleration, dt));
		}
		else {
			float3 desired = -Normalize(sensorAvg - getPosition()) * m_topSpeed;
			m_velocity += (desired - m_velocity) * (1 - pow(m_acceleration, dt));
		}
	}
}

void Melon::behaviorCaught() {
	/* NOT USED */
}

void Melon::updateAnimated(float dt) {
	// static mesh (no animation)
}

void Melon::pathfinding(float3 start) {
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

bool Melon::isRespawning() const { return m_respawn_timer != 0; }

void Melon::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	m_rollTrail.update(dt);
	m_rollTrail.emitingState(m_onGround); // emit if on ground

	m_isVisible = true;
	m_particleSystem.setPosition(getPosition());
	// updateAnimated(dt); // animation stuff
	// checkOnGroundStatus(); // checks if on ground

	doBehavior();

	// update velocity
	m_velocity += (float3(0, -1, 0) * m_gravityStrength) * dt; // gravity
	m_velocity *= pow(1.0f, dt);								   // friction
	// collision
	float3 point = getPosition() - float3(0, 1, 0) * getHalfSizes().y;
	float3 forward = m_velocity * dt;
	size_t iterations = 5;
	float3 velCopy = m_velocity;
	while (true) {
		forward = m_velocity * dt;
		float3 intersection, normal;
		if (rayCastWorld(point, forward, intersection, normal)) {
			float3 longVel = forward; // full velocity this frame (called long velocity)
			float3 shortVel =
				intersection - point; // velocity until collision (called short velocity)
			m_velocity =
				(longVel - (longVel.Dot(normal) - shortVel.Dot(normal) - 0.001f) * normal) / dt;
		}
		else
			break;

		iterations--;
		if (iterations <= 0) {
			// stuck
			m_velocity = -velCopy * 0.5f; // bounce
			break;
		}
	}
	// movement
	setPosition(getPosition() + m_velocity * dt);

	// place self over terrain if fall under
	float3 pos = getPosition() - float3(0, 1, 0) * getHalfSizes().y;
	float tHeight = SceneManager::getScene()->m_terrains.getHeightFromPosition(pos);
	if (pos.y < tHeight) {
		pos.y = tHeight + getHalfSizes().y + 0.1f;
		setPosition(pos);
	}

	// check if on ground
	m_onGround = isOnGround(point, m_aboveGroundThreshold);

	// respawn if fall into water
	if (getPosition().y < 1 && getState() == State::ACTIVE) {
		changeState(State::PASSIVE);
	}
}

void Melon::update_imgui_changeParams() {
	if (ImGui::Begin("TestingWindow")) {
		ImGui::Checkbox("Show Sensors", &m_showSensors);
		ImGui::InputFloat("speed", &m_topSpeed);
		ImGui::SliderFloat("acc", &m_acceleration, 0, 1);
		ImGui::InputFloat("heightTrigger", &m_sensorHeightTrigger);
		ImGui::InputFloat("sensor width", &m_sensorWidthScale);
		ImGui::Checkbox("fixed sensors", &m_fixedSensors);
		ImGui::InputFloat("varying forward angle", &m_varyingForwardAngle);
		ImGui::InputFloat("varying forward angle speed", &m_forwardAngleSpeed);
		ImGui::Text("forwardAngle: %f", cos(m_accumulatedTime * m_forwardAngleSpeed));
		ImGui::InputFloat("ground threshold", &m_aboveGroundThreshold);
		ImGui::InputFloat("gravity strength", (float*)&m_gravityStrength);
		ImGui::End();
	}
}

void Melon::draw_rollTrail() { 
	m_rollTrail.setPosition(getPosition() - float3(0, 1, 0) * getHalfSizes().y);
	m_rollTrail.draw();
}

void Melon::draw_sensors() {
	if (m_showSensors) {
		float3 badColor(1, 0, 0), goodColor(1, 1, 1);
		for (size_t i = 0; i < 8 + (int)m_avoidPlayer; i++) {
			m_ball.setPosition(m_sensors[i]);
			m_ball.draw_onlyMesh(m_sensorState[i] ? badColor : goodColor);
		}
	}
}

void Melon::draw_fruit() {
	Fruit::draw_fruit();
	if (m_isVisible) {
		draw_sensors();
		draw_rollTrail();
	}
}
