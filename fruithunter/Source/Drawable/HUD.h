#pragma once
#include "GlobalNamespaces.h"
#include "Sprite2D.h"
#include "TextRenderer.h"

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

	TextRenderer m_textRenderer;

	//-- Private Functions --

	void drawTargetTime();

public:
	HUD();

	void draw();
};