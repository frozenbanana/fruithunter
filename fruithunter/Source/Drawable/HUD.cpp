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
	float4 targetColors[NR_OF_TIME_TARGETS + 1] = {
		float4(1.0f, 0.85f, 0.0f, 1.0f),   // gold
		float4(0.8f, 0.8f, 0.8f, 1.0f),	   // silver
		float4(0.85f, 0.55f, 0.25f, 1.0f), // bronze
		float4(1.0f, 0.0f, 0.0f, 1.0f)	   // no target (color)
	};
	float4 color = targetColors[index];
	int target = timeTargets[BRONZE]; // no target (time)
	if (index != NR_OF_TIME_TARGETS)
		target = timeTargets[index];
	string timeString = Time2DisplayableString(target);
	//draw target time
	wstring w_timeString = std::wstring(timeString.begin(), timeString.end());
	m_text.setColor(color);
	m_text.setScale(0.4f);
	m_text.setAlignment(HorizontalAlignment::AlignMiddle, VerticalAlignment::AlignCenter);
	m_text.setPosition(m_stopwatch.getPosition() + float2(0.5f,-0.55f)*m_stopwatch.getSize());
	m_text.setText(timeString);
	m_text.draw();
}

HUD::HUD() {
	m_img_background.load("background.png");
	m_img_background.setPosition(float2(20, 720-130));
	m_img_stamina.load("stamina.png");
	m_img_stamina.setPosition(float2(1280-230, 720-60));
	m_img_staminaFrame.load("staminaFrame.png");
	m_img_staminaFrame.setPosition(m_img_stamina.getPosition() + float2(-13, -10));
	m_img_staminaFrame.setScale(float2(1.05f, 0.8f));

	m_stopwatch.load("stopwatch.png", "stopwatchBack.png", "stopwatchAnimation.png");
	m_stopwatch.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignBottom);
	m_stopwatch.setPosition(float2(0, 720));
	m_stopwatch.setScale(0.35f);

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

void HUD::draw() {
	float stamina = SceneManager::getScene()->m_player->getStamina();

	// Draw text background
	//m_img_background.draw();
	m_img_stamina.setScale(float2(stamina + 0.05f, 0.8f));
	m_img_stamina.draw();
	m_img_staminaFrame.draw();

	// Draw time and target time
	float time = SceneManager::getScene()->m_timer.getTimePassed();
	size_t rest = int((time - int(time)) * 100);
	//string timeString = "    Time: " + Time2DisplayableString((size_t)time)+"."+(rest<10?"0":"")+to_string(rest);
	string timeString = Time2DisplayableString((size_t)time);
	int timeTarget = SceneManager::getScene()->m_utility.timeTargets[TimeTargets::BRONZE];
	// stopwatch
	m_stopwatch.setAnimationFactor(time / timeTarget);
	m_stopwatch.draw();
	// time
	m_text.setColor(float4(0, 0, 0, 1));
	m_text.setScale(0.6f);
	m_text.setAlignment(HorizontalAlignment::AlignMiddle, VerticalAlignment::AlignCenter);
	m_text.setPosition(m_stopwatch.getPosition() + float2(0.5f, -0.4f) * m_stopwatch.getSize());
	m_text.setText(timeString);
	m_text.draw();
	// target time
	drawTargetTime();

	// Draw inventory numbers and fruit sprites
	int* gathered = SceneManager::getScene()->m_gatheredFruits;		  // NR_OF_FRUITS
	int* winCondition = SceneManager::getScene()->m_utility.winCondition; // NR_OF_FRUITS
	float2 itemPosition = float2(50, 50);
	float2 itemOffset = float2(0, 75);
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (winCondition[i] > 0) {
			//text
			string displayStr = to_string(gathered[i]) + "/" + to_string(winCondition[i]);
			m_text.setColor(m_fruitSprites[i].textColor);
			m_text.setScale(0.6f);
			m_text.setAlignment(HorizontalAlignment::AlignLeft, VerticalAlignment::AlignCenter);
			m_text.setPosition(itemPosition + float2(35.0f, 0.0f));
			m_text.setText(displayStr);
			m_text.draw();
			//image
			m_fruitSprites[i].sprite.setPosition(itemPosition);
			m_fruitSprites[i].sprite.draw();
			//next item preparation
			itemPosition += itemOffset;
		}
	}
}
