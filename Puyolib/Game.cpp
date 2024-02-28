#include "Game.h"
#include "../PVS_ENet/PVS_Channel.h"
#include "../PVS_ENet/PVS_Client.h"
#include <cstdio>
#include <ctime>
#include <fstream>
#include <zlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ppvs {

using ppvs::DebugMessageType;

void encode(const char* key, char* in, const int length)
{
	for (int i = 0; i < length; i++) {
		in[i] = static_cast<char>(in[i] ^ key[i % strlen(key)]);
	}
}

Game* activeGame = nullptr;

Game::Game(GameSettings* gs, DebugLog* dbg)
	: m_settings(gs)
	, m_debug(dbg)
{
	initGlobal();

	activeGame = this;
	m_choiceTimer = (gs->rankedMatch == true ? 5 : 30) * 60;

	m_black.setImage(nullptr);
	m_black.setScale(640 * 2, 480);
	m_black.setColor(0, 0, 0);
	m_black.setTransparency(0.5f);
	m_black.setPosition(-640.f / 2.f, -480.f / 4.f);

	m_statusFont = nullptr;
	m_statusText = nullptr;
}

Game::~Game()
{
	// Delete players
	while (!m_players.empty()) {
		delete m_players.back();
		m_players.pop_back();
	}

	// Delete associated global properties
	delete m_settings;
	delete m_charSelectMenu;
	delete m_mainMenu;
	delete m_currentRuleSet;
	delete m_statusText;
	delete m_statusFont;
	delete m_debugFont;
	delete m_data->front;
	delete m_data;
	delete m_debug;
}

void Game::close()
{
	// Close game
	m_runGame = false;
}

void Game::loadGlobal()
{
	// Load user settings
	m_baseAssetDir = m_settings->baseAssetDir;
	m_data->gUserSettings.backgroundDirPath = m_settings->background;
	m_data->gUserSettings.puyoDirPath = m_settings->puyo;
	m_data->gUserSettings.sfxDirPath = m_settings->sfx;

	// Set global images
	loadImages();

	// Load all sounds
	loadSounds();

	// Init shaders
	m_data->glowShader = nullptr;
	m_data->tunnelShader = nullptr;

	if (useShaders) {
		static auto glowShaderSource = "uniform sampler2D tex;"
									   "uniform float color;"
									   "void main()"
									   "{"
									   "   gl_FragColor=texture2D(tex,gl_TexCoord[0].xy)+vec4(color,color,color,0);"
									   "}";

		m_data->glowShader = m_data->front->loadShader(glowShaderSource);
		if (m_data->glowShader) {
			m_data->glowShader->setCurrentTexture("tex");
			m_data->glowShader->setParameter("color", 1.0);
		}

		static auto tunnelShaderSource = "uniform vec4 cl;"
										 "uniform float time;"
										 "uniform sampler2D tex;"
										 "void main(void) {"
										 "   vec2 ccord = gl_TexCoord[0].xy;"
										 "   vec2 pcord;"
										 "   vec2 final;"
										 "   float zoomspeed=0.5;"
										 "   float rotatespeed=0.25;"
										 "   ccord.x = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.x)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.x);"
										 "   ccord.y = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.y)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.y);"
										 "   pcord.x = 0.1/sqrt(ccord.x*ccord.x+ccord.y*ccord.y);pcord.y = atan(ccord.y,ccord.x)/3.141592;"
										 "   final.x = step(0.25,pcord.x)*mod(pcord.x+zoomspeed*time,0.5)+(1.0-step(0.25,pcord.x))*mod(pcord.x+zoomspeed*time,0.5);"
										 "   final.y = step(0.25,pcord.y)*mod(pcord.y+rotatespeed*time,0.5)+(1.0-step(0.25,pcord.y))*mod(pcord.y+rotatespeed*time,0.5);"
										 "   vec3 col = texture2D(tex,final).xyz;"
										 "   gl_FragColor = vec4(max(col/((pcord/0.1).x),0.1), 1.0) * cl;"
										 "}";

		m_data->tunnelShader = m_data->front->loadShader(tunnelShaderSource);
		if (m_data->tunnelShader) {
			m_data->tunnelShader->setCurrentTexture("tex");
			m_data->tunnelShader->setParameter("time", 1.0);
			m_data->tunnelShader->setParameter("cl", 0.0, 0.0, 1.0, 1.0);
		}
	}

	// Other
	m_data->globalTimer = 0;
	m_data->windowFocus = true;
	m_data->playSounds = m_settings->playSound;
	m_data->playMusic = m_settings->playMusic;

	m_statusFont = m_data->front->loadFont("Arial", 14);
	m_debugFont = m_data->front->loadFont("Arial", 14);
	setStatusText("");
}

void Game::loadSounds() const
{
	Sounds& snd = m_data->snd;
	m_debug->log("Loading the sound", DebugMessageType::Debug);
	setBuffer(snd.chain[0], m_assetManager->loadSound("chain1"));
	setBuffer(snd.chain[1], m_assetManager->loadSound("chain2"));
	setBuffer(snd.chain[2], m_assetManager->loadSound("chain3"));
	setBuffer(snd.chain[3], m_assetManager->loadSound("chain4"));
	setBuffer(snd.chain[4], m_assetManager->loadSound("chain5"));
	setBuffer(snd.chain[5], m_assetManager->loadSound("chain6"));
	setBuffer(snd.chain[6], m_assetManager->loadSound("chain7"));
	setBuffer(snd.allClearDrop, m_assetManager->loadSound("allClearDrop"));
	setBuffer(snd.drop, m_assetManager->loadSound("drop"));
	setBuffer(snd.fever, m_assetManager->loadSound("fever"));
	setBuffer(snd.feverLight, m_assetManager->loadSound("feverLight"));
	setBuffer(snd.feverTimeCount, m_assetManager->loadSound("feverTimeCount"));
	setBuffer(snd.feverTimeEnd, m_assetManager->loadSound("feverTimeEnd"));
	setBuffer(snd.go, m_assetManager->loadSound("go"));
	setBuffer(snd.heavy, m_assetManager->loadSound("heavy"));
	setBuffer(snd.hit, m_assetManager->loadSound("hit"));
	setBuffer(snd.lose, m_assetManager->loadSound("lose"));
	setBuffer(snd.move, m_assetManager->loadSound("move"));
	setBuffer(snd.nuisanceHitL, m_assetManager->loadSound("nuisanceHitL"));
	setBuffer(snd.nuisanceHitM, m_assetManager->loadSound("nuisanceHitM"));
	setBuffer(snd.nuisanceHitS, m_assetManager->loadSound("nuisanceHitS"));
	setBuffer(snd.nuisanceL, m_assetManager->loadSound("nuisanceL"));
	setBuffer(snd.nuisanceS, m_assetManager->loadSound("nuisanceS"));
	setBuffer(snd.ready, m_assetManager->loadSound("ready"));
	setBuffer(snd.rotate, m_assetManager->loadSound("rotate"));
	setBuffer(snd.win, m_assetManager->loadSound("win"));
	setBuffer(snd.decide, m_assetManager->loadSound("decide"));
	setBuffer(snd.cancel, m_assetManager->loadSound("cancel"));
	setBuffer(snd.cursor, m_assetManager->loadSound("cursor"));
}

