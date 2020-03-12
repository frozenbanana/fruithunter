#pragma once
#include <Audio.h>
#include "GlobalNamespaces.h"
#include "Timer.h"

class AudioHandler {
public:
	enum Sounds {
		DING_1,
		DING_2,
		DING_3,
		LIGHT_ARROW,
		HEAVY_ARROW,
		STRETCH_BOW,
		HIT_WOOD,
		HIT_FRUIT,
		COLLECT,
		BEAR_PUSH,
		BEAR_HAPPY,
		BEAR_EATING,
		GOAT_PUSH,
		GOAT_HAPPY,
		GOAT_EATING,
		GORILLA_PUSH,
		GORILLA_HAPPY,
		GORILLA_EATING,
		SLEEPING,
		SOUNDS_LENGTH
	};

	enum Music { JINGLE_GUITAR, SPANISH_GUITAR, KETAPOP, KETAPOP_DARK, ELEVATOR, MUSIC_LENGTH };
	bool isPlaying(AudioHandler::Sounds sound);
	void initalize();
	void playMusic(Music music);
	void pauseAllMusic();
	void doTransition(AudioHandler::Music music);
	void changeMusicTo(AudioHandler::Music music, float dt);
	void changeMusicByTag(int, float);
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

	void setMasterVolume(float value);
	void setMusicVolume(float value);
	void setEffectsVolume(float value);

private:
	static AudioHandler m_this;
	Music m_oldMusic;
	float m_oldVolume;

	float m_masterVolume = 1.0f;
	float m_musicVolume = 0.5f;
	float m_effectsVolume = 0.5f;

	Music m_currentMusic;
	Sounds m_currentSound;
	Timer m_timer;
	std::unique_ptr<DirectX::SoundEffect> m_soundEffects[SOUNDS_LENGTH];
	std::unique_ptr<DirectX::SoundEffectInstance> m_soundEffectsInstance[SOUNDS_LENGTH];
	std::shared_ptr<DirectX::SoundEffect> m_music[MUSIC_LENGTH];
	std::shared_ptr<DirectX::SoundEffectInstance> m_musicInstances[MUSIC_LENGTH];

	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;
};
