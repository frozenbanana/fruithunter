#pragma once
#include <Audio.h>


class AudioHandler {
public:
	AudioHandler();
	void play();
	void logStats();

private:
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
	std::unique_ptr<DirectX::SoundEffect> m_soundEffect;
};
