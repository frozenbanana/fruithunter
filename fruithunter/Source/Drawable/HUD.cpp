#include "HUD.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

void HUD::drawTargetTime() {
	// Get time passed in seconds
	float timePassed = SceneManager::getScene()->m_timer.getTimePassed();

	int* timeTargets = SceneManager::getScene()->m_utility.timeTargets;
	//find index of achieved target
	int index = NR_OF_TIME_TARGETS; // holds index for timeTargets. If no targetTime is achieved then nr_of_targets is defined as null. 
	for (int i = 0; i < NR_OF_TIME_TARGETS; i++) {
		if (timePassed < timeTargets[i]) {
			index = i;
			break;
		}
	}
	//set color and time target
	float4 color = m_targetColors[index];
	int target = timeTargets[BRONZE]; // no target (time)
	if (index != NR_OF_TIME_TARGETS)
		target = timeTargets[index];
	string timeString = Time2DisplayableString(target);
	//draw target time
	wstring w_timeString = std::wstring(timeString.begin(), timeString.end());
	m_text_time.setColor(color);
	m_text_time.setScale(0.4f);
	m_text_time.setAlignment(HorizontalAlignment::AlignMiddle, VerticalAlignment::AlignCenter);
	m_text_time.setPosition(m_stopwatch.getPosition() + float2(0.5f,-0.55f)*m_stopwatch.getSize());
	m_text_time.setText(timeString);
	m_text_time.draw();
}

void HUD::drawClock(float2 position, float scale, int time, float4 color) {
	int minutes = time / 60;
	int seconds = (int)time % 60;

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
		ImGui::End();
	}
}

HUD::HUD() {
	m_img_stamina.load("stamina.png");
	m_img_stamina.setPosition(float2(1280-230, 720-60));
	m_img_staminaFrame.load("staminaFrame.png");
	m_img_staminaFrame.setPosition(m_img_stamina.getPosition() + float2(-13, -10));
	m_img_staminaFrame.setScale(float2(1.05f, 0.8f));

	m_hd_back.load("hd_back.png");
	m_hd_back.setPosition(float2(0, 0));
	m_hd_back.setScale(0.35f);
	m_hd_ticks[FruitType::APPLE].load("appleTick.png");
	m_hd_ticks[FruitType::BANANA].load("bananaTick.png");
	m_hd_ticks[FruitType::MELON].load("melonTick.png");
	m_hd_ticks[FruitType::DRAGON].load("dragonTick.png");
	for (size_t i = 0; i < FruitType::NR_OF_FRUITS; i++) {
		m_hd_ticks[i].setAlignment(); // center
		m_hd_ticks[i].setScale(m_tickSetting.base_scale);
	}
	m_hd_emptyTick.load("emptyTick.png");
	m_hd_emptyTick.setAlignment(); // center
	m_hd_emptyTick.setScale(m_tickSetting.base_scale);

	m_stopwatch.load("stopwatch.png", "stopwatchBack.png", "stopwatchAnimation.png");
	m_stopwatch.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignBottom);
	m_stopwatch.setPosition(float2(0, 720) + float2(1, -1) * 30);
	m_stopwatch.setScale(0.275f);

	m_box.load("square_white.png");
	m_box.setSize(float2(4, 1) * 5.f);
	m_box.setAlignment();// center

	m_text_time.setFont("luckiestguy.spritefont");

	Color textColors[NR_OF_FRUITS] = { Color(1.0f, 0.0f, 0.0f, 1.0f), Color(0.9f, 0.7f, 0.2f, 1.0f),
		Color(0.4f, 0.7f, 0.3f, 1.0f), Color(1.0f, 0.3f, 0.3f, 1.0f) };
	string fruitImages[NR_OF_FRUITS] = { "apple.png", "banana.png", "melon.png",
		"dragonfruit.png" };
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		m_fruitSprites[i].sprite.load(fruitImages[i]);
		m_fruitSprites[i].sprite.setAlignment(); // center
		m_fruitSprites[i].sprite.setSize(float2(1.)*50);
		m_fruitSprites[i].textColor = textColors[i];
	}
}

