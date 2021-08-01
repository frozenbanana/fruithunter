#include "Melon.h"
#include "SceneManager.h"

Melon::Melon(float3 pos) : Fruit(FruitType::MELON, pos) {
	loadAnimated("Melon", 1);
	setCollisionDataOBB();

	setScale(m_startScale);


	m_ball.load("Sphere");
	m_ball.setScale(0.1f);

	m_rollTrail.setScale(float3(1.f, 0.1f, 1.f));
	m_rollTrail.setDesc("melon trail");
	m_rollTrail.setEmitRate(200);
}

void Melon::updateAnimated(float dt) {
	// static mesh (no animation)
}

void Melon::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	if (m_showSensors)
		update_imgui_changeParams();

	m_rollTrail.update(dt);
	m_rollTrail.setEmitingState(m_onGround); // emit if on ground

	m_isVisible = true;
	// updateAnimated(dt); // animation stuff
	// checkOnGroundStatus(); // checks if on ground

	behavior();

	// update velocity
	m_velocity += (float3(0, -1, 0) * m_gravityStrength) * dt; // gravity
	m_velocity *= pow(1.0f, dt);							   // friction
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
		else {
			break;
		}

		iterations--;
		if (iterations == 0) {
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
	updateRespawn();
	if (getPosition().y < 1) {
		respawn();
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
	}
	ImGui::End();
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

void Melon::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }

void Melon::behavior() {
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
