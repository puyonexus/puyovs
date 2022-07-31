#pragma once

#include "RuleSet/RuleSet.h"
#include <map>
#include <string>

namespace ppvs {

struct RuleSetInfo {
	RuleSetInfo();
	explicit RuleSetInfo(Rules type);

	void setRules(Rules type);

	Rules ruleSetType = Rules::TSU;
	bool custom = false;
	bool quickDrop = false;
	int marginTime = 0;
	int targetPoint = 0;
	int requiredChain = 0;
	int initialFeverCount = 0;
	int feverPower = 0;
	int puyoToClear = 0;
	int numPlayers = 0;
	int colors = 0;
};

struct ReplayRuleSetHeader {
	Rules ruleSetType;
	int marginTime;
	int targetPoint;
	int requiredChain;
	int initialFeverCount;
	int feverPower;
	int puyoToClear;
	int numPlayers;
	int quickDrop;

	int futureRules[10];
};

// Object to pass to game
struct GameSettings {
	explicit GameSettings(const RuleSetInfo& ruleSetInfo = RuleSetInfo());
	~GameSettings();

	GameSettings(const GameSettings&) = delete;
	GameSettings& operator=(const GameSettings&) = delete;
	GameSettings(GameSettings&&) = delete;
	GameSettings& operator=(GameSettings&&) = delete;

	// Game settings
	int numPlayers;
	int numHumans;
	RuleSetInfo ruleSetInfo;
	bool rankedMatch;
	int maxWins;
	RecordState recording;

	// Custom settings
	bool playSound;
	bool playMusic;
	std::string language;
	std::string background;
	std::string puyo;
	std::string sfx;
	std::map<PuyoCharacter, std::string> characterSetup;
	std::map<std::string, std::string> controls;
	PuyoCharacter defaultPuyoCharacter;
    bool swapABConfirm;
	bool useCharacterField;
	bool startWithCharacterSelect;
	bool pickColors;
	bool useCpuPlayers; // This is set for testing or endless
	bool spectating; // Set on if player intends to spectate match
	int showNames; // For replays, 0=show all, 1=not p1, 2=hide all

	// Replay
	std::deque<std::string> replayPlayList;
	std::deque<std::string> oldReplayPlayList;

	// Account settings
};

}
