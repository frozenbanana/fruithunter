#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <vector>

using namespace std;

class PerformanceTimer {
public:
	enum TimeState {
		state_accumulate,
		state_average
	};

private:
	static PerformanceTimer m_this;

	struct Folder {
		double m_collectedTime = 0;//in milliseconds
		size_t m_count = 0;
		TimeState m_state = TimeState::state_accumulate;
		string m_title;
		vector<Folder> m_subFolders;
		double getTotalTime() const;
		string stateToString() const;
		void place(vector<string>& path, double time, TimeState state);
		void increment(double time, TimeState& state);
		void toString(string& str, size_t level, float percentage);

		Folder(string title = "");
	} m_source;

	struct QueueItem {
		string m_title;
		clock_t m_time;
		TimeState m_state;
		float getTimeDifference() const;
		QueueItem(string title, TimeState state) { 
			m_title = title;
			m_state = state;
			m_time = clock();
		}
	};
	vector<QueueItem> m_queue;//maybe use list

	const string m_runtimeAnalysisFilename = "RuntimeAnalysis";
	const string m_sourceTitle = "PROGRAM";
	const string m_preFilePath = "assets/PerformanceAnalysis/";
	const string m_postFileEndings = ".txt";

	//-- Functions --
	vector<string> getPathFromQueue() const;

	void logToFile(string filename);

	PerformanceTimer();
	~PerformanceTimer();

public:
	static PerformanceTimer* getInstance();

	static void start(string title, TimeState state = TimeState::state_accumulate);
	static void stop();

	static void log();

};