// Load images (user defined)
void Game::loadImages() const
{
	// Load puyo
	m_data->imgPuyo = m_assetManager->loadImage("puyo");

	// Check rotation center of quadruplet
	if (m_data->imgPuyo && !m_data->imgPuyo->error() && m_data->imgPuyo->height() > 10) {
		for (int i = 0; i < kPuyoX; i++) {
			if (m_data->imgPuyo->pixel(11 * kPuyoX - i, 14 * kPuyoY).a > 50) {
				m_data->quadrupletCenter = kPuyoX - i;
				break;
			}
		}
	}
	m_data->imgPuyo->setFilter(FilterType::LinearFilter);

	// Lights
	m_data->imgLight = m_assetManager->loadImage("imgLight");
	m_data->imgLightS = m_assetManager->loadImage("imgLightS");
	m_data->imgLightHit = m_assetManager->loadImage("imgLightHit");
	m_data->imgFSparkle = m_assetManager->loadImage("imgFSparkle");
	m_data->imgFLight = m_assetManager->loadImage("imgFLight");
	m_data->imgFLightHit = m_assetManager->loadImage("imgFLightHit");

	m_data->imgTime = m_assetManager->loadImage("imgTime");

	// Menu
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			m_data->imgMenu[i][j] = m_assetManager->loadImage("imgMenu", toString(i)+ toString(j));
		}
	}

	// Backgrounds
	m_data->imgBackground = m_assetManager->loadImage("imgBackground");
	m_data->imgFieldFever = m_assetManager->loadImage("imgFieldFever");
	// Background of next puyo
	m_data->imgNextPuyoBackgroundR = m_assetManager->loadImage("imgNextPuyoBackgroundR");
	m_data->imgNextPuyoBackgroundL = m_assetManager->loadImage("imgNextPuyoBackgroundL");

	if (!useShaders) {
		for (int i = 0; i < 30; ++i) {
			m_data->imgFeverBack[i] = m_assetManager->loadImage("imgFeverBack", toString(i));
		}
	}

	// Load default fields. Custom fields should be loaded per character
	m_data->imgField1 = m_assetManager->loadImage("imgField1");
	m_data->imgField2 = m_assetManager->loadImage("imgField2");
	m_data->imgBorder1 = m_assetManager->loadImage("imgBorder1");
	m_data->imgBorder2 = m_assetManager->loadImage("imgBorder2");
	m_data->imgPlayerBorder = m_assetManager->loadImage("imgPlayerBorder");
	m_data->imgSpice = m_assetManager->loadImage("imgSpice");

	// Other
	m_data->imgScore = m_assetManager->loadImage("imgScore");
	m_data->imgAllClear = m_assetManager->loadImage("imgAllClear");
	m_data->imgLose = m_assetManager->loadImage("imgLose");
	m_data->imgWin = m_assetManager->loadImage("imgWin");
	m_data->imgFeverGauge = m_assetManager->loadImage("imgFeverGauge");
	m_data->imgSeconds = m_assetManager->loadImage("imgSeconds");
	m_data->imgCharHolder = m_assetManager->loadImage("imgCharHolder");
	m_data->imgNameHolder = m_assetManager->loadImage("imgNameHolder");
	m_data->imgBlack = m_assetManager->loadImage("imgBlack");
	m_data->imgDropSet = m_assetManager->loadImage("imgDropSet");
	m_data->imgChain = m_assetManager->loadImage("imgChain");
	m_data->imgCheckMark = m_assetManager->loadImage("imgCheckMark");
	m_data->imgPlayerCharSelect = m_assetManager->loadImage("imgPlayerCharSelect");

	for (int i = 0; i < kNumCharacters; ++i) {
		m_data->imgCharField[i] = m_assetManager->loadCharImage("imgCharField", static_cast<PuyoCharacter>(i));
		m_data->imgCharSelect[i] = m_assetManager->loadCharImage("imgCharSelect", static_cast<PuyoCharacter>(i));
		m_data->imgCharName[i] = m_assetManager->loadCharImage("imgCharName", static_cast<PuyoCharacter>(i));
		m_data->imgSelect[i] = m_assetManager->loadCharImage("imgCharSelect", static_cast<PuyoCharacter>(i));
	}

	m_data->imgPlayerNumber = m_assetManager->loadImage("imgPlayerNumber");
}

void Game::initPlayers()
{
	// Create players according to settings
	int n = 1;
	for (int i = 0; i < m_settings->numHumans; i++) {
		// Assign human players
		addPlayer(HUMAN, n, m_settings->numPlayers);
		// Give human controls
		n++;
	}

	if (m_settings->useCpuPlayers) {
		for (int i = 0; i < m_settings->numPlayers - m_settings->numHumans; i++) {
			// CPU players
			addPlayer(CPU, n, m_settings->numPlayers);
			n++;
		}
	} else {
		for (int i = 0; i < m_settings->numPlayers - m_settings->numHumans; i++) {
			// Online players
			addPlayer(ONLINE, n, m_settings->numPlayers);
			n++;
		}
	}
}

// Count players that haven't lost
int Game::getActivePlayers() const
{
	int pl = 0;
	for (const auto& player : m_players) {
		if (player->m_loseWin == LoseWinState::NOWIN && player->m_active) {
			pl++;
		}
	}
	return pl;
}

void Game::initGame(Frontend* f, AssetManager* as_mgr)
{
	m_data = new GameData;
	m_data->front = f;

	m_assetManager = as_mgr;
	m_assetManager->activate(f, m_debug);

	loadGlobal();

	// Set random seed
	m_randomSeedNextList = getRandom(100000);

	// Rule settings
	setRules();

	// Set background
	m_spriteBackground.setImage(m_data->imgBackground);
	m_spriteBackground.setPosition(0.f, 0.f);
	initPlayers();
	m_runGame = true;
	if (!m_network) // Start with character select menu or not
	{
		m_menuSelect = static_cast<int>(m_settings->startWithCharacterSelect);
	}
	m_timerEndMatch = 0;

	// Initialize readygo animation
	m_readyGoObj.init(m_data, PosVectorFloat(320, 240), 1, m_assetManager->getAnimationFolder("bg", "ready.xml"), "ready.xml", 3 * 60);
	m_backgroundAnimation.init(m_data, PosVectorFloat(320, 240), 1,m_assetManager->getAnimationFolder("bg"), "animation.xml", 30 * 60);
	m_backgroundAnimation.prepareAnimation("background");

	// Other stuff
	m_charSelectMenu = new CharacterSelect(this);
	m_charSelectMenu->prepare();
	m_mainMenu = new Menu(this);

	m_timerSprite[0].setImage(m_data->imgPlayerNumber);
	m_timerSprite[1].setImage(m_data->imgPlayerNumber);
	m_timerSprite[0].setSubRect(0 / 10 * 24, 0, 0, 32);
	m_timerSprite[1].setSubRect(0 / 10 * 24, 0, 0, 32);
	m_timerSprite[0].setCenterBottom();
	m_timerSprite[1].setCenterBottom();
	m_timerSprite[0].setPosition(640 - 24, 32);
	m_timerSprite[1].setPosition(640 - 24 - 24, 32);
}

