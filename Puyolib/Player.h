#pragma once

#include "Controller.h"
#include "MovePuyo.h"
#include "FieldProp.h"
#include "Field.h"
#include "RuleSet/RuleSet.h"
#include "DropPattern.h"
#include "NextPuyo.h"
#include "FeverCounter.h"
#include "Animation.h"
#include "AI.h"
#include "global.h"
#include "RNG/PuyoRNG.h"
#include "RNG/MersenneTwister.h"
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>

namespace ppvs
{

class Player;
class Game;

struct GarbageCounter
{
	int CQ, GQ;
	std::vector<Player*> accumulator;
};

struct Voices
{
	Sound chain[12], damage1, damage2, fever, feversuccess, feverfail, lose, win, choose;
};

enum AttackState
{
	NOATTACK,
	ATTACK,
	DEFEND,
	COUNTERATTACK,
	COUNTERDEFEND,
	COUNTERIDLE,
};

struct ReplayPlayerHeader
{
	char name[32];
	short currentwins;
	int onlineID;
	char playernum;
	PlayerType playertype;// (HUMAN or ONLINE)
	PuyoCharacter character;
	char active; // Active at start
	char colors;
	int vectorsize_movement; // Size of vector in bytes
	int vectorsize_message;
	int vectorsizecomp_movement; // Size of compressed vector in bytes
	int vectorsizecomp_message;
};

struct MessageEvent
{
	int time;
	char message[64];
};

class Player
{
public:
	Player(PlayerType type, int playernum, int totalPlayers, Game*);
	~Player();

	// Initialize
	void reset();
	void initValues(int randomSeed);
	FeSound* loadVoice(const std::string& folder, const char* sound);
	void initVoices();
	void initNextList();
	void playerSetup(FieldProp& p, int playernum, int playerTotal);
	void setRandomSeed(unsigned long seed_in, PuyoRNG**);

	// Get and set
	int getRandom(int in, PuyoRNG*);
	[[nodiscard]] int getPlayerNum() const { return m_playernum; }
	[[nodiscard]] PlayerType getPlayerType() const { return m_type; }
	void setPlayerType(PlayerType playertype);
	[[nodiscard]] int getReadyGoTimer() const { return m_readyGoTimer; }
	[[nodiscard]] float getGlobalScale() const { return m_globalScale; }
	[[nodiscard]] PuyoCharacter getCharacter() const { return m_character; }
	void setCharacter(PuyoCharacter pl, bool show = false);
	void setFieldImage(PuyoCharacter pc);
	[[nodiscard]] Field* getNormalField() { return &m_fieldNormal; }
	[[nodiscard]] Field* getFeverField() { return &m_fieldFever; }

	// Garbage
	void updateTray(const GarbageCounter* c = nullptr);
	void playGarbageSound();
	int getGarbageSum() const;
	int getAttackSum() const;
	void playLightEffect();
	void addFeverCount();
	void addAttacker(GarbageCounter*, Player*) const;
	void removeAttacker(GarbageCounter*, Player*);
	void checkAnyAttacker(GarbageCounter*);
	void checkFeverGarbage();
	void addToGQ(GarbageCounter*);

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
	void drawFieldBack(PosVectorFloat position, float rotation);
	void drawFieldFeverBack(PosVectorFloat position, float rotation);
	void drawAllClear(PosVectorFloat position, float scaleX, float scaleY, float rotation);
	void drawCross(FeRenderTarget* r);
	void drawLose();
	void drawWin();
	void drawColorMenu();

	// Public variables
	GameData* data = nullptr;
	Controller controls;
	Field* activeField = nullptr;
	FieldProp properties{};
	Game* currentgame = nullptr;
	MovePuyo movePuyos;
	ChainWord* pchainWord = nullptr;
	Phase currentphase{};
	FeverCounter feverGauge;
	Animation characterAnimation;
	Voices characterVoices;
	AI* cpuAI = nullptr;
	float dropspeed = 0.f;
	int colors = 0;
	int scoreVal = 0, currentScore = 0;
	int turns = 0;
	int targetPoint = 0;
	bool createPuyo = false;
	bool forgiveGarbage = false;
	bool useDropPattern = false;
	LoseWinState losewin{};
	unsigned int randomSeedFever = 0u;
	int margintimer = 0;
	bool nextPuyoActive = false;
	int destroyPuyosTimer = 0;
	bool foundChain = false;

	// Color select
	int colorMenuTimer = 0;
	int menuHeight = 0;
	int spiceSelect = 0;
	Sprite colorMenuBorder[9];
	Sprite spice[5];
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
	AttackState attdef;
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
	GarbageCounter* activeGarbage = nullptr;
	GarbageCounter normalGarbage, feverGarbage;
	NuisanceTray normalTray, feverTray;
	std::map<Player*, GarbageCounter*> targetGarbage;
	int attackers = 0;
	int garbageDropped = 0;
	int garbageCycle = 0;
	Player* lastAttacker = nullptr;

	// Online variables
	Sprite overlaySprite;
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
	Sprite charHolderSprite;
	Sprite currentCharacterSprite;
	int showCharacterTimer = 0;
	Sprite readyToPlay;
	stringList messages;
	int proposedRandomSeed = 0;
	int wins = 0;
	bool pickingCharacter = false;
	Sprite rematchIcon;
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
	FeFont* statusFont = nullptr;
	FeText* statusText = nullptr;
	std::string lastText;
	void setStatusText(const char* utf8);

	// Debugging
	int debug = 0;

	std::vector<MessageEvent> recordMessages;

private:
	void processMessage();
	void setDropsetSprite(int x, int y, PuyoCharacter pc);

	// Dropset indicator (during char select)
	Sprite dropset[16];

	std::vector<LightEffect*> m_lightEffect;
	std::deque<int> m_nextList;
	std::vector<SecondsObject*> m_secondsObj;

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

	PlayerType m_type;
	ScoreCounter m_scoreCounter;
	FeverLight m_feverLight;
	PuyoCharacter m_character;
	NextPuyo m_nextPuyo;
	Field m_fieldNormal;
	Field m_fieldFever;
	Field m_fieldTemp;
	Sprite m_fieldSprite;
	Sprite m_fieldFeverSprite;
	Sprite m_allclearSprite;
	Sprite m_crossSprite;
	Sprite m_winSprite;
	Sprite m_loseSprite;
	Sprite m_borderSprite;
	float m_nextPuyoOffsetX = 0.f;
	float m_nextPuyoOffsetY = 0.f;
	float m_nextPuyoScale = 0.f;

	// FIX: garbage bug
	bool hasMoved = false; // If player went into move phase as indication an attack had ended

	int debugCounter = 0;
};

}
