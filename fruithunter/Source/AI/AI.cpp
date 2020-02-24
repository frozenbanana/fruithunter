#include "AI.h"
#include <algorithm>
#include "Fruit.h"
#define STEP_SCALE 1.0f
#define MAX_STEAPNESS .2f
#define EPSILON 0.001f
#define MAX_STEPS 70

bool areSame(float3 a, float3 b) { return (a - b).LengthSquared() < EPSILON; }

bool isIn(shared_ptr<AI::Node> target, std::vector<shared_ptr<AI::Node>> vector) {
	for (size_t i = 0; i < vector.size(); i++) {
		if (areSame(vector[i]->position, target->position)) {
			return true;
		}
	}
	return false;
}

void AI::quickSort(std::vector<shared_ptr<AI::Node>>& unsortedVector, int low, int high) {
	if (low < high) {
		int partitionIndex = partition(unsortedVector, low, high);
		quickSort(unsortedVector, low, partitionIndex - 1);	 // Before pi
		quickSort(unsortedVector, partitionIndex + 1, high); // After pi
	}
	// It is now sorted
}

int AI::partition(std::vector<shared_ptr<AI::Node>>& unsortedVector, int low, int high) {
	// pivot (Element to be placed at right position)
	AI::Node* pivotElement = unsortedVector[high].get();

	int i = (low - 1); // Index of smaller element
	for (int j = low; j <= high - 1; j++) {
		// If current element is greater than the pivot
		if (*unsortedVector[j] > *pivotElement) {
			i++; // increment index of smaller element
			std::swap(unsortedVector[i], unsortedVector[j]);
		}
	}
	std::swap(unsortedVector[i + 1], unsortedVector[high]);
	return (i + 1);
}

void AI::handleAvailablePath(float3 myPosition) {
	if (!m_availablePath.empty()) {
		float3 positionXZ = float3(myPosition.x, 0.0f, myPosition.z);
		float3 currentTargetXZ = float3(m_availablePath.back().x, 0.0f, m_availablePath.back().z);

		// Update next path point
		if ((positionXZ - currentTargetXZ).Length() < ARRIVAL_RADIUS) {
			ErrorLogger::logFloat3("Popping point: ", m_availablePath.back());
			m_availablePath.pop_back();
		}
	}
}

bool AI::beingUsed(shared_ptr<AI::Node> child, std::vector<shared_ptr<AI::Node>>& openList,
	std::vector<shared_ptr<AI::Node>>& closedList) {
	// return isIn(child, closedList) && isIn(child, openList);


	// Previous check
	//// Check is child is in closed
	if (isIn(child, closedList)) {
		return false;
	}

	// Check is child is in open
	if (isIn(child, openList)) {
		return false;
	}
	return true;
}

bool AI::isValid(float3 childPos, float3 currentNodePos, vector<shared_ptr<Entity>>& collidables) {
	if (childPos.y - currentNodePos.y > MAX_STEAPNESS) {
		return false;
	}


	for (size_t i = 0; i < collidables.size(); ++i) {
		float3 obstacle = collidables.at(i)->getPosition();
		obstacle.y = 0.f;
		childPos.y = 0.f;
		float lengthChildToCollidableSquared = (childPos - obstacle).LengthSquared();
		float collidableRadiusSquared = collidables.at(i)->getHalfSizes().LengthSquared();

		if (lengthChildToCollidableSquared < collidableRadiusSquared) {

			return false;
		}
	}

	return true;


	// previous check
	// Check for too big height difference
	// if (childPosition.y - currentNode->position.y > MAX_STEAPNESS) {
	//	continue;
	//}

	//// check if collision with objects
	// for (size_t i = 0; i < collidables.size(); ++i) {
	//	float3 obstacle = collidables.at(i)->getPosition();
	//	obstacle.y = 0.f;
	//	childPosition.y = 0.f;
	//	float lengthChildToCollidableSquared = (childPosition - obstacle).LengthSquared();
	//	float collidableRadiusSquared = collidables.at(i)->getHalfSizes().LengthSquared();

	//	if (lengthChildToCollidableSquared < collidableRadiusSquared) {
	//		collidedWithSomething = true;
	//		break;
	//	}
	//}


	/*if (collidedWithSomething) {
		collidedWithSomething = false;
		continue;
	}*/
}


void AI::setWorld(std::shared_ptr<Terrain> terrain) { m_terrain = terrain; }

void AI::pathfinding(float3 start, float3 end, vector<shared_ptr<Entity>> collidables) {
	m_availablePath.clear();
	TerrainManager* tm = TerrainManager::getInstance();
	// enforce start and end to terrain
	start.y = tm->getHeightFromPosition(start);
	end.y = tm->getHeightFromPosition(end);

	shared_ptr<AI::Node> currentNode =
		make_shared<AI::Node>(shared_ptr<AI::Node>(), start, start, end);
	bool collidedWithSomething = false;
	size_t counter = 0;
	std::vector<shared_ptr<AI::Node>> open;
	std::vector<shared_ptr<AI::Node>> closed;
	std::list<float3> childPositionOffsets = { float3(-1.f, 0.f, -1.f), float3(0.f, 0.f, -1.f),
		float3(1.f, 0.f, -1.f), float3(-1.f, 0.f, 0.f), float3(1.f, 0.f, 0.f),
		float3(-1.f, 0.f, 1.f), float3(0.f, 0.f, 1.f), float3(1.f, 0.f, 1.f) };



	open.push_back(currentNode);
	while (!open.empty() && counter++ < MAX_STEPS) {
		quickSort(open, 0, (int)open.size() - 1);
		ErrorLogger::log(to_string(counter));
		closed.push_back(open.back());
		open.pop_back();

		// Check to see if we're inside a certain radius of end location
		shared_ptr<AI::Node> currentNode = closed.back();
		if ((currentNode->position - end).LengthSquared() < ARRIVAL_RADIUS) {
			m_availablePath.clear(); // Reset path

			// Add path steps
			while (currentNode->parent != nullptr) {
				m_availablePath.push_back(currentNode->position);
				currentNode = currentNode->parent;
			}

			if (!m_availablePath.empty()) {
				m_availablePath
					.pop_back(); // remove first position because it is the same as start.
			}
			for (auto p : m_availablePath) {
				ErrorLogger::logFloat3("Target: ", p);
			}

			return;
		}

		for (auto childOffset : childPositionOffsets) {
			// Create child AI::Node
			float3 childPosition = currentNode->position + STEP_SCALE * childOffset;
			childPosition.y = tm->getHeightFromPosition(childPosition);

			shared_ptr<AI::Node> child =
				make_shared<AI::Node>(currentNode, childPosition, start, end);


			// Check if node is in open or closed.
			if (!beingUsed(child, open, closed)) {
				continue;
			}

			if (!isValid(child->position, currentNode->position, collidables)) {
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
}

void AI::changeState(State newState) {
	m_availablePath.clear();
	m_currentState = newState;
}

AI::State AI::getState() const { return m_currentState; }

void AI::doBehavior(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	switch (m_currentState) {
	case INACTIVE:
		behaviorInactive(playerPosition);
		break;
	case PASSIVE:
		behaviorPassive(playerPosition, collidables);
		break;
	case ACTIVE:
		behaviorActive(playerPosition, collidables);
		break;
	case CAUGHT:
		behaviorCaught(playerPosition, collidables);
		break;
	case RELEASED:
		behaviorReleased();
		break;
	}
}
