#pragma once

#include "fieldProp.h"
#include "global.h"
#include "dropPattern.h"
#include <vector>
#include <string>

namespace ppvs
{

class player;
class game;

enum phase
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

enum losewinState
{
	NOWIN, WIN, LOSE, LOSEWAIT
};

typedef int rules;
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

class ruleset
{
public:
	ruleset();
	virtual ~ruleset();
	void setRules(rules);
	void setGame(game*);
	virtual phase endPhase(phase, player*);
	game* currentgame;
	gameData* data;

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
	virtual void onInit(player*);
	virtual void onAllClearPop(player*);
	virtual void onAllClear(player*);
	virtual void onLose(player*);
	virtual void onWin(player*);
	virtual void onChain(player*);
	virtual void onOffset(player*);
	virtual void onAttack(player*);
	virtual int getLinkBonus(int chain);
	virtual int getColorBonus(int chain);
	virtual int getChainBonus(player* pl);
protected:
private:
	rules m_rules;
};

class ruleset_ENDLESS : public ruleset
{
public:
	ruleset_ENDLESS();
	~ruleset_ENDLESS();
	phase endPhase(phase currentphase, player*);
	void onInit(player*);
	void onAllClearPop(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_TSU : public ruleset
{
public:
	ruleset_TSU();
	~ruleset_TSU();
	phase endPhase(phase currentphase, player*);
	void onInit(player*);
	void onAllClearPop(player*);
	void onWin(player*);
	void onLose(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_FEVER : public ruleset
{
public:
	ruleset_FEVER();
	~ruleset_FEVER();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_ENDLESSFEVER : public ruleset
{
public:
	ruleset_ENDLESSFEVER();
	~ruleset_ENDLESSFEVER();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_ENDLESSFEVERVS : public ruleset
{
public:
	ruleset_ENDLESSFEVERVS();
	~ruleset_ENDLESSFEVERVS();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_TSU_ONLINE : public ruleset
{
public:
	ruleset_TSU_ONLINE();
	~ruleset_TSU_ONLINE();
	phase endPhase(phase currentphase, player*);
	void onInit(player*);
	void onAllClearPop(player*);
	void onWin(player*);
	void onLose(player*);
	void onOffset(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_FEVER_ONLINE : public ruleset
{
public:
	ruleset_FEVER_ONLINE();
	~ruleset_FEVER_ONLINE();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_FEVER15_ONLINE : public ruleset
{
public:
	ruleset_FEVER15_ONLINE();
	~ruleset_FEVER15_ONLINE();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

class ruleset_ENDLESSFEVERVS_ONLINE : public ruleset
{
public:
	ruleset_ENDLESSFEVERVS_ONLINE();
	~ruleset_ENDLESSFEVERVS_ONLINE();
	phase endPhase(phase currentphase, player*);
	void onSetGame();
	void onInit(player*);
	void onAllClearPop(player*);
	void onAllClear(player*);
	void onLose(player*);
	void onWin(player*);
	void onChain(player*);
	void onOffset(player*);
	void onAttack(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};


class ruleset_EXCAVATION : public ruleset
{
public:
	ruleset_EXCAVATION();
	~ruleset_EXCAVATION();
	phase endPhase(phase currentphase, player*);
	void onInit(player*);
	void onAllClearPop(player*);
	void onWin(player*);
	void onLose(player*);
	int getLinkBonus(int chain);
	int getColorBonus(int chain);
	int getChainBonus(player* pl);
};

}