// Set rules from RuleSetInfo.
void Game::setRules()
{
	delete m_currentRuleSet;
	m_currentRuleSet = nullptr;

	// Create rule object
	switch (m_settings->ruleSetInfo.ruleSetType) {
	case Rules::ENDLESS:
		m_currentRuleSet = new EndlessRuleSet();
		break;
	case Rules::TSU:
		m_currentRuleSet = new TsuRuleSet();
		break;
	case Rules::FEVER:
		m_currentRuleSet = new FeverRuleSet();
		break;
	case Rules::ENDLESSFEVER:
		m_currentRuleSet = new EndlessFeverRuleSet();
		break;
	case Rules::ENDLESSFEVERVS:
		m_currentRuleSet = new EndlessFeverVsRuleSet();
		break;
	case Rules::TSU_ONLINE:
		m_currentRuleSet = new TsuOnlineRuleSet();
		break;
	case Rules::FEVER_ONLINE:
		m_currentRuleSet = new FeverOnlineRuleSet();
		break;
	case Rules::FEVER15_ONLINE:
		m_currentRuleSet = new Fever15OnlineRuleSet();
		break;
	case Rules::ENDLESSFEVERVS_ONLINE:
		m_currentRuleSet = new EndlessFeverVsOnlineRuleSet();
		break;
	case Rules::EXCAVATION:
		m_currentRuleSet = new ExcavationRuleSet();
		break;
	}
	// TODO: Should log some kind of error here.
	if (m_currentRuleSet == nullptr) {
		m_currentRuleSet = new EndlessRuleSet();
	}
	m_currentRuleSet->setGame(this);
	m_currentRuleSet->m_marginTime = m_settings->ruleSetInfo.marginTime * 60;
	if (m_settings->ruleSetInfo.targetPoint > 0)
		m_currentRuleSet->m_targetPoint = m_settings->ruleSetInfo.targetPoint;
	if (m_settings->ruleSetInfo.feverPower > 0)
		m_currentRuleSet->m_feverPower = static_cast<float>(m_settings->ruleSetInfo.feverPower) / 100.0f;
	if (m_settings->ruleSetInfo.puyoToClear > 0)
		m_currentRuleSet->m_puyoToClear = m_settings->ruleSetInfo.puyoToClear;
	if (m_settings->ruleSetInfo.requiredChain >= 0)
		m_currentRuleSet->m_requiredChain = m_settings->ruleSetInfo.requiredChain;
	if (m_settings->ruleSetInfo.initialFeverCount >= 0 && m_settings->ruleSetInfo.initialFeverCount <= 7)
		m_currentRuleSet->m_initialFeverCount = m_settings->ruleSetInfo.initialFeverCount;
	if (m_settings->ruleSetInfo.quickDrop)
		m_currentRuleSet->m_quickDrop = m_settings->ruleSetInfo.quickDrop;
	if (m_settings->ruleSetInfo.colors >= 3 && m_settings->ruleSetInfo.colors <= 5) {
		for (const auto& player : m_players)
			player->m_colors = m_settings->ruleSetInfo.colors;
		m_settings->pickColors = false;
	} else {
		for (const auto& player : m_players)
			player->m_colors = 4;
	}
}

bool Game::isFever() const
{
	bool fever = false;
	for (const auto& player : m_players) {
		if (player->m_active && player->m_feverMode) {
			fever = true;
		}
	}
	return fever;
}

