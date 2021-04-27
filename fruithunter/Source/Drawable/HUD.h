#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "Text2D.h"

class HUD {
private:
	struct OnScreenFruitItem {
		Sprite2D sprite;
		float scale = 1;
		float extraScaling = 0; // scales fruit sprite when fruit is picked up. 
		float4 textColor = float4(1.);
	};
	OnScreenFruitItem m_fruitSprites[NR_OF_FRUITS];

	Sprite2D m_img_background, m_img_stamina, m_img_staminaFrame;

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

	Text2D m_text;

	//-- Private Functions --

	void drawTargetTime();

	void imgui_tickSetting();

public:
	HUD();

	void update(float dt);

	void draw();
};