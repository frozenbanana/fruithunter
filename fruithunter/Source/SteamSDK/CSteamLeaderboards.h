#pragma once
#include <string>
#include <steam_api.h>
#include <isteamuserstats.h>

using namespace std;

class CSteamLeaderboard {
public:
	enum RequestState {
		r_inactive,
		r_failed,
		r_waiting,
		r_finished
	};

private:
	SteamLeaderboard_t m_CurrentLeaderboard; // Handle to leaderboard

	int m_nLeaderboardEntries;					 // How many entries do we have?
	LeaderboardEntry_t m_leaderboardEntries[10]; // The entries

	RequestState m_reqState_find = RequestState::r_inactive;
	void OnFindLeaderboard(LeaderboardFindResult_t* pResult, bool bIOFailure);
	CCallResult<CSteamLeaderboard, LeaderboardFindResult_t> m_callResultFindLeaderboard;
	RequestState m_reqState_upload = RequestState::r_inactive;
	void OnUploadScore(LeaderboardScoreUploaded_t* pResult, bool bIOFailure);
	CCallResult<CSteamLeaderboard, LeaderboardScoreUploaded_t> m_callResultUploadScore;
	RequestState m_reqState_download = RequestState::r_inactive;
	void OnDownloadScore(LeaderboardScoresDownloaded_t* pResult, bool bIOFailure);
	CCallResult<CSteamLeaderboard, LeaderboardScoresDownloaded_t> m_callResultDownloadScore;

public:
	RequestState getRequestState_FindLeaderboard() const;
	RequestState getRequestState_UploadScore() const;
	RequestState getRequestState_DownloadScore() const;

	bool isLeaderboardLoaded();

	bool FindLeaderboard(const char* pchLeaderboardName);
	bool UploadScore(int score);
	bool DownloadScores();

	int getEntryCount() const;
	bool getEntry(int index, LeaderboardEntry_t& entry) const;

	CSteamLeaderboard();
	~CSteamLeaderboard(){};
};
