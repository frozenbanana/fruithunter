#include "WorldMessage.h"
#include "ErrorLogger.h"
#include "Input.h"
#include "SceneManager.h"
#include "AudioController.h"

float WorldMessage::letterAnimation(float x) { return (4 * pow(x - 1, 3) + 3 * pow(x - 1, 2)) + 1; }

void WorldMessage::update(double dt) {
	int sentenceLength = m_message.length();

	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	float distance = (playerPosition - getPosition()).Length();
	bool withinRange = (distance < m_range);
	float preTimer = m_info_timer;
	// m_info_timer = Clamp<float>(m_info_timer + dt * (withinRange ? 1 : -1), 0,
	//	m_message.length() * m_info_secondsPerLetter + m_letterTransitionTime);
	float secondsPerLetter = (withinRange ? m_info_secondsPerLetter : m_info_secondsPerLetterBack);
	m_info_timer = Clamp<float>(
		m_info_timer + (dt / (secondsPerLetter * sentenceLength)) * (withinRange ? 1 : -1), 0, 1);

	//int preIndex = preTimer / m_info_secondsPerLetter;
	//int currIndex = m_info_timer / m_info_secondsPerLetter;
	int preIndex = preTimer * sentenceLength;
	int currIndex = m_info_timer * sentenceLength;
	int diffIndex = currIndex - preIndex;
	if (diffIndex > 0) {
		SoundID sid = AudioController::getInstance()->play("bubble_pop");
		AudioController::getInstance()->setPitch(sid, RandomFloat(-1, 1) * 0.5f);
	}
}

void WorldMessage::draw_message() {
	int sentenceLength = m_message.length();
	//float letterAnim = m_info_timer / m_info_secondsPerLetter;
	float letterAnim = m_info_timer * sentenceLength;
	int letterMax = Clamp((int)floor(letterAnim), 0, (int)m_message.length());
	float letterRest = letterAnim - letterMax;

	if (letterMax > 0) {
		vector<string> words = splitText(m_message, ' ');
		vector<string> lines = { "" };
		int letterCount = 0;
		for (size_t i = 0; i < words.size(); i++) {
			string word = words[i] + (i + 1 < words.size() ? " " : "");
			string next = lines.back() + word;
			m_info_text.setText(next);
			float2 size = m_info_text.getSize();
			if (size.x > m_info_maxWidth) {
				lines.push_back("");
			}
			for (size_t j = 0; j < word.length() && letterCount < letterMax; j++, letterCount++) {
				lines.back() += word[j];
			}
			if (letterCount >= letterMax)
				break;
		}

		float2 boundSize(0, 0);
		for (size_t i = 0; i < lines.size(); i++) {
			m_info_text.setText(lines[i]);
			float2 size = m_info_text.getSize();
			boundSize.x = max(boundSize.x, size.x);
			boundSize.y += size.y + (i + 1 < lines.size() ? m_info_linePadding : 0);
		}

		m_info_back.setSize(boundSize + float2(1, 1) * m_info_edgePadding * 2);
		m_info_back.setPosition(m_info_position);
		m_info_back.draw();
		float2 offset = -boundSize / 2.f;
		int letterIdx = 0;
		for (size_t i = 0; i < lines.size(); i++) {
			// float letterOffset = 0;
			// for (size_t c = 0; c < lines[i].length(); c++) {
			//	string letter = lines[i].substr(c, 1);
			//	m_info_text.setText(letter);
			//	m_info_text.setPosition(m_info_position + offset + float2(letterOffset, 0));
			//	float scaleFactor = Clamp((m_info_timer - (letterIdx * m_info_secondsPerLetter)) /
			// m_letterTransitionTime, 0.f, 1.f); 	float animation = letterAnimation(scaleFactor);
			//	m_info_text.setScale(0.4f * animation);
			//	m_info_text.draw();
			//	m_info_text.setScale(0.4f);
			//	letterOffset += (letter == " " ? 15 : m_info_text.getSize().x);
			//	letterIdx++;
			//}

			m_info_text.setText(lines[i]);
			m_info_text.setPosition(m_info_position + offset);
			m_info_text.draw();

			offset.y += m_info_text.getSize().y + m_info_linePadding;
		}
	}
}

void WorldMessage::reset() { m_info_timer = 0; }

void WorldMessage::imgui_properties() {
	Transformation::imgui_properties();
	ImGui::InputText("Message", &m_message);
	// ImGui::SliderFloat("Range", &m_range, 1, 20, "%.1f");
}

void WorldMessage::write(ofstream& file) {
	Transformation::stream_write(file);
	fileWrite(file, m_message);
}

void WorldMessage::read(ifstream& file) {
	reset();
	Transformation::stream_read(file);
	fileRead(file, m_message);
}

WorldMessage::WorldMessage() : Fragment(Fragment::Type::entity) {
	m_info_text.setScale(0.4f);
	m_info_text.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignTop);

	m_info_back.load("square_white.png");
	m_info_back.setAlignment();
	m_info_back.setColor(Color(0, 0, 0));
	m_info_back.setAlpha(0.5f);
}

WorldMessage::WorldMessage(const WorldMessage& other) : Fragment(other) {
	m_message = other.m_message;
	m_info_timer = 0;
}