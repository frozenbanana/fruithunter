#pragma once
#include <Audio.h>
#include <vector>



class AudioHandler {
public:
	enum Sounds { LALA = 0, LENGTH = 1 };
	void initalize();
	static void startMenuAmbient();
	static void startPlayAmbient();
	static void pauseAmbient();
	void playOneTime(AudioHandler::Sounds sound);
	static void logStats();
	static AudioHandler* getInstance();
	AudioHandler() { initalize(); }

private:
	static AudioHandler m_this;
	std::unique_ptr<DirectX::SoundEffect> m_soundEffects[AudioHandler::LENGTH];
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
	std::unique_ptr<DirectX::SoundEffect> m_ambientMenu;
	std::unique_ptr<DirectX::SoundEffect> m_ambientPlay;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMenuSound;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientPlaySound;
};
