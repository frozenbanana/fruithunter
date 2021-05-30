#include "HUD.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

void HUD::drawClock(float2 position, float scale, time_t timeMs, float4 color) {
	time_t milliseconds = timeMs % 1000;
	time_t seconds = (timeMs / 1000) % 60;
	time_t minutes = (timeMs / 1000) / 60;

	m_text_time.setColor(color);
	m_text_time.setScale(scale);

	m_text_time.setAlignment(HorizontalAlignment::AlignMiddle, VerticalAlignment::AlignCenter);
	m_text_time.setPosition(position);
	m_text_time.setText(":");
	m_text_time.draw();

	float horOffset = 10*scale;
	m_text_time.setAlignment(HorizontalAlignment::AlignRight, VerticalAlignment::AlignCenter);
	m_text_time.setPosition(position + float2(-horOffset, 0));
	m_text_time.setText((minutes < 10 ? "0" : "") + to_string(minutes));
	m_text_time.draw();

	m_text_time.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignCenter);
	m_text_time.setPosition(position + float2(horOffset, 0));
	m_text_time.setText((seconds < 10 ? "0" : "") + to_string(seconds));
	m_text_time.draw();
}

void HUD::imgui_tickSetting() {
	if (ImGui::Begin("TickSettings")) {
		ImGui::SliderFloat("slowdown", &m_tickSetting.slowdown, 0, 1);
		ImGui::InputFloat("gravity", &m_tickSetting.gravity);
		ImGui::InputFloat("kickoff", &m_tickSetting.kickoff);
		ImGui::InputFloat("offset w", &m_tickSetting.offset_width);
		ImGui::InputFloat("offset h", &m_tickSetting.offset_height);
	}
	ImGui::End();
}

HUD::HUD() {
	m_fruits[FruitType::APPLE].load("fruit_apple.png");
	m_fruits[FruitType::BANANA].load("fruit_banana.png");
	m_fruits[FruitType::MELON].load("fruit_melon.png");
	m_fruits[FruitType::DRAGON].load("fruit_dragon.png");
	m_ticks[FruitType::APPLE].load("appleTick.png");
	m_ticks[FruitType::BANANA].load("bananaTick.png");
	m_ticks[FruitType::MELON].load("melonTick.png");
	m_ticks[FruitType::DRAGON].load("dragonTick.png");
	for (size_t i = 0; i < FruitType::NR_OF_FRUITS; i++) {
		m_fruits[i].setScale(m_tickSetting.base_scale);
		m_fruits[i].setAlignment(); // center
		m_ticks[i].setScale(m_tickSetting.base_scale);
		m_ticks[i].setAlignment(); // center
	}
	m_emptyTick.load("emptyTick.png");
	m_emptyTick.setAlignment(); // center
	m_emptyTick.setScale(m_tickSetting.base_scale);

	m_stopwatch.load("stopwatch_front.png", "stopwatch_background.png", "stopwatchAnimation.png");
	m_stopwatch.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignBottom);
	m_stopwatch.setPosition(float2(0, 720) + float2(1, -1) * 30);
	m_stopwatch.setScale(0.275f);

	m_stopwatchMarker.load("square_white.png");
	m_stopwatchMarker.setSize(float2(4, 1) * 5.f);
	m_stopwatchMarker.setAlignment();// center

	m_text_time.setFont("luckiestguy.spritefont");

	m_slowmo.load("slowmo_front.png","slowmo_background.png","slowmo_animation.png");
	m_slowmo.setAlignment(HorizontalAlignment::AlignRight, VerticalAlignment::AlignBottom);
	m_slowmo.setScale(0.45f);
	m_slowmo.setPosition(float2(1280, 720) * 0.975f);
}

