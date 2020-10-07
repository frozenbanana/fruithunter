#include "AI.h"
#include <algorithm>
#include "PathFindingThread.h"
#include "TerrainBatch.h"
#include "SceneManager.h"


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
			m_availablePath.pop_back();
		}
	}
}

bool AI::beingUsed(shared_ptr<AI::Node> child, std::vector<shared_ptr<AI::Node>>& openList,
	std::vector<shared_ptr<AI::Node>>& closedList) {

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

bool AI::isValid(
	float3 childPos, float3 currentNodePos, float radius) {

	


	if (abs(childPos.y - currentNodePos.y) > MAX_STEAPNESS) {
		return false;
	}
	if (childPos.y < 1.5f) {
		return false;
	}

	auto normal = SceneManager::getScene()->m_terrains.getNormalFromPosition(childPos);
	normal.Normalize();
	// Don't you climb no walls
	if (abs(float3(0.0f, 1.0f, 0.0f).Dot(normal)) < 0.87f)
		return false;

	vector<shared_ptr<Entity>*> objects = SceneManager::getScene()->m_entities.getElementsByPosition(childPos);
	for (size_t i = 0; i < objects.size(); ++i) {
		if (!(*objects[i])->getIsCollidable())
			continue;

		float3 newPoint = (*objects[i])->getPosition() - childPos;
		newPoint.Normalize();
		newPoint.y = (*objects[i])->getPosition().y;
		newPoint *= radius;
		newPoint += childPos;

		if ((*objects[i])->checkCollision(newPoint))
			return false;
	}


	return true;
}

bool AI::checkAnimals(std::vector<float4> animals, float3 childPos) { 
	childPos.y = 0.f;
	for (auto e : animals) {
		float3 animalPos;
		animalPos.x = e.x;
		animalPos.y = 0.f;
		animalPos.z = e.z;
		float len = (childPos - animalPos).LengthSquared();
		if ( len < e.w*e.w*1.5f) {
			return false;
		}
	}	
	return true; 
}

void AI::makeReadyForPath(float3 destination) {
	m_readyForPath = true;
	m_destination = destination;
}

void AI::pathfinding(float3 start, std::vector<float4> *animals) {
	// ErrorLogger::log("thread starting for pathfinding");
	auto pft = PathFindingThread::getInstance();
	
	if ((start - m_destination).LengthSquared() < 0.5f)
		return;
	if (m_readyForPath) {
		m_availablePath.clear();

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

void AI::changeState(State newState) {
	m_currentState = newState;
	m_availablePath.clear();
}

AI::State AI::getState() const { return m_currentState; }

bool AI::giveNewPath() const { return m_readyForPath; }

void AI::doBehavior() {
	auto pft = PathFindingThread::getInstance();

	// pft->m_mutex.lock();
	switch (m_currentState) {
	case INACTIVE:
		behaviorInactive();
		break;
	case PASSIVE:
		behaviorPassive();
		break;
	case ACTIVE:
		behaviorActive();
		break;
	case CAUGHT:
		behaviorCaught();
		break;
	case RELEASED:
		behaviorReleased();
		break;
	}
	// pft->m_mutex.unlock();
}
