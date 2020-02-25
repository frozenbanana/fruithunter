#pragma once
#include <Audio.h>
#include "GlobalNamespaces.h"
#include "Timer.h"

class AudioHandler {
public:
	enum Sounds {
		LIGHT_ARROW = 0,
		HEAVY_ARROW = 1,
		STRETCH_BOW = 2,
		HIT_WOOD = 3,
		HIT_FRUIT = 4,
		COLLECT = 5,

		SOUNDS_LENGTH = 6
	};
	enum Music {
		JINGLE_GUITAR = 0,
		SPANISH_GUITAR = 1,
		KETAPOP = 2,
		KETAPOP_DARK = 3,
		MUSIC_LENGTH = 4
	};
	bool isPlaying(AudioHandler::Sounds sound);
	void initalize();
	void playMusic(Music music);
	void pauseAllMusic();
	void doTransition(float timeStart, AudioHandler::Music music);
	void changeMusicTo(AudioHandler::Music music, float dt);
	void pauseInstance(AudioHandler::Sounds sound);
	void playOnce(AudioHandler::Sounds sound);
	void playInstance(AudioHandler::Sounds sound);
	void playInstance(AudioHandler::Sounds sound, float coefficient);
	void playOnceByDistance(
		AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition);
	static void logStats();
	static AudioHandler* getInstance();
	AudioHandler() { initalize(); }
	float m_maxHearingDistance = 80.f;

private:
	static AudioHandler m_this;
	Music m_oldMusic;
	float m_oldVolume;
	Music m_currentMusic;
	Sounds m_currentSound;
	Timer m_timer;
	std::unique_ptr<DirectX::SoundEffect> m_soundEffects[SOUNDS_LENGTH];
	std::unique_ptr<DirectX::SoundEffectInstance> m_soundEffectsInstance[SOUNDS_LENGTH];
	std::shared_ptr<DirectX::SoundEffect> m_music[MUSIC_LENGTH];
	std::shared_ptr<DirectX::SoundEffectInstance> m_musicInstances[MUSIC_LENGTH];

	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
};
