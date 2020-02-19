#include "AI.h"
#include <algorithm>
#include "Fruit.h"
#define STEP_SCALE 1.f
#define MAX_STEAPNESS 5.f

void AI::setWorld(std::shared_ptr<Terrain> terrain) { m_terrain = terrain; }

void AI::pathfinding(float3 start, float3 end, vector<shared_ptr<Entity>> collidables) {
	TerrainManager* tm = TerrainManager::getInstance();
	ErrorLogger::log("Inside pathfinding");
	AI::Node currentNode;
	std::vector<AI::Node> open;
	std::list<float3> closed;
	std::list<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f), float3(0.f, 0.f, -1.f),
		float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f), float3(1.f, 0.f, 0.f),
		float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f), float3(1.f, 0.f, 1.f) };

	open.push_back(AI::Node(start, start, end));

	while (!open.empty()) {
		std::sort(open.begin(), open.end(),
			[](const AI::Node& n1, const AI::Node& n2) -> bool { return n1.f > n2.f; });

		closed.push_back(open.back().position);
		open.pop_back();

		// Check to see if we're inside a certain radius of end location
		if ((closed.back() - end).Length() < 0.75f) {
			m_availablePath.clear(); // Reset path
			// Add path steps
			if (!closed.empty()) {
				m_availablePath = closed;
			}
			ErrorLogger::log(
				"Path found! Amout of steps: " + std::to_string(m_availablePath.size()));
			int counter = 0;
			for (float3 p : m_availablePath) {
				ErrorLogger::logFloat3("step " + to_string(counter++), p);
			}
			return;
		}

		// Set current AI::Node
		currentNode = AI::Node(closed.back(), start, end);

		// std::vector<AI::Node> childrenList;
		for (auto childOffset : childPositionOffsets) {
			// Create child AI::Node
			float3 childPosition = currentNode.position + STEP_SCALE * childOffset;
			childPosition.y = tm->getHeightFromPosition(childPosition);
			if (childPosition.y - currentNode.position.y > MAX_STEAPNESS) {
				continue;
			}

			AI::Node child = AI::Node(childPosition, start, end);
			// Check is child is in closed
			if (std::find(closed.begin(), closed.end(), child.position) != closed.end()) {
				continue;
			}

			// Check is child is in open
			if (std::find(open.begin(), open.end(), child) != open.end()) {
				continue;
			}

			// check if collision with objects
			for (size_t i = 0; i < collidables.size(); ++i) {
				float lengthChildToCollidableCenter =
					(childPosition - collidables.at(i)->getPosition()).Length();
				float collidableRadius = collidables.at(i)->getHalfSizes().Length();
				// ErrorLogger::log("CollisionRadius: " + std::to_string(collidableRadius));
				if (lengthChildToCollidableCenter < collidableRadius * 5.f) {
					// ErrorLogger::logFloat3("Failed child::", childPosition);
					// ErrorLogger::logFloat3("        with::", collidables.at(i)->getPosition());
					break;
					continue;
				}
			}

			// Add child to open
			// ErrorLogger::logFloat3("Approved child", child.position);
			open.push_back(child);
		}
	}
	ErrorLogger::log("Exiting pathfinding");
}

void AI::changeState(State newState) { m_currentState = newState; }

AI::State AI::getState() const { return m_currentState; }

void AI::doBehavior(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	switch (m_currentState) {
	case PASSIVE:
		behaviorPassive(playerPosition, collidables);
		break;
	case ACTIVE:
		behaviorActive(playerPosition, collidables);
		break;
	case CAUGHT:
		behaviorCaught(playerPosition, collidables);
		break;
	}
}
