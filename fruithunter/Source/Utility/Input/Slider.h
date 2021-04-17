#pragma once
#include "Text2D.h"
#include <CommonStates.h>
#include "Sprite2D.h";

class Slider {
private:
	float2 m_position;
	float m_scale = 0.08f;
	string m_label = "";

	const float2 m_sliderOffset = float2(150.f, 0.f);

	const float4 m_grabberColor_highlighted = float4(0.5f, 0.5f, 0.5f, 1.f);
	const float4 m_grabberColor_standard = float4(1.f, 1.f, 1.f, 1.f);
	Color m_colour = m_grabberColor_standard;

	Sprite2D m_img_background, m_img_grabber;

	Text2D m_textRenderer;

	float m_grabPos;
	float m_value;
	float m_preValue; // temp variable when pressing grabber

	bool m_sliding = false;

	float2 getSliderPos() const;

public:
	Slider();
	~Slider();
	void initialize(string label, float2 pos);
	float getValue();
	void setPosition(float2 position);
	void setValue(float value);
	bool update();
	void draw();
};