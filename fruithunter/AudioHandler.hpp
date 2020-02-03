#pragma once
#include <Audio.h>

enum Sounds {
	MENU,
	PLAY,
};

class AudioHandler {
public:
	AudioHandler();
	void play();
	void startMenuAmbient();
	void startPlayAmbient();
	void pauseAmbient();
	void logStats();

private:
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
	std::unique_ptr<DirectX::SoundEffect> m_ambientMenu;
	std::unique_ptr<DirectX::SoundEffect> m_ambientPlay;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientMenuSound;
	std::unique_ptr<DirectX::SoundEffectInstance> m_ambientPlaySound;
};
