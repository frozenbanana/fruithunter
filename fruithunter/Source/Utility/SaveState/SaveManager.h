#pragma once
#include "GlobalNameSpaces.h"

#define MAX_SAVE_SLOTS 3
#define NR_OF_LEVELS 3

struct LevelData{
	bool isCompleted = false;
	size_t timeOfCompletion = 0;//in seconds
};

class SaveManager {
private:
	static SaveManager m_this;

	const string m_prePath = "assets/SaveStates/";

	size_t m_loadedSlot = -1;
	LevelData m_activeState[NR_OF_LEVELS];

	bool loadFile(string filename, LevelData* levels) const;
	bool saveFile(string filename, LevelData* levels) const;
	void saveLoaded();

	SaveManager();
	~SaveManager();
public:
	static SaveManager* getInstance();

	const LevelData* getActiveSave() const;
	vector<LevelData*> getAllSaveStates() const;

	void setLevelCompletion(size_t index, size_t timeOfCompletion);
	void resetSaveState();

	bool load(size_t slot);

};
