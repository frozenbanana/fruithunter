#pragma once
#include "GlobalNamespaces.h"

#define LEVELS = 1;

struct level {
	// HeightMap
	std::vector<std::string> m_heightMapNames;
	std::vector<float3> m_heightMapPos;
	std::vector<XMINT2> m_heightMapSubSize;
	std::vector<XMINT2> m_heightMapDivision;

	// Fruits
	std::vector<int> m_fruitType;
	std::vector<float3> m_fruitPos;

	//Playerinfo
	float3 m_playerStartPos;

	//Level utility info
	std::vector<int> m_winCondition;
};

class levelHandler {
private:
	std::vector<level> m_levelsArr;
	
public:
	void initialise();
};