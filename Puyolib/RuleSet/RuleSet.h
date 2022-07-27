#pragma once

#include "../global.h"

namespace ppvs {

class Player;
class Game;

enum class Phase {
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

enum class LoseWinState {
	NOWIN,
	WIN,
	LOSE,
	LOSEWAIT,
};

enum class Rules : int {
	ENDLESS,
	TSU,
	FEVER,
	ENDLESSFEVER,
	ENDLESSFEVERVS,
	TSU_ONLINE,
	FEVER_ONLINE,
	FEVER15_ONLINE,
	ENDLESSFEVERVS_ONLINE,
	EXCAVATION,
};

int getTargetFromMargin(int initialTarget, int marginTime, int currentTime);

class RuleSet {
public:
	RuleSet();
	virtual ~RuleSet();
	void setRules(Rules);
	void setGame(Game*);
	virtual Phase endPhase(Phase, Player*);

    Game* m_currentGame = nullptr;
	GameData* m_data = nullptr;

	// Some constants throughout a game
	int m_fastDrop = 0;
	int m_marginTime = 0;
	int m_initialFeverCount = 0;
	int m_puyoToClear = 0;
	int m_requiredChain = 0;
	int m_targetPoint {}; // Pass to player
	float m_feverPower = 0.f;
	int m_nFeverChains = 0; // 4=ppf, 12=ppf+pp15
	bool m_delayedFall = false;
	bool m_doubleSpawn = false;
	bool m_addDropBonus = false;
	bool m_voicePatternFever = false;
	bool m_allClearStart = false; // Whether the first 4 puyos can create an all clear
	bool m_feverDeath = false; // Whether you can die in fever at t=0
	bool m_bonusEq = false; // Whether a player gets a bonus of 1 garbage preparing an offset
	bool m_quickDrop = false;

	// Pointer to score lists
	// Note: in fever rules, chain bonus is character related
	int *m_linkBonus {}, *m_colorBonus {}, *m_chainBonus {};

	// Rule set function callbacks
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
	Rules m_rules {};
};

class EndlessRuleSet final : public RuleSet {
public:
	EndlessRuleSet();
	~EndlessRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class TsuRuleSet final : public RuleSet {
public:
	TsuRuleSet();
	~TsuRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onWin(Player*) override;
	void onLose(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class FeverRuleSet final : public RuleSet {
public:
	FeverRuleSet();
	~FeverRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class EndlessFeverRuleSet final : public RuleSet {
public:
	EndlessFeverRuleSet();
	~EndlessFeverRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class EndlessFeverVsRuleSet final : public RuleSet {
public:
	EndlessFeverVsRuleSet();
	~EndlessFeverVsRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class TsuOnlineRuleSet final : public RuleSet {
public:
	TsuOnlineRuleSet();
	~TsuOnlineRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onWin(Player*) override;
	void onLose(Player*) override;
	void onOffset(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class FeverOnlineRuleSet final : public RuleSet {
public:
	FeverOnlineRuleSet();
	~FeverOnlineRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class Fever15OnlineRuleSet final : public RuleSet {
public:
	Fever15OnlineRuleSet();
	~Fever15OnlineRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class EndlessFeverVsOnlineRuleSet final : public RuleSet {
public:
	EndlessFeverVsOnlineRuleSet();
	~EndlessFeverVsOnlineRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onSetGame() override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onAllClear(Player*) override;
	void onLose(Player*) override;
	void onWin(Player*) override;
	void onChain(Player*) override;
	void onOffset(Player*) override;
	void onAttack(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

class ExcavationRuleSet final : public RuleSet {
public:
	ExcavationRuleSet();
	~ExcavationRuleSet() override;
	Phase endPhase(Phase currentPhase, Player*) override;
	void onInit(Player*) override;
	void onAllClearPop(Player*) override;
	void onWin(Player*) override;
	void onLose(Player*) override;
	int getLinkBonus(int chain) override;
	int getColorBonus(int chain) override;
	int getChainBonus(Player* pl) override;
};

}
