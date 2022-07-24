#pragma once

#include "sprite.h"
#include "frontend.h"
#include "player.h"
#include "ruleset.h"
#include "global.h"
#include "gameSettings.h"
#include "otherObjects.h"
#include "animation.h"
#include "characterSelect.h"
#include "menu.h"
#include <string>
#include <iostream>

#define GAMESTATUS_WAITING      0
#define GAMESTATUS_IDLE         1
#define GAMESTATUS_REMATCHING   2
#define GAMESTATUS_PLAYING      3
#define GAMESTATUS_SPECTATING   4

#define REPLAYSTATE_NORMAL      0
#define REPLAYSTATE_PAUSED      1
#define REPLAYSTATE_FASTFORWARD 2
#define REPLAYSTATE_FASTFORWARDX4 3
#define REPLAYSTATE_REWIND      4

#define PVS_REPLAYVERSION   3

struct PVS_Client;

namespace ppvs
{

struct replay_header
{
	char mw[4]; // = {'R','P','V','S'}
	short versionNumber;
	char date[11]; // = "YYYY:MM:DD"
	char time[9]; // = "HH:MM:SS"
	int duration; // (number of frames)
	char numberOfActivePlayers;
	unsigned int randomseed;
};

struct networkMessage
{
	std::string name;
	std::string mes;
};

class frontend;
class game
{
public:
	game(gameSettings* gs);
	~game();

	void close();

	// Main loop
	bool runGame;

	// Game related
	void initGame(frontend* f);
	void playGame();
	void renderGame();
	void setWindowFocus(bool focus) const;
	void setRules();
	bool isFever() const;

	// Other
	void checkEnd();
	void addPlayer(playerType type, int playernum, int Nplayers);
	void resetPlayers();
	int getActivePlayers() const;

	// Online
	PVS_Client* network;
	std::string channelName;
	bool connected;
	std::deque<networkMessage> messageCenter;
	int currentGameStatus;
	int countActivePlayers() const; // Do not confuse with "getActivePlayers()"
	int countBoundPlayers() const;
	bool stopChaining; // Set true to stop all players from chaining
	bool checkLowestID() const;
	void sendDescription() const;
	std::string sendUpdate() const; // Send the state of player 1 to update spectators
	int choiceTimer;
	int colorTimer;
	int activeAtStart;

	// Public variables
	int menuSelect;
	animation readyGoObj;
	animation backgroundAnimation;
	gameData* data;
	ruleset* currentruleset;
	std::vector<player*> players;
	gameSettings* settings;
	characterSelect* charSelectMenu;
	menu* mainMenu;
	unsigned int randomSeed_NextList; // All players should use the same random seed
	std::deque<std::string> debugMessages;
	bool forceStatusText;
	TranslatableStrings translatableStrings;

	void saveReplay() const;
	void loadReplay(std::string filename);
	int replayTimer;
	std::string winsString;
	void nextReplay();
	void previousReplay();
	int currentReplayVersion;
	int replayState;
	int replayBackwardsTimer;
	bool backwardsOnce;
	bool legacyRandomizer; // See loadReplay
	bool legacyNuisanceDrop;

	// Ranked match
	void rankedMatch();
	int rankedTimer; // Timer that waits until sending a new find opponent request
	int rankedState;
	std::string newRankedMatchMessage;

	// Music
	int targetVolumeNormal;
	int currentVolumeNormal;
	int targetVolumeFever;
	int currentVolumeFever;
	float globalVolume;
	void changeMusicVolume();
	void loadMusic();

private:
	// Main loop
	void Loop();

	// Init
	void loadGlobal();
	void initPlayers();
	void loadSounds() const;
	void loadImages() const;

	// Timers
	int timerEndMatch;
	sprite timerSprite[2];

	// Game objects
	sprite m_spriteBackground;
	sprite black;

	// Text
	ffont* statusFont;
	ftext* statusText;
	std::string lastText;
	void setStatusText(const char* utf8);

	bool playNext; // Helper variable for replays
};

}