void Game::playGame()
{
	if (m_settings->recording == RecordState::REPLAYING && m_replayState == ReplayState::PAUSED) {
		return;
	}

	// Set controller states
	for (int i = 0; i < static_cast<int>(m_players.size()); i++) {
		if (m_settings->useCpuPlayers && i >= m_settings->numHumans)
			break;

		FeInput input = m_data->front->inputState(i);
		m_players[i]->m_controls.setState(input, m_data->matchTimer);
	}

	// Active player when NOT playing
	for (const auto& player : m_players) {
		if (m_currentGameStatus != GameStatus::PLAYING && player->m_prepareActive) {
			player->m_active = true;
			player->m_prepareActive = false;
		}
	}

	// Set choice timer
	if (m_currentGameStatus == GameStatus::IDLE && !m_settings->useCpuPlayers) {
		m_timerSprite[0].setSubRect(m_choiceTimer / 60 % 10 * 24, 0, 24, 32);
		m_timerSprite[1].setSubRect(m_choiceTimer / 60 / 10 * 24, 0, 24, 32);

		m_timerSprite[1].setVisible(m_choiceTimer > 10 * 60);

		if (m_choiceTimer > 0) {
			m_choiceTimer--;
		}

		// No-one to rematch
		if (countActivePlayers() <= 1 && m_settings->rankedMatch == false) {
			m_choiceTimer = -1;
		}

		// 2 player game: no count down
		if (m_players.size() == 2 && m_settings->rankedMatch == false) {
			m_choiceTimer = -1;
		}

		// Check if new player came on
		if (m_choiceTimer < 0 && countActivePlayers() > 1 && !m_settings->rankedMatch) {
			m_choiceTimer = 30 * 60;
		}

		// Ranked: start counting as soon as opponent enters
		if (m_choiceTimer < 0 && countBoundPlayers() > 1 && m_settings->rankedMatch == true) {
			m_choiceTimer = 5 * 60;
		}

		// Time is up
		if (m_choiceTimer == 0) {
			m_players[0]->m_controls.release();
			if (m_settings->swapABConfirm == false) {
				m_players[0]->m_controls.m_a = 1;
			} else {
				m_players[0]->m_controls.m_b = 1;
			}
			// Select rematch
			m_mainMenu->m_select = 0;
		}
	}
	// Color timer menu
	if (m_players[0]->getPlayerType() == HUMAN && m_players[0]->m_currentPhase == Phase::PICKCOLORS && !m_settings->useCpuPlayers) {
		m_timerSprite[0].setSubRect(((m_colorTimer / 60) % 10) * 24, 0, 24, 32);
		m_timerSprite[1].setSubRect((m_colorTimer / 60 / 10) * 24, 0, 24, 32);

		if (m_colorTimer > 10 * 60) {
			m_timerSprite[1].setVisible(true);
		} else {
			m_timerSprite[1].setVisible(false);
		}

		if (m_colorTimer > 0) {
			m_colorTimer--;
		}
	}
	// Replay timer
	if (m_settings->recording == RecordState::REPLAYING && m_replayTimer > 0) {
		m_replayTimer--;
		if (m_replayTimer == 1) {
			// Load new replay
			nextReplay();
		}
	}

	// Menus (display them over the main game)
	// Character select
	if (m_menuSelect == 1) {
		m_charSelectMenu->play();
	} else if (m_menuSelect == 2) {
		m_mainMenu->play();
	}

	// Check if match can start
	if (m_currentGameStatus == GameStatus::REMATCHING) {
		int rematching = 0;
		int active = 0;
		for (const auto& player : m_players) {
			if (player->m_rematch) {
				rematching++;
			}
			if (player->m_active) {
				active++;
			}
		}
		if (active == rematching && rematching > 1 && active > 0) {
			// Start
			// Reset rematch status
			for (const auto& player : m_players) {
				player->m_rematch = false;
				player->m_rematchIcon.setVisible(false);
			}
			if (!m_settings->spectating) {
				m_currentGameStatus = GameStatus::PLAYING;
			} else {
				m_currentGameStatus = GameStatus::SPECTATING;
				m_menuSelect = 0;
			}

			// Start game
			resetPlayers();
		} else if (rematching == 0 && active == 0) {
			m_currentGameStatus = GameStatus::IDLE;
		}
	}

	// Ready go
	if (!m_players.empty()) {
		m_readyGoObj.playAnimation();
	}
	m_backgroundAnimation.playAnimation();

	// Replay
	if (m_settings->recording == RecordState::REPLAYING) {
		const int t = m_data->matchTimer;
		for (const auto& player : m_players) {
			// Check event by looping through vector
			// If the time is -1, it's considered as processed
			for (size_t i = 0; i < player->m_recordMessages.size(); i++) {
				if (player->m_recordMessages[i].time == t) {
					// Process all events with the correct time
					// Ignore color select
					if (player->m_recordMessages[i].message[0] == 's'
						|| player->m_recordMessages[i].message[0] == 'c') {
						continue;
					}

					// Add message
					player->addMessage(player->m_recordMessages[i].message);
					player->m_recordMessages[i].time = -1;
				} else if (player->m_recordMessages[i].time > t) {
					break;
				}
			}
		}
	}

	rankedMatch();

	// Main game
	for (const auto& player : m_players) {
		player->play();
	}

	// Check end of match
	checkEnd();

	// Set status text
	for (size_t i = 0; i < m_players.size(); i++) {
		std::string str;
		if (m_settings->recording == RecordState::REPLAYING) {
			if (m_players[i]->m_onlineName.empty())
				continue;

			// Show playername and wins
			if (m_settings->showNames == 0 || (m_settings->showNames == 1 && i == 0)) {
				str += m_players[i]->m_onlineName + "\n";
			} else {
				str += "...\n";
			}
			str += "Wins: " + toString(m_players[i]->m_wins) + "\n";
			m_players[i]->setStatusText(str.c_str());
			continue;
		}

		// Player is bound
		if (!m_players[i]->m_onlineName.empty()) {
			str += m_players[i]->m_onlineName + "\n";
			str += "Wins: " + toString(m_players[i]->m_wins) + "\n";

			if (m_players[i]->getPlayerType() == ONLINE) {
				if (!m_players[i]->m_active)
					str += "Waiting to join...\n";
				else {
					if (m_players[i]->m_pickingCharacter && (m_currentGameStatus == GameStatus::IDLE || m_currentGameStatus == GameStatus::REMATCHING))
						str += "Currently picking\n a character.";
					else if (!m_players[i]->m_rematch && (m_currentGameStatus == GameStatus::IDLE || m_currentGameStatus == GameStatus::REMATCHING))
						str += "Waiting for rematch.\n";
				}
			}
			m_players[i]->setStatusText(str.c_str());
		}
	}

	changeMusicVolume();

	m_data->globalTimer++;
	m_data->matchTimer++;

	// Access menu during endless mode
	if (m_settings->ruleSetInfo.numPlayers == 1 && m_menuSelect == 0) {
		if (!m_players.empty() && m_players[0]->m_loseWin == LoseWinState::NOWIN && m_players[0]->m_controls.m_start > 0) {
			m_players[0]->setLose();
		}
	}

	if (m_backwardsOnce)
		return;

	// Replay normal
	if (m_settings->recording == RecordState::REPLAYING && m_replayState == ReplayState::NORMAL)
		m_replayBackwardsTimer = m_data->matchTimer;

	// Replay backwards
	if (m_settings->recording == RecordState::REPLAYING && m_replayState == ReplayState::REWIND) {
		if (m_replayBackwardsTimer > 0) {
			m_replayBackwardsTimer -= 60;
		}

		if (!m_settings->oldReplayPlayList.empty()) {
			const bool soundSettings = m_settings->playSound;
			m_settings->playSound = false;
			m_data->playSounds = false;

			loadReplay(m_settings->oldReplayPlayList.back());
			// Get ready to play
			resetPlayers();
			m_data->matchTimer = 0;
			// Loop until time is reached
			while (m_data->matchTimer < m_replayBackwardsTimer) {
				m_replayState = ReplayState::NORMAL;
				m_backwardsOnce = true;
				playGame();
				m_replayState = ReplayState::REWIND;
			}
			m_backwardsOnce = false;
			m_settings->playSound = soundSettings;
			m_data->playSounds = soundSettings;
			m_replayState = ReplayState::NORMAL;
		} else {
			m_replayBackwardsTimer = m_data->matchTimer;
			m_replayState = ReplayState::NORMAL;
		}
	}
	// Replay fastforward
	if (m_settings->recording == RecordState::REPLAYING && m_replayState == ReplayState::FAST_FORWARD && m_data->matchTimer % 2 == 0) {
		playGame();
	} else if (m_settings->recording == RecordState::REPLAYING && m_replayState == ReplayState::FAST_FORWARD_X4 && m_data->matchTimer % 4 == 0) {
		playGame();
		playGame();
		playGame();
	}
}