void HUD::update(float dt) {

	for (size_t f = 0; f < FruitType::NR_OF_FRUITS; f++) {
		int count_target = SceneManager::getScene()->m_utility.winCondition[f];
		int count_current = SceneManager::getScene()->m_gatheredFruits[f];
		int count_previous = m_tick_previousCatchCount[f];
		// resize if needed
		if (m_tickAnimations[f].size() != count_target) {
			m_tickAnimations[f].resize(count_target); // resize to fit
			m_tick_previousCatchCount[f] = 0;
		}
		// ticks
		for (size_t i = 0; i < m_tickAnimations[f].size(); i++) {
			// activator
			if (i >= count_previous && i < count_current) {
				m_tickAnimations[f][i].scalingVelocity = m_tickSetting.kickoff;
			}
			// animation
			float desiredScale = (i < count_current) ? 1 : 0.75f;
			m_tickAnimations[f][i].scalingVelocity +=
				(desiredScale - m_tickAnimations[f][i].scale) * m_tickSetting.gravity *
				dt;																		 // gravity
			m_tickAnimations[f][i].scale += m_tickAnimations[f][i].scalingVelocity * dt; // movement
			m_tickAnimations[f][i].scalingVelocity *= pow(m_tickSetting.slowdown, dt);	 // slowdown
		}
		// update catch count
		m_tick_previousCatchCount[f] = count_current;
	}

}

void HUD::draw() {
	// Draw stamina
	float stamina = SceneManager::getScene()->m_player->getStamina();
	m_slowmo.setAnimationFactor(stamina);
	m_slowmo.draw();

	// Draw time and target time
	time_t* timeTargets = SceneManager::getScene()->m_utility.timeTargets;
	time_t timeMs = SceneManager::getScene()->m_timer.getTimePassedAsMilliseconds();
	string timeString = Milliseconds2DisplayableString(timeMs);
	// stopwatch
	m_stopwatch.setAnimationFactor((float)timeMs / timeTargets[TimeTargets::BRONZE]);
	m_stopwatch.setFadeLength(0.01f);
	m_stopwatch.draw();
	float2 stopwatchCenter =
		m_stopwatch.getPosition() + float2(0.5f, -(1 - 0.587f)) * m_stopwatch.getSize();
	// time
	drawClock(stopwatchCenter + float2(0, m_stopwatch.getSize().y * 0.05f), 0.475f, timeMs,
		float4(1, 1, 1, 1));
	// time target
	int index = NR_OF_TIME_TARGETS;
	for (int i = 0; i < NR_OF_TIME_TARGETS; i++) {
		if (timeMs <= timeTargets[i]) {
			index = i;
			break;
		}
	}
	time_t timeTarget = (index != NR_OF_TIME_TARGETS) ? timeTargets[index]
												   : timeTargets[BRONZE];
	drawClock(stopwatchCenter + float2(0, m_stopwatch.getSize().y * -0.125f), 0.4f, timeTarget,
		m_targetColors[index]);

	// timetarget markers
	float radius = m_stopwatch.getSize().x * 0.5f * 0.89f;
	for (size_t i = 0; i < TimeTargets::NR_OF_TIME_TARGETS; i++) {
		float factor = (float)SceneManager::getScene()->m_utility.timeTargets[i] /
					   SceneManager::getScene()->m_utility.timeTargets[TimeTargets::BRONZE];
		float radian = (-0.25f * XM_PI) * (1-factor) + (1.25f * XM_PI) * factor;
		m_stopwatchMarker.setPosition(stopwatchCenter + float2(cos(radian), sin(radian)) * radius);
		m_stopwatchMarker.setRotation(radian);
		m_stopwatchMarker.setColor(m_targetColors[i]);
		m_stopwatchMarker.draw();
	}

	// Draw inventory numbers and fruit sprites
	float2 backSize = float2(100, 300);
	float2 tickSize = m_emptyTick.getSize();
	for (size_t f = 0; f < FruitType::NR_OF_FRUITS; f++) {
		float mid = backSize.y / 2;
		float2 baseTickPos = float2(backSize.x - backSize.x / 12,
			mid + m_tickSetting.offset_height * f -
				(m_tickSetting.offset_height * (FruitType::NR_OF_FRUITS - 1)) / 2);

		m_fruits[f].setPosition(float2(baseTickPos.x / 2, baseTickPos.y));
		m_fruits[f].draw();

		int count_current = SceneManager::getScene()->m_gatheredFruits[f];
		for (size_t t = 0; t < m_tickAnimations[f].size(); t++) {
			Sprite2D* spr = (t < count_current) ? &m_ticks[f] : &m_emptyTick;
			spr->setPosition(baseTickPos + float2(t * m_tickSetting.offset_width, 0));
			spr->setScale(m_tickSetting.base_scale * m_tickAnimations[f][t].scale);
			spr->draw();
		}
	}
}
