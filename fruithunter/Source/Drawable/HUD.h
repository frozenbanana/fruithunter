#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "Text2D.h"
#include "Sprite2DAlphaAnimation.h"

class HUD {
private:
	const float4 m_targetColors[NR_OF_TIME_TARGETS + 1] = {
		float4(212.f / 255.f, 170.f / 255.f, 0.f / 255.f, 1.0f),   // gold
		float4(128.f / 255.f, 128.f / 255.f, 128.f / 255.f, 1.0f), // silver
		float4(220.f / 255.f, 95.f / 255.f, 0.f / 255.f, 1.0f),	   // bronze
		float4(1.0f, 0.0f, 0.0f, 1.0f)							   // no target (color)
	};

	struct OnScreenFruitItem {
		Sprite2D sprite;
		float scale = 1;
		float extraScaling = 0; // scales fruit sprite when fruit is picked up. 
		float4 textColor = float4(1.);
	};
	OnScreenFruitItem m_fruitSprites[NR_OF_FRUITS];

	Sprite2D m_img_stamina, m_img_staminaFrame;
	Sprite2DAlphaAnimation m_stopwatch;
	Sprite2D m_box;

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
	Sprite2D m_hd_back;
	Sprite2D m_hd_ticks[FruitType::NR_OF_FRUITS];
	Sprite2D m_hd_emptyTick;

	struct Tick {
		float scalingVelocity = 0;
		float scale = 1;
	};
	vector<Tick> m_tickAnimations[FruitType::NR_OF_FRUITS];
	int m_tick_previousCatchCount[FruitType::NR_OF_FRUITS] = { 0 };

	Text2D m_text_time;
	Text2D m_text;

	//-- Private Functions --

	void drawTargetTime();

	void imgui_tickSetting();

	void drawClock(float2 position, float scale, int time, float4 color);

public:
	HUD();

	void update(float dt);

	void draw();
};