#pragma once
#include <Audio.h>
#include <vector>
#include <string>
#include "Timer.h"

using namespace std;

typedef size_t SoundID;

class AudioController {
private:
	static AudioController m_this;
	SoundID m_idBank = 0;

	const string m_pre_path = "assets/sounds/";
	const string m_fileEnding = ".wav";

	AudioEngine m_audioEngine;

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
		SoundID id = 0;
	};
	vector<SoundInstance> m_playlist;

	// -- Private Functions --
	int findID(SoundID id);
	int findEffect(string sound);
	bool load(string sound);

	SoundID genID();

	AudioController();

public:
	static AudioController* getInstance();

	void setMasterVolume(float value);
	void setMusicVolume(float value);
	void setEffectsVolume(float value);

	void update();

	SoundID play(string sound, bool repeat = false);
	void stop(SoundID id);
	bool isListed(SoundID id);



};
