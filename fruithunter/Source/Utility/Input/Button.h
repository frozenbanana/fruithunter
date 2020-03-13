#pragma once
#include "TextRenderer.h"

class Button {
private:
	TextRenderer m_textRenderer;
	string m_label;
	float2 m_position;
	float2 m_size;
	float4 m_colour;
	bool m_on = true;
	bool m_isToggle = false;
	bool m_isLowMedHighUltra = false;
	int m_lowMedHighUltra = 0;

public:
	enum Setting { LOW, MEDIUM, HIGH, ULTRA };

	Button();
	~Button();
	void setLabel(string label);
	void initialize(string label, float2 position);
	void initialize(string label, float2 position, bool on);
	void initialize(string label, float2 position, Setting value);
	bool getOnOff();
	int getLowMedHighUltra();
	bool update();
	void draw();
};