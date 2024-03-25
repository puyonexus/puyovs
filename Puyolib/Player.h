#pragma once

#include "AI.h"
#include "Animation.h"
#include "Controller.h"
#include "FeverCounter.h"
#include "Field.h"
#include "FieldProp.h"
#include "MovePuyo.h"
#include "NextPuyo.h"
#include "RNG/MersenneTwister.h"
#include "RNG/PuyoRng.h"
#include "RuleSet/RuleSet.h"
#include "global.h"
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace ppvs {

class Player;
class Game;
enum class ImageToken;
enum class SoundEffectToken;

struct GarbageCounter {
	int cq = 0, gq = 0;
	std::vector<Player*> accumulator;
};

struct Voices {
	Sound chain[12], damage1, damage2, fever, feverSuccess, feverFail, lose, win, choose;
};

enum AttackState {
	NO_ATTACK,
	ATTACK,
	DEFEND,
	COUNTER_ATTACK,
	COUNTER_DEFEND,
	COUNTER_IDLE,
};

struct ReplayPlayerHeader {
	char name[32];
	short currentWins;
	int onlineId;
	char playerNum;
	PlayerType playerType; // (HUMAN or ONLINE)
	PuyoCharacter character;
	char active; // Active at start
	char colors;
	int vectorSizeMovement; // Size of vector in bytes
	int vectorSizeMessage;
	int vectorSizeCompressedMovement; // Size of compressed vector in bytes
	int vectorSizeCompressedMessage;
};

struct MessageEvent {
	int time;
	char message[64];
};

class Player {
public:
	Player(PlayerType type, int playerNum, int totalPlayers, Game*);
	~Player();

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;
	Player(Player&&) = delete;
	Player& operator=(Player&&) = delete;

	// Initialize
	void reset();
	void initValues(int randomSeed);
	FeSound* loadVoice(const std::string& folder, const char* sound);
	FeSound* loadVoice(SoundEffectToken token);
	void initVoices();
	void initNextList();
	void playerSetup(FieldProp& properties, int playerNum, int playerTotal);
	void setRandomSeed(unsigned long seedIn, PuyoRng**);

	// Get and set
	static int getRandom(int in, PuyoRng*);
	[[nodiscard]] int getPlayerNum() const { return m_playerNum; }
	[[nodiscard]] PlayerType getPlayerType() const { return m_type; }
	void setPlayerType(PlayerType playerType);
	[[nodiscard]] int getReadyGoTimer() const { return m_readyGoTimer; }
	[[nodiscard]] float getGlobalScale() const { return m_globalScale; }
	[[nodiscard]] PuyoCharacter getCharacter() const { return m_character; }
	void setCharacter(PuyoCharacter character, bool show = false);
	void setFieldImage(PuyoCharacter character);
	[[nodiscard]] Field* getNormalField() { return &m_fieldNormal; }
	[[nodiscard]] Field* getFeverField() { return &m_fieldFever; }

	// Garbage
	void updateTray(const GarbageCounter* c = nullptr);
	void playGarbageSound();
	[[nodiscard]] int getGarbageSum() const;
	[[nodiscard]] int getAttackSum() const;
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
	void getReady(); // Phase 0
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
	void checkFever(); // Phase 50
	void startFever(); // Phase 51
	void checkEndFever(); // Phase 50
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
	GameData* m_data = nullptr;
	Controller m_controls;
	Field* m_activeField = nullptr;
	FieldProp m_properties {};
	Game* m_currentGame = nullptr;
	MovePuyo m_movePuyo;
	ChainWord* m_chainWord = nullptr;
	Phase m_currentPhase {};
	FeverCounter m_feverGauge;
	Animation m_characterAnimation;
	Voices m_characterVoices;
	AI* m_cpuAi = nullptr;
	float m_dropSpeed = 0.f;
	int m_colors = 0;
	int m_scoreVal = 0, m_currentScore = 0;
	int m_turns = 0;
	int m_targetPoint = 0;
	bool m_createPuyo = false;
	bool m_forgiveGarbage = false;
	bool m_useDropPattern = false;
	LoseWinState m_loseWin {};
	unsigned int m_randomSeedFever = 0u;
	int m_marginTimer = 0;
	bool m_nextPuyoActive = false;
	int m_destroyPuyosTimer = 0;
	bool m_foundChain = false;

	// Color select
	int m_colorMenuTimer = 0;
	int m_menuHeight = 0;
	int m_spiceSelect = 0;
	Sprite m_colorMenuBorder[9];
	Sprite m_spice[5];
	bool m_pickedColor = false;
	bool m_takeover = false;

	// Chain state variables
	float m_garbageTimer = 0.f;
	float m_garbageSpeed = 0.f;
	int m_chainPopSpeed = 0;
	int m_garbageEndTime = 0;
	int m_puyoBounceSpeed = 0;
	int m_puyoBounceEnd = 0;
	float m_gravity = 0.f;
	AttackState m_attackState = NO_ATTACK;
	int m_chain = 0, m_puyosPopped = 0, m_totalGroups = 0,
		m_groupR = 0, m_groupG = 0, m_groupB = 0, m_groupY = 0, m_groupP = 0,
		m_predictedChain = 0, m_point = 0, m_bonus = 0, m_linkBonus = 0,
		m_allClear = 0, m_rememberMaxY = 0, m_rememberX = 0;
	int m_dropBonus = 0;
	int m_divider = 0;
	bool m_bonusEq = false;

	// Voice
	int m_diacute = 0;
	int m_playVoice = 0;
	int m_stutterTimer = 0;

	// Fever mode
	bool m_feverMode = false;
	bool m_feverEnd = false;
	int m_currentFeverChainAmount = 0;
	bool m_poppedChain = false; // Is reset during PREPARE phase
	int m_feverColor = 0; // Use enum?
	float m_feverColorR = 0.f, m_feverColorG = 0.f, m_feverColorB = 0.f; // RGB values used
	int m_feverSuccess = 0; // Used for playing voice
	void checkAllClearStart();

	// Garbage variables
	int m_eq = 0, m_tray = 0;
	GarbageCounter* m_activeGarbage = nullptr;
	GarbageCounter m_normalGarbage, m_feverGarbage;
	NuisanceTray m_normalTray, m_feverTray;
	std::map<Player*, GarbageCounter*> m_targetGarbage;
	int m_attackers = 0;
	int m_garbageDropped = 0;
	int m_garbageCycle = 0;
	Player* m_lastAttacker = nullptr;

	// Online variables
	Sprite m_overlaySprite;
	void bindPlayer(const std::string& name, unsigned int id, bool setActive);
	void unbindPlayer();
	void addMessage(std::string mes);
	std::string m_onlineName; // Use this to check if peer is bound to player
	std::string m_previousName; // Set at start of match, useful for replays
	unsigned int m_onlineId = 0;
	bool m_active = false; // Acknowledged as active player
	bool m_prepareActive = false; // FIX: wait setting player to active until end of game
	int m_activeAtStart; // Active player started game
	bool m_rematch = false;
	Sprite m_charHolderSprite;
	Sprite m_currentCharacterSprite;
	int m_showCharacterTimer = 0;
	Sprite m_readyToPlay;
	StringList m_messages;
	int m_proposedRandomSeed = 0;
	int m_wins = 0;
	bool m_pickingCharacter = false;
	Sprite m_rematchIcon;
	int m_rematchIconTimer = 0;
	bool m_loseConfirm = false;
	void getUpdate(std::string str);
	int m_calledRandomFeverChain = 0;
	bool m_forceStatusText = false;
	void prepareDisconnect();

	int m_waitForConfirm = 0;
	void confirmGarbage();
	void waitGarbage();
	void waitLose();

	// Text
	FeFont* m_statusFont = nullptr;
	FeText* m_statusText = nullptr;
	std::string m_lastText;
	void setStatusText(const char* utf8);

	// Debugging
	int m_debug = 0;
	void hotReload();

	std::vector<MessageEvent> m_recordMessages;

private:
	void processMessage();
	void setDropSetSprite(int x, int y, PuyoCharacter pc);

	// Drop set indicator (during char select)
	Sprite m_dropSet[16];

	std::vector<LightEffect*> m_lightEffect;
	std::deque<int> m_nextList;
	std::vector<SecondsObject*> m_secondsObj;

	// Timers
	int m_lightTimer = 0;
	int m_loseWinTimer = 0;
	int m_readyGoTimer = 0;
	int m_transitionTimer = 0;
	int m_allClearTimer = 0;
	float m_transformScale = 0.f;

	PuyoRng* m_rngNextList = nullptr;
	PuyoRng* m_rngFeverChain = nullptr;
	PuyoRng* m_rngFeverColor = nullptr;
	MersenneTwister* m_rngNuisanceDrop = nullptr;
	std::vector<int> m_nuisanceList;
	int m_playerNum = 0;
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
	Sprite m_allClearSprite;
	Sprite m_crossSprite;
	Sprite m_winSprite;
	Sprite m_loseSprite;
	Sprite m_borderSprite;
	float m_nextPuyoOffsetX = 0.f;
	float m_nextPuyoOffsetY = 0.f;
	float m_nextPuyoScale = 0.f;

	// FIX: garbage bug
	bool m_hasMoved = false; // If player went into move phase as indication an attack had ended

	int m_debugCounter = 0;
};

}
