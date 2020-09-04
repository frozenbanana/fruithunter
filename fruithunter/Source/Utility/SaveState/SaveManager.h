#pragma once
#include "GlobalNameSpaces.h"

struct SceneCompletion {
	string scene = "";
	size_t timeToComplete = 0;// in seconds
	TimeTargets grade = TimeTargets::NR_OF_TIME_TARGETS; // no medal
	bool isCompleted() const { return grade != TimeTargets::NR_OF_TIME_TARGETS; }
};

class SaveManager {
private:
	static SaveManager m_this;

	const string m_filename = "player_progress";
	const string m_path_wd = "assets/"; // working directory
	vector<SceneCompletion> m_progress;

	void load();
	void save();

	SaveManager();
	~SaveManager();
public:
	static SaveManager* getInstance();

	static const SceneCompletion* getProgress(string scene);
	static void setProgress(string scene, size_t timeToComplete, TimeTargets grade);
	static void resetProgression();

};
