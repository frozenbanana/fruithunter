#pragma once
#include "GlobalNamespaces.h"

class MetricCollector {
private:
	// fps
	vector<float> elapsedTimeBank;
	vector<float> fpsBank;
	float fpsCollectorTimer = 0;
	float fpsCollector = 0;
	float fpsCollectorCount = 0;
	const int fpsCollectedSeconds = 60 * 1;


public:
	void reset();

	void update();

	void draw_imgui();
	
};
