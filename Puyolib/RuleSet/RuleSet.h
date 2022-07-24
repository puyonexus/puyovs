#pragma once

#include "../FieldProp.h"
#include "../global.h"
#include "../DropPattern.h"
#include <vector>
#include <string>

namespace ppvs
{

class Player;
class Game;

enum Phase
{
	PICKCOLORS = -2,
	IDLE = -1,
	GETREADY = 0,
	PREPARE = 1,

	MOVE = 10,
	CREATEPUYO = 20,
	DROPPUYO = 21,
	FALLPUYO = 22,

	SEARCHCHAIN = 30,
	DESTROYPUYO = 32,
	GARBAGE = 33,

	CHECKALLCLEAR = 40,
	DROPGARBAGE = 41,
	FALLGARBAGE = 42,
	CHECKLOSER = 43,
	WAITGARBAGE = 45, // Wait until receving garbage message
	WAITCONFIRMGARBAGE = 46,

	CHECKFEVER = 50,
	PREPAREFEVER = 51,
	DROPFEVER = 52,
	CHECKENDFEVER = 53,
	ENDFEVER = 54,
	CHECKENDFEVERONLINE = 55, // Wait until receiving end fever message

	LOSEDROP = 60,
	WIN_IDLE = 61,
	WAITLOSE = 62 // Online player loses: wait until receiving lose message
};

enum LoseWinState
{
	NOWIN,
	WIN,
	LOSE,
	LOSEWAIT,
};

typedef int Rules;
#define ENDLESS         0
#define TSU             1
#define FEVER           2
#define ENDLESSFEVER    3
#define ENDLESSFEVERVS  4
#define TSU_ONLINE      5
#define FEVER_ONLINE    6
#define FEVER15_ONLINE  7
#define ENDLESSFEVERVS_ONLINE  8
#define EXCAVATION      9

int getTargetFromMargin(int initialTarget, int marginTime, int currentTime);

class RuleSet
{
public:
	RuleSet();
	virtual ~RuleSet();
	void setRules(Rules);
	void setGame(Game*);
	virtual Phase endPhase(Phase, Player*);
	Game* currentgame;
	GameData* data;

	// Some constants throughout a game
	int fastDrop;
	int marginTime;
	int initialFeverCount;
	int puyoToClear;
	int requiredChain;
	int targetPoint; // Pass to player
	float feverPower;
	int NFeverChains; // 4=ppf, 12=ppf+pp15
	bool delayedFall;
	bool doubleSpawn;
	bool addDropBonus;
	bool voicePatternFever;
	bool allClearStart; // Whether the first 4 puyos can create an all clear
	bool feverDeath; // Whether you can die in fever at t=0
	bool bonusEQ; // Whether a player gets a bonus of 1 garbage preparing an offset
	bool quickDrop;

	// Pointer to score lists
	// Note: in fever rules, chainbonus is character related
	int* linkBonus, * colorBonus, * chainBonus;

	// Ruleset function callbacks
	virtual void onSetGame();
	virtual void onInit(Player*);
	virtual void onAllClearPop(Player*);
	virtual void onAllClear(Player*);
	virtual void onLose(Player*);
	virtual void onWin(Player*);
	virtual void onChain(Player*);
	virtual void onOffset(Player*);
	virtual void onAttack(Player*);
	virtual int getLinkBonus(int chain);
	virtual int getColorBonus(int chain);
	virtual int getChainBonus(Player* pl);

private:
	Rules m_rules;
};

class EndlessRuleSet : public RuleSet
{
public:
	EndlessRuleSet();
	~EndlessRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class TsuRuleSet : public RuleSet
{
public:
	TsuRuleSet();
	~TsuRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onWin(Player*);
	void onLose(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class FeverRuleSet : public RuleSet
{
public:
	FeverRuleSet();
	~FeverRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class EndlessFeverRuleSet : public RuleSet
{
public:
	EndlessFeverRuleSet();
	~EndlessFeverRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class EndlessFeverVsRuleSet : public RuleSet
{
public:
	EndlessFeverVsRuleSet();
	~EndlessFeverVsRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class TsuOnlineRuleSet : public RuleSet
{
public:
	TsuOnlineRuleSet();
	~TsuOnlineRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onWin(Player*);
	void onLose(Player*);
	void onOffset(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class FeverOnlineRuleSet : public RuleSet
{
public:
	FeverOnlineRuleSet();
	~FeverOnlineRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class Fever15OnlineRuleSet : public RuleSet
{
public:
	Fever15OnlineRuleSet();
	~Fever15OnlineRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

class EndlessFeverVsOnlineRuleSet : public RuleSet
{
public:
	EndlessFeverVsOnlineRuleSet();
	~EndlessFeverVsOnlineRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onSetGame();
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onAllClear(Player*);
	void onLose(Player*);
	void onWin(Player*);
	void onChain(Player*);
	void onOffset(Player*);
	void onAttack(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};


class ExcavationRuleSet : public RuleSet
{
public:
	ExcavationRuleSet();
	~ExcavationRuleSet();
	Phase endPhase(Phase currentphase, Player*);
	void onInit(Player*);
	void onAllClearPop(Player*);
	void onWin(Player*);
	void onLose(Player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(Player* pl);
};

}
