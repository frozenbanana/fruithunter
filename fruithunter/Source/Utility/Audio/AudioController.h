#pragma once
#include <Audio.h>
#include <vector>
#include <string>
#include "Timer.h"

using namespace std;

typedef size_t SoundID;

class AudioController {
public:
	enum SoundType {
		Music,
		Effect
	};

private:
	static AudioController m_this;
	SoundID m_idBank = 0;

	const string m_pre_path = "assets/sounds/";
	const string m_fileEnding = ".wav";

	unique_ptr<AudioEngine> m_audioEngine;

	Timer m_timer;

	float m_masterVolume = 1.0f;
	float m_musicVolume = 0.5f;
	float m_effectsVolume = 0.5f;

	struct SoundTemplate {
		unique_ptr<SoundEffect> effect;
		string name;
	};
	vector<SoundTemplate> m_library;
	struct SoundInstance {
		unique_ptr<SoundEffectInstance> instance;
		SoundType type = SoundType::Effect;
		SoundID id = 0;
		float volume = 1;
	};
	vector<SoundInstance> m_playlist;

	struct SoundEvent {
		SoundID id = 0;
		bool update(float dt) { 
			AudioController* ac = AudioController::getInstance();
			int index = ac->findID(id);
			if (index != -1)
				return update_event(dt, &ac->m_playlist[index]);
			else
				return true;
		}
		virtual bool update_event(float dt, SoundInstance* instance) = 0;
	};
	struct EventFadeIn : SoundEvent {
		float timer = 0;
		float timerMax = 0;
		bool update_event(float dt, SoundInstance* instance) { 
			AudioController* ac = AudioController::getInstance();
			timer = Clamp<float>(timer - dt, 0, timerMax);
			instance->volume = 1-(timer / timerMax);
			ac->updateInstanceVolume(instance);
			return (timer == 0);
		}
		EventFadeIn(SoundID _id, float time) { 
			id = _id;
			timerMax = time;
			timer = timerMax;
		}
	};
	struct EventFadeOut : SoundEvent {
		float timer = 0;
		float timerMax = 0;
		bool update_event(float dt, SoundInstance* instance) {
			AudioController* ac = AudioController::getInstance();
			timer = Clamp<float>(timer - dt, 0, timerMax);
			instance->volume = (timer / timerMax);
			ac->updateInstanceVolume(instance);
			if (timer == 0) {
				ac->stop(id);
				return true;
			}
			return false;
		}
		EventFadeOut(SoundID _id, float time) {
			id = _id;
			timerMax = time;
			timer = timerMax;
		}
	};
	vector<shared_ptr<SoundEvent>> m_events;

	// -- Private Functions --
	int findID(SoundID id);
	int findEffect(string sound);
	bool load(string sound);

	float getVolume(SoundType type) const;
	void updatePlaylistVolumes();
	void updateInstanceVolume(SoundInstance* instance);

	SoundID genID();

	AudioController();

public:
	static AudioController* getInstance();

	void setMasterVolume(float value);
	void setMusicVolume(float value);
	void setEffectsVolume(float value);

	void update();

	/* Clears playlist */
	void flush();

	SoundID play(string sound, AudioController::SoundType type = SoundType::Effect, bool repeat = false);
	//SoundID play3D(string sound, AudioController::SoundType type = SoundType::Effect, bool repeat = false);
	void scaleVolumeByDistance(SoundID id, float distance, float min = 1, float max = 10);
	void setVolume(SoundID id, float value);
	void fadeOut(SoundID id, float time);
	void fadeIn(SoundID id, float time);
	void stop(SoundID id);
	bool isListed(SoundID id);

};
