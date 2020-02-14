#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"

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

	// Playerinfo
	float3 m_playerStartPos;

	// Level utility info
	int m_winCondition[FRUITS];
};

class levelHandler {
private:
	std::vector<level> m_levelsArr;
	
public:
	void initialise();
	void loadLevel(int levelNr);

	std::vector<Fruit> fruitsInLevel;
};