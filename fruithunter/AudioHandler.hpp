#pragma once
#include <Audio.h>


class AudioHandler {
public:
	void initalize();
	void playOneTime(); // one time sounds
	static void startMenuAmbient();
	static void startPlayAmbient();
	static void pauseAmbient();
	static void logStats();
	static AudioHandler* getInstance();
	AudioHandler() { initalize(); }

private:
	static AudioHandler m_this;
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
	std::unique_ptr<DirectX::SoundEffect> m_ambientMenu;
	std::unique_ptr<DirectX::SoundEffect> m_ambientPlay;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMenuSound;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientPlaySound;
};