void Game::renderGame()
{
	// Tunnel shader
	if (m_data->tunnelShader) {
		m_data->tunnelShader->setParameter("time", static_cast<double>(m_data->globalTimer) / 60.0);
	}

	// Clear screen
	m_data->front->clear();

	// Draw background
	m_spriteBackground.draw(m_data->front);
	m_backgroundAnimation.draw();

	// Draw timer
	if (m_currentGameStatus == GameStatus::IDLE && !m_settings->useCpuPlayers && (countActivePlayers() > 1 || m_settings->rankedMatch)) {
		m_timerSprite[0].draw(m_data->front);
		m_timerSprite[1].draw(m_data->front);
	}
	if (m_players[0]->getPlayerType() == HUMAN && m_players[0]->m_currentPhase == Phase::PICKCOLORS
		&& !m_settings->useCpuPlayers && !m_players[0]->m_pickedColor) {
		m_timerSprite[0].draw(m_data->front);
		m_timerSprite[1].draw(m_data->front);
	}

	// Draw player related objects
	for (const auto& player : m_players) {
		// Draw fields
		player->draw();
	}
	for (const auto& player : m_players) {
		// Draw light effect
		player->drawEffect();
	}
	if (!m_players.empty()) {
		// Needs at least 1 player to draw ready-go
		m_readyGoObj.draw();
	}

	// Draw menuSelects
	if (m_menuSelect == 1) {
		m_charSelectMenu->draw();
	}
	if (m_menuSelect == 2) {
		m_mainMenu->draw();
	}

	// Darken screen
	if (m_rankedState >= 3) {
		m_black.draw(m_data->front);
		// Draw status text
		if (m_statusText) {
			m_data->front->setColor(255, 255, 255, 255);
			m_statusText->draw(8, 0);
		}
	}

	// Render debug text
	updateDebugText();
	if (m_debugText) {
		m_data->front->setColor(255, 255, 255, 255);
		m_debugText->draw(16, 0);
	}

	m_data->front->swapBuffers();
}

void Game::setStatusText(const char* utf8)
{
	if (utf8 == m_lastText)
		return;
	if (!m_statusFont)
		return;
	delete m_statusText;

	m_statusText = m_statusFont->render(utf8);
	m_lastText = utf8;
}

// Uses the status text font to render the current debug string
void Game::updateDebugText()
{
	if (!m_debugFont)
		return;
	if (debugString.empty())
		return;
	delete m_debugText;

	m_debugText = m_debugFont->render(debugString.c_str());
}

void Game::setWindowFocus(bool focus) const
{
	m_data->windowFocus = focus;
}

// Main loop of the game
void Game::loop()
{
	assert(m_assetManager != nullptr);
	initGame(m_data->front, m_assetManager);

	Sprite debugSprite;
	debugSprite.setImage(m_data->imgNextPuyoBackgroundL);

	// Frame limiter variables
	double t = 0.0;
	const double dt = 1.0 / 60.0;
	double accumulator = 0.0;
	uint64_t startTime = timeGetTime();

	// FPS counter
	double second = 0;
	int fps = 0;
	int fpsCounter = 0;

	// Main loop
	while (m_runGame) {
		// Frame limiter
		const uint64_t endTime = timeGetTime();
		const double deltaTime = static_cast<double>(endTime - startTime) / 1000.0; // Convert to second
		startTime = endTime;
		accumulator += deltaTime;

		// FPS counter (screen refresh rate)
		second += deltaTime;
		if (second > 1) {
			fps = 0;
			fpsCounter = 0;
			second = 0;
		}

		// Main gameplay loop
		while (accumulator >= dt) {
			// Inner game loop
			playGame();

			// End of loop
			// Update framelimiter variables
			t += dt;
			accumulator -= dt;
			// FPS
			fpsCounter++;

			// Draw frame in the last loop
			if (accumulator < dt) {
				// Screen refresh rate
				fps++;

				// Innner loop render
				renderGame();

				// Display
				m_data->front->swapBuffers();
			}
		}
	}
}

void Game::addPlayer(const PlayerType type, const int playerNum, const int numPlayers)
{
	m_players.push_back(new Player(type, playerNum, numPlayers, this));
}

void Game::resetPlayers()
{
	// Set new next list seed
	if (m_settings->useCpuPlayers) {
		m_randomSeedNextList = getRandom(10000);
	} else {
		size_t lowest = m_players[0]->m_onlineId;
		int id = 0;

		// Find lowest ID
		for (size_t i = 0; i < m_players.size(); i++) {
			if (m_players[i]->m_active && m_players[i]->m_onlineId < lowest) {
				lowest = m_players[i]->m_onlineId;
				id = static_cast<int>(i);
			}
		}

		// Set random seed to proposed random seed
		if (m_settings->recording != RecordState::REPLAYING) {
			m_randomSeedNextList = m_players[id]->m_proposedRandomSeed;
		}
	}

	// Reset players
	m_winsString = "";
	for (const auto& player : m_players) {
		player->reset();
		m_winsString += toString(player->m_wins) + "-";
	}
	if (m_winsString.length() > 0) {
		m_winsString = m_winsString.substr(0, m_winsString.length() - 1);
	}
	m_activeAtStart = countActivePlayers();

	m_menuSelect = 0;
	m_timerEndMatch = 0;
	loadMusic();
}

// Check if match has ended.
void Game::checkEnd()
{
	// Wait a little bit before actually goint into the menu, use timerEndMatch for that

	if (m_menuSelect == 0) {
		// In match
		for (const auto& player : m_players) {
			// Check if all players are in win or lose state
			if (player->m_loseWin == LoseWinState::NOWIN && player->m_active) {
				// Still playing
				return;
			}
		}
		if (m_data->windowFocus) {
			m_data->front->musicEvent(MusicCanStop);
		}
		m_targetVolumeNormal = 25;
		m_targetVolumeFever = 25;
		if (m_currentGameStatus == GameStatus::PLAYING || m_settings->useCpuPlayers) {
			m_timerEndMatch++;
			if (m_timerEndMatch == 120) {
				// Go to menu
				m_menuSelect = 2;
				if (m_settings->rankedMatch) {
					m_choiceTimer = 5 * 60;
				} else {
					m_choiceTimer = 30 * 60;
				}
				m_colorTimer = 10 * 60;
				m_rankedTimer = 15 * 60;

				// Online game: reset state if you were a player
				if (m_connected) {
					m_currentGameStatus = GameStatus::IDLE;
				}

				// Release everyone's buttons (otherwise online players start choosing stuff)
				for (const auto& player : m_players) {
					player->m_controls.release();
				}

				// Change channel description
				if (checkLowestId()) {
					sendDescription();
				}
			}
		} else if (m_settings->spectating) {
			// Just wait until next match starts
			m_menuSelect = -1;
			if (m_connected) {
				m_currentGameStatus = GameStatus::REMATCHING;
			}

			// Play next replay
			if (!m_settings->replayPlayList.empty()) {
				m_replayTimer = 3 * 60;
			}
		}
	}
}

int Game::countActivePlayers() const
{
	int n = 0;
	for (const auto& player : m_players) {
		if (player->m_active) {
			n++;
		}
	}
	return n;
}
int Game::countBoundPlayers() const
{
	int n = 0;
	for (const auto& player : m_players) {
		if (!player->m_onlineName.empty()) {
			n++;
		}
	}
	return n;
}

