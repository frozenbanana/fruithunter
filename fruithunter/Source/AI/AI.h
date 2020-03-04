#pragma once
#include "GlobalNamespaces.h"
#include "TerrainManager.h"
#include "Player.h"
#include <list>
#include <mutex>
#define ARRIVAL_RADIUS 3.0f
#define MAX_STEAPNESS .2f

// int runningThreads = 0;


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
	void setWorld(std::shared_ptr<Terrain> terrain);
	void pathfinding(float3 start);
	void changeState(State newState);
	State getState() const;
	bool giveNewPath() const;

protected:
	float m_passiveRadius, m_activeRadius;

	float3 m_destination;
	// mutex m_mutex;

	bool m_readyForPath = false;
	State m_currentState;
	std::shared_ptr<Terrain> m_terrain;
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

	bool isValid(float3 childPos, float3 currentNodePos, vector<shared_ptr<Entity>> collidables);
	bool isValid(float3 childPos, float3 currentNodePos);
	void makeReadyForPath(float3 destination);

private:
	bool beingUsed(shared_ptr<AI::Node> child, std::vector<shared_ptr<AI::Node>>& openList,
		std::vector<shared_ptr<AI::Node>>& closedList);
};
