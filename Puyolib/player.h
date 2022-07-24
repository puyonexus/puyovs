#pragma once

#include "controller.h"
#include "movePuyo.h"
#include "fieldProp.h"
#include "field.h"
#include "ruleset.h"
#include "dropPattern.h"
#include "nextPuyo.h"
#include "feverCounter.h"
#include "animation.h"
#include "AI.h"
#include "global.h"
#include "puyorng.h"
#include "mt.h"
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>

namespace ppvs
{
class player;
class game;

struct garbageCounter
{
	int CQ, GQ;
	std::vector<player*> accumulator;
};

struct voices
{
	sound chain[12], damage1, damage2, fever, feversuccess, feverfail, lose, win, choose;
};

enum attackState
{
	NOATTACK, ATTACK, DEFEND, COUNTERATTACK, COUNTERDEFEND, COUNTERIDLE
};

struct replay_player_header
{
	char name[32];
	short currentwins;
	int onlineID;
	char playernum;
	playerType playertype;// (HUMAN or ONLINE)
	puyoCharacter character;
	char active; // Active at start
	char colors;
	int vectorsize_movement; // Size of vector in bytes
	int vectorsize_message;
	int vectorsizecomp_movement; // Size of compressed vector in bytes
	int vectorsizecomp_message;
};

struct messageEvent
{
	int time;
	char message[64];
};

class player
{
public:
	player(playerType type, int playernum, int totalPlayers, game*);
	~player();

	// Initialize
	void reset();
	void initValues(int randomSeed);
	fsound* loadVoice(const std::string& folder, const char* sound);
	void initVoices();
	void initNextList();
	void playerSetup(fieldProp& p, int playernum, int playerTotal);
	void setRandomSeed(unsigned long seed_in, PuyoRNG**);

	// Get and set
	int getRandom(int in, PuyoRNG*);
	[[nodiscard]] int getPlayerNum() const { return m_playernum; }
	[[nodiscard]] playerType getPlayerType() const { return m_type; }
	void setPlayerType(playerType playertype);
	[[nodiscard]] int getReadyGoTimer() const { return m_readyGoTimer; }
	[[nodiscard]] float getGlobalScale() const { return m_globalScale; }
	[[nodiscard]] puyoCharacter getCharacter() const { return m_character; }
	void setCharacter(puyoCharacter pl, bool show = false);
	void setFieldImage(puyoCharacter pc);
	[[nodiscard]] field* getNormalField() { return &m_fieldNormal; }
	[[nodiscard]] field* getFeverField() { return &m_fieldFever; }

	// Garbage
	void updateTray(const garbageCounter* c = nullptr);
	void playGarbageSound();
	int getGarbageSum() const;
	int getAttackSum() const;
	void playLightEffect();
	void addFeverCount();
	void addAttacker(garbageCounter*, player*) const;
	void removeAttacker(garbageCounter*, player*);
	void checkAnyAttacker(garbageCounter*);
	void checkFeverGarbage();
	void addToGQ(garbageCounter*);

	void resetNuisanceDropPattern();
	int nuisanceDropPattern();

	// Other objects
	void setScoreCounter();
	void setScoreCounterPB();
	void showSecondsObj(int n);
	void setMarginTimer();

	// Gameplay
	int TGMR(int color, int n);
	void addNewColorPair(int n);
	void popColor();
	void play();
	void endPhase();
	void prepareVoice(int chain, int predicted);
	void playVoice();
	void getReady();// Phase 0
	void chooseColor();
	void prepare(); // Phase 1
	void cpuMove(); // Phase 10
	void destroyPuyos(); // Phase 31
	void startGarbage(); // Phase 33
	void garbagePhase(); // Phase 34
	void counterGarbage();
	void endGarbage(); // Phase 35
	void checkAllClear(); // Phase 41
	void checkLoser(bool endphase = true); // Phase 43
	void checkWinner();
	void loseGame();
	void winGame();
	void checkFever();// Phase 50
	void startFever();// Phase 51
	void checkEndFever();// Phase 50
	void checkEndFeverOnline();
	void dropFeverChain(); // Phase 52
	void endFever(); // Phase 54
	void playFever();
	void setLose();

	// Draw code
	void draw();
	void drawEffect();
	void drawFieldBack(posVectorFloat position, float rotation);
	void drawFieldFeverBack(posVectorFloat position, float rotation);
	void drawAllClear(posVectorFloat position, float scaleX, float scaleY, float rotation);
	void drawCross(frendertarget* r);
	void drawLose();
	void drawWin();
	void drawColorMenu();

	// Public variables
	gameData* data = nullptr;
	controller controls;
	field* activeField = nullptr;
	fieldProp properties{};
	game* currentgame = nullptr;
	movePuyo movePuyos;
	chainWord* pchainWord = nullptr;
	phase currentphase{};
	feverCounter feverGauge;
	animation characterAnimation;
	voices characterVoices;
	AI* cpuAI = nullptr;
	float dropspeed = 0.f;
	int colors = 0;
	int scoreVal = 0, currentScore = 0;
	int turns = 0;
	int targetPoint = 0;
	bool createPuyo = false;
	bool forgiveGarbage = false;
	bool useDropPattern = false;
	losewinState losewin{};
	unsigned int randomSeedFever = 0u;
	int margintimer = 0;
	bool nextPuyoActive = false;
	int destroyPuyosTimer = 0;
	bool foundChain = false;

