#include "Melon.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 1;
	m_meshAnim.setFrameTargets(0, 0);
	setScale(0.5);
	changeState(AI::State::PASSIVE);
	m_direction = float3((float)(rand() % 1), 0.0f, (float)(rand() % 1));
	m_velocity = float3(1.f);
	m_direction.Normalize();
}

void Melon::updateAnimated(float dt) {}

// TODO, deside is we are going to use m_direction or m_velocity or both for movement.
void Melon::move(float dt) {
	if (!m_availablePath.empty() && m_currentState == ACTIVE) {
		m_direction = (m_availablePath.back() - m_position);
		m_availablePath.pop_back();
	}

	m_direction.y = 0.f;
	m_position += m_direction * dt;
	setPosition(m_position);
}


void Melon::update(float dt, float3 playerPosition, Terrain* terrain) {
	float distanceToPlayer = (playerPosition - m_position).Length();

	if (distanceToPlayer < 5.f && m_currentState == PASSIVE) {
		circulateAround(playerPosition); // updates path
		changeState(AI::ACTIVE);
	}
	else if (m_availablePath.empty()) { // is path finished set pasive state
		AI::changeState(AI::PASSIVE);
		m_direction = float3(sin(dt), 0.0f, (float)(rand() % 5));
		m_direction.Normalize();
	}
	updateAnimated(dt);
	m_position.y = terrain->getHeightFromPosition(m_position);
	move(dt);
}

void Melon::circulateAround(float3 playerPosition) {

	float3 playerToMelon = m_position - playerPosition;
	float angle = 3.1416 * 0.25; // 8th of halfcircle

	// rotate to find target
	float3 target =
		playerPosition + float3(playerToMelon.x * cos(angle) - playerToMelon.x * sin(angle), 0.0f,
							 playerToMelon.z * sin(angle) + playerToMelon.z * cos(angle));

	ErrorLogger::log("inside circulateAround, target: (" + std::to_string(target.x) + " , " +
					 std::to_string(target.y) + " , " + std::to_string(target.z) + ")");
	pathfinding(m_position, playerPosition);
}