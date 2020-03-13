#pragma once
#include "TextRenderer.h"

#define COL_INACTIVE float4(1.f, 1.0f, 1.0f, 1.0f)
#define COL_ACTIVE float4(1.f, 0.0f, 0.0f, 1.0f)

class Button {
private:
	TextRenderer m_textRenderer;
	string m_label;
	float2 m_position;
	float2 m_size;
	float4 m_colour;
	bool m_on = true;
	bool m_isToggle = false;

public:
	Button();
	~Button();
	void setLabel(string label);
	void initialize(string label, float2 position);
	void initialize(string label, float2 position, bool on);
	bool getOnOff();
	bool update();
	void draw();
};