void HUD::update(float dt) {

	for (size_t f = 0; f < FruitType::NR_OF_FRUITS; f++) {
		int count_target = SceneManager::getScene()->m_utility.winCondition[f];
		int count_current = SceneManager::getScene()->m_gatheredFruits[f];
		int count_previous = m_tick_previousCatchCount[f];
		// resize if needed
		if (m_tickAnimations[f].size() < count_target) {
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
	float stamina = SceneManager::getScene()->m_player->getStamina();

	// Draw text background
	m_img_stamina.setScale(float2(stamina + 0.05f, 0.8f));
	m_img_stamina.draw();
	m_img_staminaFrame.draw();

	// Draw time and target time
	int* timeTargets = SceneManager::getScene()->m_utility.timeTargets;
	float time = SceneManager::getScene()->m_timer.getTimePassed();
	string timeString = Time2DisplayableString((size_t)time);
	int minutes = time / 60;
	int seconds = (int)time % 60;
	// stopwatch
	m_stopwatch.setAnimationFactor(time / timeTargets[TimeTargets::BRONZE]);
	m_stopwatch.setFadeLength(0.01f);
	m_stopwatch.draw();
	float2 stopwatchCenter =
		m_stopwatch.getPosition() + float2(0.5f, -(1 - 0.587f)) * m_stopwatch.getSize();
	// time
	drawClock(stopwatchCenter + float2(0, m_stopwatch.getSize().y * 0.05f), 0.475f, time,
		float4(0, 0, 0, 1));
	// time target
	float timePassed = SceneManager::getScene()->m_timer.getTimePassed();
	int index = NR_OF_TIME_TARGETS;
	for (int i = 0; i < NR_OF_TIME_TARGETS; i++) {
		if (timePassed < timeTargets[i]) {
			index = i;
			break;
		}
	}
	int timeTarget = (index != NR_OF_TIME_TARGETS) ? timeTargets[index]
												   : timeTargets[BRONZE];
	drawClock(stopwatchCenter + float2(0, m_stopwatch.getSize().y * -0.125f), 0.4f, timeTarget,
		m_targetColors[index]);

	// timetarget markers
	float radius = m_stopwatch.getSize().x * 0.5f * 0.89f;
	for (size_t i = 0; i < TimeTargets::NR_OF_TIME_TARGETS; i++) {
		float factor = (float)SceneManager::getScene()->m_utility.timeTargets[i] /
					   SceneManager::getScene()->m_utility.timeTargets[TimeTargets::BRONZE];
		float radian = (-0.25f * XM_PI) * (1-factor) + (1.25f * XM_PI) * factor;
		m_box.setPosition(stopwatchCenter + float2(cos(radian), sin(radian)) * radius);
		m_box.setRotation(radian);
		m_box.setColor(m_targetColors[i]);
		m_box.draw();
	}

	// Draw inventory numbers and fruit sprites
	//int* gathered = SceneManager::getScene()->m_gatheredFruits;		  // NR_OF_FRUITS
	//int* winCondition = SceneManager::getScene()->m_utility.winCondition; // NR_OF_FRUITS
	//float2 itemPosition = float2(50, 50);
	//float2 itemOffset = float2(0, 75);
	//for (size_t i = 0; i < NR_OF_FRUITS; i++) {
	//	if (winCondition[i] > 0) {
	//		//text
	//		string displayStr = to_string(gathered[i]) + "/" + to_string(winCondition[i]);
	//		m_text.setColor(m_fruitSprites[i].textColor);
	//		m_text.setScale(0.6f);
	//		m_text.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignCenter);
	//		m_text.setPosition(itemPosition + float2(35.0f, 0.0f));
	//		m_text.setText(displayStr);
	//		m_text.draw();
	//		//image
	//		m_fruitSprites[i].sprite.setPosition(itemPosition);
	//		m_fruitSprites[i].sprite.draw();
	//		//next item preparation
	//		itemPosition += itemOffset;
	//	}
	//}


	m_hd_back.draw();
	float2 backSize = m_hd_back.getSize();
	float2 tickSize = m_hd_emptyTick.getSize();
	for (size_t f = 0; f < FruitType::NR_OF_FRUITS; f++) {
		float mid = m_hd_back.getSize().y / 2;
		float2 baseTickPos = float2(backSize.x - backSize.x / 12,
			mid + m_tickSetting.offset_height * f -
				(m_tickSetting.offset_height * (FruitType::NR_OF_FRUITS - 1)) / 2);
		int count_target = SceneManager::getScene()->m_utility.winCondition[f];
		int count_current = SceneManager::getScene()->m_gatheredFruits[f];
		for (size_t t = 0; t < count_target; t++) {
			Sprite2D* spr = (t < count_current) ? &m_hd_ticks[f] : &m_hd_emptyTick;
			spr->setPosition(baseTickPos + float2(t * m_tickSetting.offset_width, 0));
			spr->setScale(m_tickSetting.base_scale * m_tickAnimations[f][t].scale);
			spr->draw();
		}
	}
}
