#pragma once
#include "GlobalNamespaces.h"
#include "Terrain.h"
#include "EntityRepository.h"
#include <list>
#include <mutex>
#define ARRIVAL_RADIUS 3.0f
#define MAX_STEAPNESS .1f

class AI {
public:
	struct Node {
		shared_ptr<Node> parent;
		float f, g, h;
		float3 position;
		Node() {
			parent;
			position = float3{ 0.0f, 0.0f, 0.0f };
			f = g = h = 0.0f;
		};
		Node(shared_ptr<Node> par, float3 pos, float3 start, float3 goal) {
			parent = par;
			position = pos;
			g = float3(start - pos).Length();
			h = float3(goal - pos).Length();
			f = g + h;
		};
		bool operator==(const Node& other) const { return this->position == other.position; }
		bool operator>(const Node& other) const { return this->f > other.f; }
		bool operator<(const Node& other) const { return this->f < other.f; }
		void operator=(const Node& other) {
			parent = other.parent;
			f = other.f;
			g = other.g;
			h = other.h;
			position = other.position;
		}
	};
	enum State { INACTIVE, PASSIVE, ACTIVE, CAUGHT, RELEASED };
	virtual void pathfinding(float3 start, std::vector<float4> *animals);
	void changeState(State newState);
	State getState() const;
	bool giveNewPath() const;
	void setWorked();
protected:
	float m_passiveRadius, m_activeRadius;
	bool m_beingWorked = false;
	float3 m_destination;
	// mutex m_mutex;

	bool m_readyForPath = false;
	bool m_beingWorked = false;
	State m_currentState;
	std::list<float3> m_availablePath;
	virtual void behaviorInactive(float3 playerPosition){};
	virtual void behaviorPassive(float3 playerPosition){};
	virtual void behaviorActive(float3 playerPosition){};
	virtual void behaviorCaught(float3 playerPosition){};
	virtual void behaviorReleased(){};

	void doBehavior(float3 playerPosition);
	void quickSort(std::vector<shared_ptr<AI::Node>>& unsortedVector, int low, int high);
	int partition(std::vector<shared_ptr<AI::Node>>& unsortedVector, int low, int high);

	void handleAvailablePath(float3 myPosition);

	bool isValid(
		float3 childPos, float3 currentNodePos, EntityRepository& collidables, float radius);
	bool isValid(float3 childPos, float3 currentNodePos);
	bool checkAnimals(std::vector<float4> animals, float3 childPos);
	void makeReadyForPath(float3 destination);

private:
	bool beingUsed(shared_ptr<AI::Node> child, std::vector<shared_ptr<AI::Node>>& openList,
		std::vector<shared_ptr<AI::Node>>& closedList);
};