bool Game::checkLowestId() const
{
	// Check if active player has the lowest ID
	if (!m_connected) {
		return false;
	}

	// Only player
	if (const PVS_Channel* ch = m_network->channelManager.getChannel(m_channelName);
		ch == nullptr
		|| m_network->channelManager.getStatus(m_channelName, m_network->getPVS_Peer()) != 1
		|| (m_currentGameStatus == GameStatus::WAITING || m_currentGameStatus == GameStatus::SPECTATING)) {
		return false;
	}

	for (size_t i = 1; i < m_players.size(); i++) {
		// If another player has a higher ID, we are not the lowest.
		if (m_players[i]->m_active && m_players[i]->m_onlineId > m_players[0]->m_onlineId) {
			return false;
		}
	}
	return true;
}

void Game::sendDescription() const
{
	if (!m_connected) {
		return;
	}

	// Player?
	if (const PVS_Channel* ch = m_network->channelManager.getChannel(m_channelName);
		ch != nullptr && m_network->channelManager.getStatus(m_channelName, m_network->getPVS_Peer()) == 1) {
		if (!m_settings->rankedMatch) {
			// Send message about game
			std::string str = "type:friendly|"; // Temp

			std::string ruleString = "rules:Tsu";
			const RuleSetInfo* rs = &(m_settings->ruleSetInfo);
			if (rs->ruleSetType == Rules::TSU_ONLINE)
				ruleString = "rules:Tsu";
			else if (rs->ruleSetType == Rules::FEVER_ONLINE)
				ruleString = "rules:Fever";
			else if (rs->ruleSetType == Rules::FEVER15_ONLINE)
				ruleString = "rules:Fever(15th)";
			else if (rs->ruleSetType == Rules::ENDLESSFEVERVS_ONLINE)
				ruleString = "rules:EndlessFeverVS";
			str += ruleString;

			char other[1000];
			sprintf(other, "|marginTime:%i|targetPoint:%i|requiredChain:%i|initialFeverCount:%i|feverPower:%i|puyoToClear:%i|quickDrop:%i|Nplayers:%i|colors:%i", rs->marginTime, rs->targetPoint, rs->requiredChain, rs->initialFeverCount, rs->feverPower, rs->puyoToClear, (int)rs->quickDrop, rs->numPlayers, rs->colors);
			str += other;
			str += "|current:" + toString(countBoundPlayers());
			if (rs->custom)
				str += "|custom:1";
			else
				str += "|custom:0";
			str += "|channelname:" + m_channelName;
			// Add all peers
			std::string scr;
			for (size_t i = 0; i < m_players.size(); i++) {
				if (m_players[i]->m_onlineName != "")
					scr += m_players[i]->m_onlineName + " - " + toString(m_players[i]->m_wins) + "\n";
			}
			str += "|currentscore:" + scr;
			m_network->requestChannelDescription(m_channelName, str);
		}
	}
}

std::string Game::sendUpdate() const
{
	// 0[spectate]1[currentphase]2[fieldstringnormal]3[fevermode]4[fieldfever]5[fevercount]
	// 6[rng seed]7[fever rng called]8[turns]9[colors]
	// 10[margintimer]11[chain]12[currentFeverChainAmount]13[normal GQ]14[fever GQ]
	// 15[predictedchain]16[allclear]
	std::string str = "spectate|";
	Player* pl = m_players[0];
	str += toString(static_cast<int>(pl->m_currentPhase)) + "|";

	// Get normal field
	str += pl->getNormalField()->getFieldString() + " |";
	str += toString(pl->m_feverMode) + "|";
	str += pl->getFeverField()->getFieldString() + " |";
	str += toString(pl->m_feverGauge.getCount()) + "|";
	str += toString(m_randomSeedNextList) + "|";
	str += toString(pl->m_calledRandomFeverChain) + "|";
	str += toString(pl->m_turns) + "|";
	str += toString(pl->m_colors) + "|";
	str += toString(pl->m_marginTimer) + "|";
	str += toString(pl->m_chain) + "|";
	str += toString(pl->m_currentFeverChainAmount) + "|";
	str += toString(pl->m_normalGarbage.gq) + "|";
	str += toString(pl->m_predictedChain) + "|";
	str += toString(pl->m_allClear) + "|";

	return str;
}

