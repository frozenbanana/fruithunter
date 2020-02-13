#include "LevelHandler.h"

void levelHandler::initialise() {
	level level0;
	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");
	level0.m_heightMapNames.push_back("heightmap3.jpg");

	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(10.f, 0.f, 0.f));
	level0.m_heightMapPos.push_back(float3(0.f, 0.f, 10.f));
	level0.m_heightMapPos.push_back(float3(10.f, 0.f, 10.f));

	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));
	level0.m_heightMapSubSize.push_back(XMINT2(50, 50));

	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));
	level0.m_heightMapDivision.push_back(XMINT2(5, 5));

	level0.m_fruitType.push_back(APPLE);
	level0.m_fruitType.push_back(BANANA);
	level0.m_fruitType.push_back(MELON);

	level0.m_playerStartPos = float3(13.f, 0.0f, 0.0f);

	level0.m_fruitType.push_back(1);
	level0.m_fruitType.push_back(1);
	level0.m_fruitType.push_back(1);

	m_levelsArr.push_back(level0);
}
