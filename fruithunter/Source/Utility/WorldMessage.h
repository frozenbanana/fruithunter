#pragma once
#include "Entity.h"
#include "Transformation.h"
#include "Fragment.h"
#include "EffectSystem.h"
#include "Sprite2D.h"
#include "Text2D.h"

class WorldMessage : public Fragment, public Transformation {
private:
	EffectSystem m_effect;

	string m_message = "";

	const float m_range = 4;

	Sprite2D m_info_back;
	Text2D m_info_text;
	float m_info_timer = 0;
	const float2 m_info_position = float2(1280 / 2.f, 640);
	const float m_info_maxWidth = 650;
	const float m_info_secondsPerLetter = 0.05f;//0.02f
	const float m_info_secondsPerLetterBack = 0.01f;
	const float m_info_edgePadding = 10;
	const float m_info_linePadding = 10;
	const float m_letterTransitionTime = 0.7f; // in seconds

	static vector<string> splitText(string text, char splitter) {
		vector<string> sections;
		sections.push_back("");
		for (size_t i = 0; i < text.length(); i++) {
			if (text[i] == splitter) {
				sections.push_back("");
			}
			else {
				sections.back() += text[i];
			}
		}
		return sections;
	}

	static float letterAnimation(float x);

public:

	void update(double dt);

	void draw_message();

	void reset();

	void imgui_properties() override;

	void write(ofstream& file);
	void read(ifstream& file);

	WorldMessage();
	WorldMessage(const WorldMessage& other);

};