void Game::saveReplay() const
{
	if (m_settings->recording != RecordState::RECORDING) {
		return;
	}

	if (m_activeAtStart == 0) {
		return;
	}

	time_t rawTime;
	tm timeInfo {};

	time(&rawTime);
#ifdef _WIN32
	localtime_s(&timeInfo, &rawTime);
#else
	localtime_r(&rawTime, &timeInfo);
#endif

	// Open file
	std::ofstream outfile;
	std::string replayfolder = "User/Replays/";

	// Create userfolder
	if (!m_settings->spectating) {
		if (m_players.size() > 2) {
			replayfolder += toString(m_players.size()) + "p/";
		} else {
			replayfolder += toString(m_players[1]->m_previousName) + "/";
		}
	} else {
		replayfolder += "Spectator/";
	}
	createFolder(replayfolder);

	// Create datefolder
	char dt[20];
	strftime(dt, 20, "%Y-%m-%d", &timeInfo);
	replayfolder += std::string(dt) + "/";
	createFolder(replayfolder);

	// Rulesname
	std::string rulesname = "TSU_";
	if (m_settings->ruleSetInfo.ruleSetType == Rules::FEVER_ONLINE)
		rulesname = "FEVER_";
	else if (m_settings->ruleSetInfo.ruleSetType == Rules::FEVER15_ONLINE)
		rulesname = "FEVER15_";
	else if (m_settings->ruleSetInfo.ruleSetType == Rules::ENDLESSFEVERVS_ONLINE)
		rulesname = "ENDLESSFEVER_";
	std::string playersname = "_";
	if (m_players.size() > 2)
		playersname = std::string("(") + toString(m_activeAtStart) + "p)_";
	else if (m_players.size() == 2) {
		playersname = toString(m_players[0]->m_previousName) + "_vs_" + toString(m_players[1]->m_previousName) + "_";
	}
	char ft[20];
	strftime(ft, 20, "(%H%M%S)_", &timeInfo);
	std::string scorename;
	if (m_players.size() == 2) {
		scorename = m_winsString;
	}
	std::string filename = rulesname + ft + playersname + scorename;
	outfile.open((replayfolder + filename + ".rvs").c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

	// Construct header
	ReplayHeader rh = {
		{ 'R', 'P', 'V', 'S' },
		kReplayVersion,
		"", // Date
		"", // Time
		m_data->matchTimer,
		static_cast<char>(m_activeAtStart),
		m_randomSeedNextList
	};
	strftime(rh.date, 11, "%Y/%m/%d", &timeInfo);
	strftime(rh.time, 9, "%H:%M:%S", &timeInfo);

	outfile.write(reinterpret_cast<char*>(&rh), sizeof(ReplayHeader));

	ReplayRuleSetHeader rrh = {
		m_settings->ruleSetInfo.ruleSetType,
		m_settings->ruleSetInfo.marginTime,
		m_settings->ruleSetInfo.targetPoint,
		m_settings->ruleSetInfo.requiredChain,
		m_settings->ruleSetInfo.initialFeverCount,
		m_settings->ruleSetInfo.feverPower,
		m_settings->ruleSetInfo.puyoToClear,
		m_settings->ruleSetInfo.numPlayers,
		m_settings->ruleSetInfo.quickDrop,
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};

	outfile.write(reinterpret_cast<char*>(&rrh), sizeof(ReplayRuleSetHeader));

	// Go through all players
	for (auto& player : m_players) {
		// Player header
		int n = player->getPlayerNum();

		ReplayPlayerHeader rph = {
			"", // Name
			static_cast<short>(player->m_wins),
			static_cast<int>(player->m_onlineId),
			static_cast<char>(n),
			player->getPlayerType(),
			player->getCharacter(),
			static_cast<char>(player->m_activeAtStart),
			static_cast<char>(player->m_colors),
			static_cast<int>(player->m_controls.m_recordEvents.size() * sizeof(ControllerEvent)),
			static_cast<int>(player->m_recordMessages.size() * sizeof(MessageEvent)),
			0,
			0
		};

		// Copy name
		std::string name = player->m_previousName.substr(0, 31);
		strcpy(rph.name, name.c_str());

		// Compress move vector
		int movLength = static_cast<int>(player->m_controls.m_recordEvents.size() * sizeof(ControllerEvent));
		auto movComprLength = static_cast<unsigned long>(static_cast<float>(movLength) * 1.1f + 12);
		auto* movcompressed = new unsigned char[movComprLength];
		if (movLength != 0) {
			int z_result = compress(
				movcompressed,
				&movComprLength,
				reinterpret_cast<unsigned char*>(&player->m_controls.m_recordEvents[0]),
				movLength);

			// Check result
			if (z_result != Z_OK) {
				// Stop
				outfile.close();
				remove((replayfolder + filename + ".rvs").c_str());
				return;
			}
			// Update size
			rph.vectorSizeCompressedMovement = static_cast<int>(movComprLength);

			encode("pvs2424", reinterpret_cast<char*>(movcompressed), static_cast<int>(movComprLength));
		} else
			movComprLength = 0;

		// Compress messages
		int mesLength = static_cast<int>(player->m_recordMessages.size() * sizeof(MessageEvent));
		auto mesComprLength = static_cast<unsigned long>((static_cast<float>(mesLength) * 1.1f) + 12);
		auto* mescompressed = new unsigned char[mesComprLength];
		if (mesLength != 0) {
			int z_result = compress(
				mescompressed,
				&mesComprLength,
				reinterpret_cast<unsigned char*>(&player->m_recordMessages[0]),
				mesLength);

			// Check result
			if (z_result != Z_OK) {
				// Stop
				outfile.close();
				remove((replayfolder + filename + ".rvs").c_str());
				return;
			}
			// Update size
			rph.vectorSizeCompressedMessage = static_cast<int>(mesComprLength);
			encode("pvs2424", reinterpret_cast<char*>(mescompressed), static_cast<int>(mesComprLength));
		} else
			mesComprLength = 0;

		// Write compressed
		outfile.write(reinterpret_cast<char*>(&rph), sizeof(ReplayPlayerHeader));
		if (movLength != 0)
			outfile.write(reinterpret_cast<char*>(movcompressed), movComprLength);
		if (mesLength != 0)
			outfile.write(reinterpret_cast<char*>(mescompressed), mesComprLength);

		// Remove compressed
		delete[] movcompressed;
		player->m_controls.m_recordEvents.clear();
		// Remove compressed
		delete[] mescompressed;
		player->m_recordMessages.clear();
	}
	outfile.close();
}

void Game::loadReplay(const std::string& filename)
{
	m_settings->recording = RecordState::REPLAYING;
	m_connected = false;
	m_network = nullptr;

	std::ifstream infile;
	infile.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!infile.is_open()) {
		return;
	}

	// Read header
	ReplayHeader rh {};
	infile.read(reinterpret_cast<char*>(&rh), sizeof(ReplayHeader));

	// Check version
	if (kReplayVersion < rh.versionNumber) {
		return;
	}

	m_currentReplayVersion = rh.versionNumber;

	// Backwards compatibility for randomizer: version 3 introduces the pp2 randomizer
	if (m_currentReplayVersion < 3) {
		m_legacyRng = true;
		m_legacyNuisanceDrop = true;
	} else {
		m_legacyRng = false;
		m_legacyNuisanceDrop = false;
	}

	// Set
	m_randomSeedNextList = rh.randomSeed;

	// Read rules header
	ReplayRuleSetHeader rrh {};
	infile.seekg(sizeof(ReplayHeader), std::ios::beg);
	infile.read(reinterpret_cast<char*>(&rrh), sizeof(ReplayRuleSetHeader));

	m_settings->ruleSetInfo.quickDrop = rrh.quickDrop;
	m_settings->ruleSetInfo.ruleSetType = rrh.ruleSetType;
	m_settings->ruleSetInfo.marginTime = rrh.marginTime;
	m_settings->ruleSetInfo.targetPoint = rrh.targetPoint;
	m_settings->ruleSetInfo.requiredChain = rrh.requiredChain;
	m_settings->ruleSetInfo.initialFeverCount = rrh.initialFeverCount;
	m_settings->ruleSetInfo.feverPower = rrh.feverPower;
	m_settings->ruleSetInfo.puyoToClear = rrh.puyoToClear;
	m_settings->ruleSetInfo.numPlayers = rrh.numPlayers;
	m_settings->numPlayers = rrh.numPlayers;

	// Set rules
	setRules();

	// Set up players
	if (static_cast<int>(m_players.size()) != rrh.numPlayers) {
		while (!m_players.empty()) {
			delete m_players.back();
			m_players.pop_back();
		}
		initPlayers();
	}

	// By now the number of players should be correct
	if (rrh.numPlayers != static_cast<int>(m_players.size()))
		return;

	unsigned int sizePrevious = sizeof(ReplayHeader) + sizeof(ReplayRuleSetHeader);
	for (const auto& player : m_players) {
		// Read player header
		ReplayPlayerHeader rph {};
		infile.seekg(sizePrevious, std::ios::beg);
		infile.read(reinterpret_cast<char*>(&rph), sizeof(ReplayPlayerHeader));

		// Update player
		player->bindPlayer(rph.name, rph.onlineId, true);
		player->m_wins = rph.currentWins;
		player->setPlayerType(rph.playerType);
		player->setCharacter(rph.character);
		player->m_active = rph.active;
		player->m_colors = static_cast<unsigned char>(rph.colors);
		player->m_controls.m_recordEvents.clear();
		player->m_recordMessages.clear();

		// Size of vectors
		unsigned long movSize = rph.vectorSizeMovement;
		unsigned long mesSize = rph.vectorSizeMessage;
		const int movSizeComp = rph.vectorSizeCompressedMovement;
		const int mesSizeComp = rph.vectorSizeCompressedMessage;

		// Prepare vec
		if (movSize > 0) {
			player->m_controls.m_recordEvents.resize(movSize / sizeof(ControllerEvent));
		}
		if (mesSize > 0) {
			player->m_recordMessages.resize(mesSize / sizeof(MessageEvent));
		}

		// Read compressed data
		const auto movCompressed = new unsigned char[movSizeComp];
		const auto mesCompressed = new unsigned char[mesSizeComp];
		if (movSize > 0) {
			infile.seekg(static_cast<std::ifstream::off_type>(sizePrevious + sizeof(ReplayPlayerHeader)), std::ios::beg);
			infile.read(reinterpret_cast<char*>(movCompressed), movSizeComp);
			encode("pvs2424", reinterpret_cast<char*>(movCompressed), movSizeComp);
		}
		if (mesSize > 0) {
			infile.seekg(static_cast<std::ifstream::off_type>(sizePrevious + sizeof(ReplayPlayerHeader) + movSizeComp), std::ios::beg);
			infile.read(reinterpret_cast<char*>(mesCompressed), mesSizeComp);
			encode("pvs2424", reinterpret_cast<char*>(mesCompressed), mesSizeComp);
		}

		// Decompress into vectors
		if (movSize > 0) {
			uncompress(
				reinterpret_cast<unsigned char*>(&player->m_controls.m_recordEvents[0]),
				&movSize,
				movCompressed, // Source buffer - the compressed data
				movSizeComp); // Length of compressed data in bytes
		}
		if (mesSize > 0) {
			uncompress(
				reinterpret_cast<unsigned char*>(&player->m_recordMessages[0]),
				&mesSize,
				mesCompressed, // Source buffer - the compressed data
				mesSizeComp); // Length of compressed data in bytes
		}

		// Delete temporary buffer
		delete[] movCompressed;
		delete[] mesCompressed;

		// Update size
		sizePrevious += sizeof(ReplayPlayerHeader) + movSizeComp + mesSizeComp;
	}
	infile.close();
}

