#pragma once
#include "GlobalNamespaces.h"
#include "Terrain.h"
//#include"Player.h"

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
	// virtual float3 getNextPosition();
	void pathfinding(float3 start, float3 end);
	void changeState(State newState);
	State getState() const;
	// void idle();
	// void detectPlayer(float3 playerPosition);

protected:
	float m_passiveRadius, m_activationRadius;


	State m_currentState;
	std::shared_ptr<Terrain> m_terrain;
	std::vector<float3> m_availablePath;
	float3 m_direction;
	virtual void behaviorPassive(float3 playerPosition){};
	virtual void behaviorActive(float3 playerPosition){};
	virtual void behaviorCaught(float3 playerPosition){};
	void doBehavior(float3 playerPosition);
};
