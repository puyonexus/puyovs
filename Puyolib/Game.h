#pragma once

#include "Animation.h"
#include "CharacterSelect.h"
#include "Frontend.h"
#include "GameSettings.h"
#include "Menu.h"
#include "Player.h"
#include "RuleSet/RuleSet.h"
#include "Sprite.h"
#include "global.h"
#include "DebugLog.h"
#include <iostream>
#include <string>

enum class GameStatus {
	WAITING,
	IDLE,
	REMATCHING,
	PLAYING,
	SPECTATING,
};

enum class ReplayState {
	NORMAL,
	PAUSED,
	FAST_FORWARD,
	FAST_FORWARD_X4,
	REWIND,
};

constexpr int kReplayVersion = 3;

struct PVS_Client;

namespace ppvs {

struct ReplayHeader {
	char mw[4]; // = {'R','P','V','S'}
	short versionNumber;
	char date[11]; // = "YYYY:MM:DD"
	char time[9]; // = "HH:MM:SS"
	int duration; // (number of frames)
	char numberOfActivePlayers;
	unsigned int randomSeed;
};

struct NetworkMessage {
	std::string name;
	std::string mes;
};

class Frontend;

class Game {
public:
	explicit Game(GameSettings* gs, DebugLog *dbg);
	~Game();

	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(Game&&) = delete;

	void close();

	// Main loop
	bool m_runGame = false;

	// Game related
	void initGame(Frontend* f);
	void playGame();
	void renderGame();
	void setWindowFocus(bool focus) const;
	void setRules();
	[[nodiscard]] bool isFever() const;

	// Other
	void checkEnd();
	void addPlayer(PlayerType type, int playerNum, int numPlayers);
	void resetPlayers();
	[[nodiscard]] int getActivePlayers() const;

	// Debugging
	DebugLog *m_debug = nullptr;

	// Online
	PVS_Client* m_network = nullptr;
	std::string m_channelName;
	bool m_connected = false;
	std::deque<NetworkMessage> m_messageCenter;
	GameStatus m_currentGameStatus = GameStatus::WAITING;
	[[nodiscard]] int countActivePlayers() const; // Do not confuse with "getActivePlayers()"
	[[nodiscard]] int countBoundPlayers() const;
	bool m_stopChaining = false; // Set true to stop all players from chaining
	[[nodiscard]] bool checkLowestId() const;
	void sendDescription() const;
	[[nodiscard]] std::string sendUpdate() const; // Send the state of player 1 to update spectators
	int m_choiceTimer = 0;
	int m_colorTimer = 10 * 60;
	int m_activeAtStart = 0;

	// Public variables
	int m_menuSelect = 0;
	Animation m_readyGoObj {};
	Animation m_backgroundAnimation {};
	GameData* m_data = nullptr;
	std::string m_baseAssetDir;
	RuleSet* m_currentRuleSet = nullptr;
	std::vector<Player*> m_players;
	GameSettings* m_settings = nullptr;
	CharacterSelect* m_charSelectMenu = nullptr;
	Menu* m_mainMenu = nullptr;
	unsigned int m_randomSeedNextList = 0; // All players should use the same random seed
	//std::deque<std::string> m_debugMessages;
	bool m_forceStatusText = false;
	TranslatableStrings m_translatableStrings {};

	void saveReplay() const;
	void loadReplay(const std::string& filename);
	int m_replayTimer = 3 * 60;
	std::string m_winsString;
	void nextReplay();
	void previousReplay();
	int m_currentReplayVersion = 0;
	ReplayState m_replayState = ReplayState::NORMAL;
	int m_replayBackwardsTimer = 0;
	bool m_backwardsOnce = false;
	bool m_legacyRng = false; // See loadReplay
	bool m_legacyNuisanceDrop = false;

	// Ranked match
	void rankedMatch();
	int m_rankedTimer = 15 * 60; // Timer that waits until sending a new find opponent request
	int m_rankedState = 0;
	std::string m_newRankedMatchMessage;

	// Music
	int m_targetVolumeNormal = 100;
	int m_currentVolumeNormal = 0;
	int m_targetVolumeFever = 100;
	int m_currentVolumeFever = 100;
	float m_globalVolume = 1.f;
	void changeMusicVolume();
	void loadMusic();

private:
	// Main loop
	void loop();

	// Init
	void loadGlobal();
	void initPlayers();
	void loadSounds() const;
	void loadImages() const;

	// Timers
	int m_timerEndMatch = 0;
	Sprite m_timerSprite[2] {};

	// Game objects
	Sprite m_spriteBackground {};
	Sprite m_black {};

	// Text
	FeFont* m_statusFont = nullptr;
	FeText* m_statusText = nullptr;
	std::string m_lastText;
	void setStatusText(const char* utf8);

	bool m_playNext = true; // Helper variable for replays
};

}
