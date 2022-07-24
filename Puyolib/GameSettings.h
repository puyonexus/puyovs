#pragma once

#include "RuleSet/RuleSet.h"
#include "DropPattern.h"
#include <string>
#include <map>

namespace ppvs
{

struct RuleSetInfo
{
	RuleSetInfo();
	RuleSetInfo(Rules type);
	void setRules(Rules type);
	Rules rulesetType;
	bool custom;
	bool quickDrop;
	int marginTime;
	int targetPoint;
	int requiredChain;
	int initialFeverCount;
	int feverPower;
	int puyoToClear;
	int Nplayers;
	int colors;
};

struct ReplayRuleSetHeader
{
	Rules rulesetType;
	int marginTime;
	int targetPoint;
	int requiredChain;
	int initialFeverCount;
	int feverPower;
	int puyoToClear;
	int Nplayers;
	int quickDrop;

	int futureRules[10];
};

// Object to pass to game
struct GameSettings
{
	GameSettings(const RuleSetInfo& rulesetInfo = RuleSetInfo());
	~GameSettings();

	// Game settings
	int Nplayers;
	int Nhumans;
	RuleSetInfo rulesetInfo;
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
	bool useCharacterField;
	bool startWithCharacterSelect;
	bool pickColors;
	bool useCPUplayers; // This is set for testing or endless
	bool spectating; // Set on if player intends to spectate match
	int showNames; // For replays, 0= showall, 1=not p1, 2=hide all

	// Replay
	std::deque<std::string> replayPlayList;
	std::deque<std::string> oldReplayPlayList;

	// Account settings
};

}