void Game::nextReplay()
{
	if (m_settings->recording != RecordState::REPLAYING) {
		return;
	}

	if (!m_playNext) {
		m_settings->oldReplayPlayList.push_back(m_settings->replayPlayList.front());
		m_settings->replayPlayList.pop_front();
		m_playNext = true;
	}

	if (!m_settings->replayPlayList.empty()) {
		loadReplay(m_settings->replayPlayList.front());
		m_settings->oldReplayPlayList.push_back(m_settings->replayPlayList.front());
		m_settings->replayPlayList.pop_front();
		// Get ready to play
		resetPlayers();
	} else {
		return;
	}

	m_replayTimer = 0;
}

void Game::previousReplay()
{
	if (m_settings->recording != RecordState::REPLAYING) {
		return;
	}

	if (m_playNext) {
		m_settings->replayPlayList.push_front(m_settings->oldReplayPlayList.back());
		m_settings->oldReplayPlayList.pop_back();
		m_playNext = false;
	}

	if (!m_settings->oldReplayPlayList.empty()) {
		m_settings->replayPlayList.push_front(m_settings->oldReplayPlayList.back());
		loadReplay(m_settings->replayPlayList.front());
		m_settings->oldReplayPlayList.pop_back();
		// Get ready to play
		resetPlayers();
	} else {
		return;
	}

	m_replayTimer = 0;
}

void Game::rankedMatch()
{
	if (m_currentGameStatus != GameStatus::IDLE && m_currentGameStatus != GameStatus::WAITING || !m_settings->rankedMatch) {
		return;
	}

	if (m_rankedTimer > 0) {
		m_rankedTimer--;
	}

	if (m_rankedTimer == 0) {
		// The client is expected to have applied you for ranked match
		if (m_rankedState == 0 && m_channelName.empty()) {
			m_network->sendToServer(CHANNEL_MATCH, m_settings->ruleSetInfo.ruleSetType == Rules::TSU_ONLINE ? "find|0" : "find|1");
			// Upon matching again, the timer is reduced
			m_rankedTimer = 5 * 60;
		}
		if (m_rankedState == 2) // Getting ready to close match
		{
			for (const auto& player : m_players) {
				player->unbindPlayer();
			}
			m_rankedState = 3;
			m_rankedTimer = 15 * 60;
		} else
			m_rankedTimer = 15 * 60;
	}
	// Wait for player input
	if (m_rankedState == 3) {
		setStatusText(m_translatableStrings.rankedWaiting.c_str());
		if (!m_players.empty() && m_players[0]->m_controls.m_start == 1) {
			if (!m_newRankedMatchMessage.empty()) {
				// Apply for new match
				m_network->sendToServer(CHANNEL_MATCH, m_newRankedMatchMessage.c_str());
				m_newRankedMatchMessage = "";
			}
			m_rankedState = 0;
			m_rankedTimer = 15 * 60;
		}
	}
}

void Game::changeMusicVolume()
{
	if (!m_data->windowFocus) {
		return;
	}
	if (m_currentVolumeNormal > m_targetVolumeNormal) {
		m_currentVolumeNormal -= 1;
		if (m_currentVolumeNormal < 0)
			m_currentVolumeNormal = 0;
		m_data->front->musicVolume(static_cast<float>(m_currentVolumeNormal) / 100.0f * m_globalVolume, false);
	} else if (m_currentVolumeNormal < m_targetVolumeNormal && m_data->globalTimer % 3 == 0) {
		m_currentVolumeNormal += 1;
		if (m_currentVolumeNormal > 100)
			m_currentVolumeNormal = 100;
		m_data->front->musicVolume(static_cast<float>(m_currentVolumeNormal) / 100.0f * m_globalVolume, false);
	}
	if (m_currentVolumeFever > m_targetVolumeFever) {
		m_currentVolumeFever -= 1;
		if (m_currentVolumeFever < 0)
			m_currentVolumeFever = 0;
		m_data->front->musicVolume(static_cast<float>(m_currentVolumeFever) / 100.0f * m_globalVolume, true);
	} else if (m_currentVolumeFever < m_targetVolumeFever) {
		m_currentVolumeFever += 1;
		if (m_currentVolumeFever > 100)
			m_currentVolumeFever = 100;
		m_data->front->musicVolume(static_cast<float>(m_currentVolumeFever) / 100.0f * m_globalVolume, true);
	}
}

void Game::loadMusic()
{
	if (m_settings->recording == RecordState::REPLAYING && m_backwardsOnce == true) {
		return;
	}

	m_data->front->musicEvent(MusicContinue);
	m_targetVolumeNormal = 100;
	m_targetVolumeFever = 100;

	// Force to set volume
	m_currentVolumeNormal -= 1;
	m_currentVolumeFever -= 1;
}

}
