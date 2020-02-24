#pragma once
#include <Audio.h>
#include "GlobalNamespaces.h"

class AudioHandler {
public:
	enum Sounds { LIGHT_ARROW = 0, HEAVY_ARROW = 1, STRETCH_BOW = 2, HIT_WOOD = 3, LENGTH = 4 };
	void initalize();
	static void startMenuAmbient();
	static void startPlayAmbient();
	static void pauseAmbient();
	void playOnce(AudioHandler::Sounds sound);
	void playOnceByDistance(
		AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition);
	static void logStats();
	static AudioHandler* getInstance();
	AudioHandler() { initalize(); }
	float m_maxHearingDistance = 80.f;

private:
	static AudioHandler m_this;
	std::unique_ptr<DirectX::SoundEffect> m_soundEffects[AudioHandler::LENGTH];
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
	std::unique_ptr<DirectX::SoundEffect> m_ambientMenu;
	std::unique_ptr<DirectX::SoundEffect> m_ambientPlay;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMenuSound;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientPlaySound;
};
