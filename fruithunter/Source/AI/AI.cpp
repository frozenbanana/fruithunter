#include "AI.h"
#include <algorithm>
#define STEP_SCALE 1.

void AI::setWorld(std::shared_ptr<Terrain> terrain) { m_terrain = terrain; }

void AI::pathfinding(float3 start, float3 end) {
	start.y = 0.f;
	end.y = 0.f;
	ErrorLogger::log("Inside pathfinding");
	AI::Node currentNode;
	std::vector<AI::Node> open, closed;
	std::vector<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f), float3(0.f, 0.f, -1.f),
		float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f), float3(1.f, 0.f, 0.f),
		float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f), float3(1.f, 0.f, 1.f) };

	open.push_back(AI::Node(start, start, end));

	while (!open.empty()) {
		std::sort(open.begin(), open.end(),
			[](const AI::Node& n1, const AI::Node& n2) -> bool { return n1.f > n2.f; });

		closed.push_back(open.back());
		open.pop_back();

		// Check if path is complete
		ErrorLogger::log("closed.back().position:  (" + std::to_string(closed.back().position.x) +
						 " , " + std::to_string(closed.back().position.y) + " , " +
						 std::to_string(closed.back().position.z) + "), end: (" +
						 std::to_string(end.x) + " , " + std::to_string(end.y) + " , " +
						 std::to_string(end.z) + "), " +
						 std::to_string((closed.back().position - end).Length()));
		// ErrorLogger::log("open[0-3]:  (" +   std::to_string(open[0].x) + " , " +
		//									 std::to_string(open[0].y) + " , " +
		//									 std::to_string(open[0].z) + "), , (" +
		//									 std::to_string(open[1].x) + " , " +
		//									 std::to_string(open[1].y) + " , " +
		//									 std::to_string(open[1].z) + " (" +
		//									 std::to_string(open[2].x) + " , " +
		//				                     std::to_string(open[2].y) + " , " +
		// std::to_string(open[2].z)
		//+));

		if ((closed.back().position - end).Length() < 1.4f) {
			m_availablePath.clear(); // Reset path
			// Add path steps
			while (!closed.empty()) {
				m_availablePath.push_back(closed.back().position);
				closed.pop_back();
			}
			ErrorLogger::log(
				"Path found! Amout of steps: " + std::to_string(m_availablePath.size()));

			/*ErrorLogger::log(std::to_string(m_availablePath.front().x));*/
			return;
		}

		// Set current AI::Node
		currentNode = closed.back();

		// Generate children
		// ErrorLogger::log("Generating children");

		// std::vector<AI::Node> childrenList;
		for (auto childOffset : childPositionOffsets) {
			// TODO: add check is possible to create child
			// E.g object detection from grid

			// Create child AI::Node
			float3 childPosition = currentNode.position + STEP_SCALE * childOffset;
			AI::Node child = AI::Node(childPosition, start, end);
			// Check is child is in closed
			if (std::find(closed.begin(), closed.end(), child) != closed.end()) {
				continue;
			}

			// Check is child is in open
			if (std::find(open.begin(), open.end(), child) != open.end()) {
				continue;
			}

			// Add child to open
			open.push_back(child);
		}
	}
	ErrorLogger::log("Exiting pathfinding");
}

void AI::changeState(State newState) { m_currentState = newState; }

AI::State AI::getState() const { return m_currentState; }

void AI::doBehavior(float3 playerPosition) {
	switch (m_currentState) {
	case PASSIVE:
		behaviorPassive(playerPosition);
		break;
	case ACTIVE:
		behaviorActive(playerPosition);
		break;
	case CAUGHT:
		behaviorCaught(playerPosition);
		break;
	}
}
