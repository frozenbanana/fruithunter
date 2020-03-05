#pragma once
#include "TextRenderer.h"

class Button {
private:
	TextRenderer m_textRenderer;
	string m_label;
	float2 m_position;
	float2 m_size;
	float4 m_colour;

public:
	void setLabel(string label);
	Button();
	~Button();
	void initialize(string label, float2 position);
	bool update();
	void draw();
};