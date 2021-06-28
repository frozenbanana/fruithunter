#pragma once
#include "Text2D.h"
#include "Sprite2D.h";

class Slider : public Drawable2D {
private:
	string m_label = "";

	const float2 m_sliderOffset = float2(150.f, 0.f);

	const float4 m_grabberColor_highlighted = float4(0.5f, 0.5f, 0.5f, 1.f);
	const float4 m_grabberColor_standard = float4(1.f, 1.f, 1.f, 1.f);
	Color m_colour = m_grabberColor_standard;

	Sprite2D m_img_background, m_img_grabber;

	Text2D m_textRenderer;

	float m_grabPos = 0;
	float m_value = 0;
	float m_preValue = 0; // temp variable when pressing grabber

	bool m_sliding = false;

	float2 getSliderPos() const;

	void _draw(const Transformation2D& source);

public:
	float2 getLocalSize() const;
	float getValue();
	void setValue(float value);

	bool update();

	void initialize(string label, float2 pos);

	Slider();
	~Slider();
};