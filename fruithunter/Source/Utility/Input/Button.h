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
	bool m_isLowMedHighUltra = false;
	bool m_isResolution = false;
	int m_lowMedHighUltra = 0;
	int m_resolution = 0;

public:
	enum Setting { LOW, MEDIUM, HIGH, ULTRA };
	enum Resolution { HD, FHD, QHD, UHD };

	Button();
	~Button();
	void setLabel(string label);
	void initialize(string label, float2 position);
	void initialize(string label, float2 position, bool on);
	void initialize(string label, float2 position, Setting value);
	void initialize(string label, float2 position, Resolution value);
	bool getOnOff();
	int getLowMedHighUltra();
	int getResolution();
	bool update();
	void draw();
};