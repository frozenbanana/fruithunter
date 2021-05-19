#include "CSteamLeaderboards.h"
#include "ErrorLogger.h"

bool CSteamLeaderboard::isLeaderboardLoaded() { return m_CurrentLeaderboard != 0; }

int CSteamLeaderboard::getEntryCount() const { 
	if (!m_CurrentLeaderboard)
			return 0;
	return m_nLeaderboardEntries;
}

bool CSteamLeaderboard::getEntry(int index, LeaderboardEntry_t& entry) const { 
	if (index < 0 && index >= m_nLeaderboardEntries)
		return false;
	entry = m_leaderboardEntries[index];
	return true;
	
}

CSteamLeaderboard::CSteamLeaderboard() : m_CurrentLeaderboard(NULL), m_nLeaderboardEntries(0) {}

bool CSteamLeaderboard::FindLeaderboard(const char* pchLeaderboardName) {
	if (m_reqState_find != RequestState::r_waiting) {
		m_reqState_find = RequestState::r_waiting;

		m_CurrentLeaderboard = NULL;

		SteamAPICall_t hSteamAPICall = SteamUserStats()->FindLeaderboard(pchLeaderboardName);
		m_callResultFindLeaderboard.Set(hSteamAPICall, this, &CSteamLeaderboard::OnFindLeaderboard);

		return true;
	}
	else {
		return false;
	}
}

bool CSteamLeaderboard::UploadScore(int score) {
	if (m_reqState_upload != RequestState::r_waiting) {
		if (!m_CurrentLeaderboard) {
			m_reqState_upload = RequestState::r_failed;
			return false;
		}
		m_reqState_upload = RequestState::r_waiting;

		SteamAPICall_t hSteamAPICall = SteamUserStats()->UploadLeaderboardScore(
			m_CurrentLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, score, NULL, 0);
		m_callResultUploadScore.Set(hSteamAPICall, this, &CSteamLeaderboard::OnUploadScore);

		return true;
	}
	else
		return false;
}

bool CSteamLeaderboard::DownloadScores() {
	if (m_reqState_download != RequestState::r_waiting) {
		if (!m_CurrentLeaderboard) {
			m_reqState_download = RequestState::r_failed;
			return false;
		}
		m_reqState_download = RequestState::r_waiting;

		// load the specified leaderboard data around the current user
		SteamAPICall_t hSteamAPICall = SteamUserStats()->DownloadLeaderboardEntries(
			m_CurrentLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -4, 5);
		m_callResultDownloadScore.Set(hSteamAPICall, this, &CSteamLeaderboard::OnDownloadScore);

		return true;
	}
	else
		return false;
}

void CSteamLeaderboard::OnFindLeaderboard(LeaderboardFindResult_t* pCallback, bool bIOFailure) { // see if we encountered an error during the call
	if (!pCallback->m_bLeaderboardFound || bIOFailure) {
		m_reqState_find = RequestState::r_failed;
		ErrorLogger::log("Leaderboard could not be found");
		return;
	}
	m_reqState_find = RequestState::r_finished;

	m_CurrentLeaderboard = pCallback->m_hSteamLeaderboard;
}

void CSteamLeaderboard::OnUploadScore(LeaderboardScoreUploaded_t* pCallback, bool bIOFailure) {
	if (!pCallback->m_bSuccess || bIOFailure) {
		m_reqState_upload = RequestState::r_failed;
		ErrorLogger::log("Score could not be uploaded to Steam\n");
		return;
	}
	m_reqState_upload = RequestState::r_finished;
}

void CSteamLeaderboard::OnDownloadScore(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure) {
	if (!bIOFailure) {
		m_reqState_download = RequestState::r_finished;

		m_nLeaderboardEntries = min(pCallback->m_cEntryCount, 10);

		for (int index = 0; index < m_nLeaderboardEntries; index++) {
			SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries,
				index, &m_leaderboardEntries[index], NULL, 0);
		}
	}
	else
		m_reqState_download = RequestState::r_failed;
}

CSteamLeaderboard::RequestState CSteamLeaderboard::getRequestState_FindLeaderboard() const {
	return m_reqState_find;
}

CSteamLeaderboard::RequestState CSteamLeaderboard::getRequestState_UploadScore() const {
	return m_reqState_upload;
}

CSteamLeaderboard::RequestState CSteamLeaderboard::getRequestState_DownloadScore() const {
	return m_reqState_download;
}
