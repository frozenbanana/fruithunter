#pragma once
#include "Entity.h"
class Apple : public Entity {
private:
	float m_frameTime;
	int m_nrOfFramePhases;
	int m_currentFramePhase;


public:
	Apple();
	void updateAnimated(float dt);
};
