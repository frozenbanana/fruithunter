#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "Text2D.h"
#include "Sprite2DAlphaAnimation.h"

class HUD {
private:
	const float4 m_targetColors[NR_OF_TIME_TARGETS + 1] = {
		float4(212.f / 255.f, 170.f / 255.f, 0.f / 255.f, 1.0f),   // gold
		float4(175.f / 255.f, 175.f / 255.f, 175.f / 255.f, 1.0f), // silver
		float4(220.f / 255.f, 95.f / 255.f, 0.f / 255.f, 1.0f),	   // bronze
		float4(1.0f, 0.0f, 0.0f, 1.0f)							   // no target (color)
	};

	Sprite2DAlphaAnimation m_stopwatch;
	Sprite2D m_stopwatchMarker;

	Sprite2DAlphaAnimation m_slowmo;

	struct TickSetting {
		float slowdown = 0.00001f;
		float gravity = 500.f;
		float kickoff = 10.f;
		float offset_width = 13;
		float offset_height = 60;
		float base_scale = 0.35f;
	} m_tickSetting;

	float m_hd_ticks_scalingVelocity[FruitType::NR_OF_FRUITS] = { 0 };
	float m_hd_ticks_scaling[FruitType::NR_OF_FRUITS] = { 1 };
	Sprite2D m_fruits[FruitType::NR_OF_FRUITS];
	Sprite2D m_ticks[FruitType::NR_OF_FRUITS];
	Sprite2D m_emptyTick;

	struct Tick {
		float scalingVelocity = 0;
		float scale = 1;
	};
	vector<Tick> m_tickAnimations[FruitType::NR_OF_FRUITS];
	int m_tick_previousCatchCount[FruitType::NR_OF_FRUITS] = { 0 };

	Text2D m_text_time;
	Text2D m_text;

	//-- Private Functions --

	void imgui_tickSetting();

	void drawClock(float2 position, float scale, size_t timeMs, float4 color);

public:
	HUD();

	void update(float dt);

	void draw();
};