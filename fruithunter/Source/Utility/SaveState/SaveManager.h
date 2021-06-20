#pragma once
#include "GlobalNameSpaces.h"
#include "fstreamLib.h"

#define PLAYER_FILEPATH "assets/"
#define PLAYER_FILENAME "playerProgress"

namespace ProgressStructs {
	class PlayerProgressionBase {
	protected:
		int version;
		virtual void clear() = 0;
		virtual void _read(ifstream& file) = 0;
		virtual void _write(ofstream& file) = 0;
		virtual void _pass(ifstream& file, int _version) = 0;

	public:
		int getVersion() const;
		void read(ifstream& file, int _version);
		void write(ofstream& file);
		PlayerProgressionBase(int _version) { version = _version; }
	};

	class PlayerProgression_Test : public PlayerProgressionBase {
	private:
		void clear();
		void _read(ifstream& file);
		void _write(ofstream& file);
		void _pass(ifstream& file, int  _version);

	public:
		struct SceneCompletion {
			string scene = "";
			time_t time = 0; // milliseconds
		};
		vector<SceneCompletion> level_progress;

		PlayerProgression_Test() : PlayerProgressionBase(0x0001) {}
	};

	class PlayerProgression_1_0 : public PlayerProgressionBase {
	private:
		void clear();
		void _read(ifstream& file);
		void _write(ofstream& file);
		void _pass(ifstream& file, int _version);

	public:
		struct SceneCompletion {
			string scene = "";
			time_t timeToComplete = 0; // milliseconds
		};
		vector<SceneCompletion> level_progress;


		PlayerProgression_1_0() : PlayerProgressionBase(0x0100) {}
	};
}

class SaveManager {
private:
	static SaveManager m_this;

	ProgressStructs::PlayerProgression_1_0 m_progress;

	int readFileVersion(ifstream& file);
	void writeFileVersion(ofstream& file, int _version);
	void load();
	void save();

	SaveManager();
	~SaveManager();
public:
	static SaveManager* getInstance();

	bool getLevelProgress(string scene, time_t& timeMs);
	void setLevelProgress(string scene, time_t timeMs);
};