	// Color select
	int colorMenuTimer = 0;
	int menuHeight = 0;
	int spiceSelect = 0;
	sprite colorMenuBorder[9];
	sprite spice[5];
	bool pickedColor = false;
	bool takeover = false;

	// Chain state variables
	float garbageTimer = 0.f;
	float garbageSpeed = 0.f;
	int chainPopSpeed = 0;
	int garbageEndTime = 0;
	int puyoBounceSpeed = 0;
	int puyoBounceEnd = 0;
	float gravity = 0.f;
	attackState attdef;
	int chain = 0, puyosPopped = 0, totalGroups = 0,
		groupR = 0, groupG = 0, groupB = 0, groupY = 0, groupP = 0,
		predictedChain = 0, point = 0, bonus = 0, linkBonus = 0,
		allClear = 0, rememberMaxY = 0, rememberX = 0;
	int dropBonus = 0;
	int divider = 0;
	bool bonusEQ = false;

	// Voice
	int diacute = 0;
	int playvoice = 0;
	int stutterTimer = 0;

	// Fever mode
	bool feverMode = false;
	bool feverEnd = false;
	int currentFeverChainAmount = 0;
	bool poppedChain = false; // Is reset during PREPARE phase
	int feverColor = 0; // Use enum?
	float feverColorR = 0.f, feverColorG = 0.f, feverColorB = 0.f; // RGB values used
	int feverSuccess = 0; // Used for playing voice
	void checkAllClearStart();

	// Garbage variables
	int EQ = 0, tray = 0;
	garbageCounter* activeGarbage = nullptr;
	garbageCounter normalGarbage, feverGarbage;
	nuisanceTray normalTray, feverTray;
	std::map<player*, garbageCounter*> targetGarbage;
	int attackers = 0;
	int garbageDropped = 0;
	int garbageCycle = 0;
	player* lastAttacker = nullptr;

	// Online variables
	sprite overlaySprite;
	void bindPlayer(const std::string& name, unsigned int id, bool setActive);
	void unbindPlayer();
	void addMessage(std::string mes);
	std::string onlineName; // Use this to check if peer is bound to player
	std::string previousName; // Set at start of match, useful for replays
	unsigned int onlineID = 0;
	bool active = false; // Acknowledged as active player
	bool prepareActive = false; // FIX: wait setting player to active until end of game
	int activeAtStart; // Active player started game
	bool rematch = false;
	sprite charHolderSprite;
	sprite currentCharacterSprite;
	int showCharacterTimer = 0;
	sprite readyToPlay;
	stringList messages;
	int proposedRandomSeed = 0;
	int wins = 0;
	bool pickingCharacter = false;
	sprite rematchIcon;
	int rematchIconTimer = 0;
	bool loseConfirm = false;
	void getUpdate(std::string str);
	int calledRandomFeverChain = 0;
	bool forceStatusText = false;
	void prepareDisconnect();

	int waitForConfirm = 0;
	void confirmGarbage();
	void waitGarbage();
	void waitLose();

	// Text
	ffont* statusFont = nullptr;
	ftext* statusText = nullptr;
	std::string lastText;
	void setStatusText(const char* utf8);

	// Debugging
	int debug = 0;

	std::vector<messageEvent> recordMessages;

private:
	void processMessage();
	void setDropsetSprite(int x, int y, puyoCharacter pc);

	// Dropset indicator (during char select)
	sprite dropset[16];

	std::vector<lightEffect*> m_lightEffect;
	std::deque<int> m_nextList;
	std::vector<secondsObject*> m_secondsObj;

	// Timers
	int m_lightTimer = 0;
	int m_loseWinTimer = 0;
	int m_readyGoTimer = 0;
	int m_transitionTimer = 0;
	int allclearTimer = 0;
	float m_transformScale = 0.f;

	PuyoRNG* m_randomizerNextList = nullptr;
	PuyoRNG* m_randomizerFeverChain = nullptr;
	PuyoRNG* m_randomizerFeverColor = nullptr;
	MersenneTwister* m_randomizerNuisanceDrop = nullptr;
	std::vector<int> m_nuisanceList;
	long m_randomseed = 0l;
	int m_playernum = 0;
	float m_globalScale = 0.f; // Global scale for certain objects (for example sprite animations)

	playerType m_type;
	scoreCounter m_scoreCounter;
	feverLight m_feverLight;
	puyoCharacter m_character;
	nextPuyo m_nextPuyo;
	field m_fieldNormal;
	field m_fieldFever;
	field m_fieldTemp;
	sprite m_fieldSprite;
	sprite m_fieldFeverSprite;
	sprite m_allclearSprite;
	sprite m_crossSprite;
	sprite m_winSprite;
	sprite m_loseSprite;
	sprite m_borderSprite;
	float m_nextPuyoOffsetX = 0.f;
	float m_nextPuyoOffsetY = 0.f;
	float m_nextPuyoScale = 0.f;

	// FIX: garbage bug
	bool hasMoved = false; // If player went into move phase as indication an attack had ended

	int debugCounter = 0;
};

}
