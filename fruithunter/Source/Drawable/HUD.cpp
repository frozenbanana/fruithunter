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
	string timeString = "Target: " + Time2DisplayableString(target) + ".00";
	//draw target time
	wstring w_timeString = std::wstring(timeString.begin(), timeString.end());
	m_text.setColor(color);
	m_text.setScale(0.45);
	m_text.setAlignment(HorizontalAlignment::Left, VerticalAlignment::Center);
	m_text.setPosition(float2(30, 720 - 100));
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
	m_img_staminaFrame.setScale(float2(1.05, 0.8));

	Color textColors[NR_OF_FRUITS] = { Color(1, 0, 0, 1), Color(0.9, 0.7, 0.2, 1),
		Color(0.4, 0.7, 0.3, 1), Color(1, 0.3, 0.3, 1) };
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
	m_img_background.draw();
	m_img_stamina.setScale(float2(stamina + 0.05, 0.8));
	m_img_stamina.draw();
	m_img_staminaFrame.draw();

	// Draw time and target time
	float time = SceneManager::getScene()->m_timer.getTimePassed();
	size_t rest = int((time - int(time)) * 100);
	string timeString = "    Time: " + Time2DisplayableString((size_t)time)+"."+(rest<10?"0":"")+to_string(rest);
	m_text.setColor(float4(1.));
	m_text.setScale(0.45);
	m_text.setAlignment(HorizontalAlignment::Left, VerticalAlignment::Center);
	m_text.setPosition(float2(30, 720 - 60));
	m_text.setText(timeString);
	m_text.draw();
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
			m_text.setScale(0.6);
			m_text.setAlignment(HorizontalAlignment::Left, VerticalAlignment::Center);
			m_text.setPosition(itemPosition + float2(35,0));
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
