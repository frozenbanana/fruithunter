#include "AI.h"
#include <algorithm>

void AI::setWorld(std::shared_ptr<Terrain> terrain) { m_terrain = terrain; }

void AI::pathfinding(float3 start, float3 end) {
	AI::Node currentNode;
	std::vector<AI::Node> open, closed;
	std::vector<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f), float3(0.f, 0.f, -1.f),
		float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f), float3(1.f, 0.f, 0.f),
		float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f), float3(1.f, 0.f, 1.f) };

	open.push_back(AI::Node(start, start, end));

	while (!open.empty()) {
		std::sort(open.begin(), open.end(),
			[](const AI::Node& n1, const AI::Node& n2) -> bool { return n1 > n2; });

		closed.push_back(open.front());
		open.pop_back();

		// Check if path is complete
		if (closed.back().position == end) {
			m_availablePath.clear(); // Reset path
			// Add path steps
			while (!closed.empty()) {
				m_availablePath.push_back(closed.back().position);
				closed.pop_back();
			}
			ErrorLogger::log(std::to_string(m_availablePath.front().x));
			return;
		}

		// Set current AI::Node
		currentNode = closed.back();

		// Generate children
		std::vector<AI::Node> childrenList;
		for (auto childOffset : childPositionOffsets) {
			// TODO: add check is possible to create child
			// E.g object detection from grid

			// Create child AI::Node
			float3 childPosition = currentNode.position + childOffset;
			AI::Node child = AI::Node(childPosition, start, end);

			// Check is child is in closed
			if (std::find(closed.begin(), closed.end(), child) == closed.end()) {
				continue;
			}

			// Check is child is in open
			if (std::find(open.begin(), open.end(), child) == open.end()) {
				continue;
			}

			// Add child to open
			open.push_back(child);
		}
	}
}

void AI::changeState(State newState) { m_currentState = newState; }
