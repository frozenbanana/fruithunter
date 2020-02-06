#pragma once
#include "Entity.hpp"
class Apple : public Entity {
private:
	float m_frameTime;
	int m_nrOfFramePhases;
	int m_currentFramePhase;


public:
	Apple();
	void updateAnimated(float dt);
};
