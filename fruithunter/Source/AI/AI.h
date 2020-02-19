#pragma once
#include "GlobalNamespaces.h"
#include "TerrainManager.h"
#include "Player.h"
#include <list>


class AI {
public:
	struct Node {
		float f, g, h;
		float3 position;
		Node() {
			position = float3{ 0.0f, 0.0f, 0.0f };
			f = g = h = 0.0f;
		};
		Node(float3 pos, float3 start, float3 goal) {
			position = pos;
			g = float3(start - pos).Length();
			h = float3(goal - pos).Length();
			f = g + h;
		};
		bool operator==(const Node& other) const { return this->position == other.position; }
		bool operator>(const Node& other) const { return this->f > other.f; }
	};
	enum State { INACTIVE, PASSIVE, ACTIVE, CAUGHT };
	void setWorld(std::shared_ptr<Terrain> terrain);
	void pathfinding(float3 start, float3 end, vector<shared_ptr<Entity>> collidables);
	void changeState(State newState);
	State getState() const;

protected:
	float m_passiveRadius, m_activationRadius;


	State m_currentState;
	std::shared_ptr<Terrain> m_terrain;
	std::list<float3> m_availablePath;
	float3 m_direction;
	virtual void behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables){};
	virtual void behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables){};
	virtual void behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables){};
	void doBehavior(float3 playerPosition, vector<shared_ptr<Entity>> collidables);
};
