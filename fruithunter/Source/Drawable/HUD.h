#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "Text2D.h"
#include "Sprite2DAlphaAnimation.h"

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
	Sprite2DAlphaAnimation m_stopwatch;

	Text2D m_text;

	//-- Private Functions --

	void drawTargetTime();

public:
	HUD();

	void draw();
};