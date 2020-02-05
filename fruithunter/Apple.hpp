#pragma once
#include "Entity.h"
class Apple : public Entity {
private:
	clock_t m_clock;
	float m_frameTime;
	int m_nrOfFramePhases;
	int m_currentFramePhase;


public:
	Apple();
	void updateAnimated();
};
