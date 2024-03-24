#include "Player.h"
#include "../PVS_ENet/PVS_Client.h"
#include "Game.h"
#include "RNG/PuyoRng.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace ppvs {


Player::Player(const PlayerType type, const int playerNum, const int totalPlayers, Game* g)
	: m_feverGauge(g->m_data)
	, m_feverLight(g->m_data)
{
	m_currentGame = g;
	m_data = m_currentGame->m_data;
	m_debug = 0;
	m_debugCounter = 0;

	m_rngNextList = nullptr;
	m_rngFeverChain = nullptr;
	m_rngFeverColor = nullptr;
	m_rngNuisanceDrop = new MersenneTwister();

	// Types of players: human, CPU, network etc
	m_type = type;
	m_playerNum = playerNum;
	m_active = false;
	m_prepareActive = false;
	m_activeAtStart = 0;
	m_rematch = false;
	m_onlineId = 0;
	m_wins = 0;
	m_proposedRandomSeed = 0;
	m_waitForConfirm = 0;
	m_loseConfirm = false;
	m_lastAttacker = nullptr;

	m_forceStatusText = false;

	m_feverMode = false;
	m_feverEnd = false;
	m_calledRandomFeverChain = 0;
	m_bonusEq = false;
	m_hasMoved = false;

	// Default active field: normal field
	m_activeField = &m_fieldNormal;

	// Controller
	m_controls.init(m_playerNum, m_type, m_currentGame->m_settings->recording);

	// Mover
	m_movePuyo.init(m_data);
	m_dropSpeed = 2;

	// Tray
	m_normalTray.init(m_data);
	m_feverTray.init(m_data);

	// Initialize player values
	m_colors = 4;

	// Player 1 abides default character
	m_character = playerNum != 1 ? ARLE : m_currentGame->m_settings->defaultPuyoCharacter;
	m_cpuAi = nullptr;
	m_divider = 2;

	// Give CPU an AI
	if (m_type == CPU) {
		m_cpuAi = new AI(this);
	}

	// TEMP: in online version, other players must inform each-other from their randomSeedFever
	if (m_currentGame->m_settings->useCpuPlayers) {
		initValues(ppvs::getRandom(1000));
		// All player are active
		m_active = true;
	} else {
		initValues(static_cast<int>(m_currentGame->m_randomSeedNextList + m_onlineId));
	}

	// TEMP
	// Field size may be set up by rule set. For now, use standard field size
	m_properties.gridHeight = 28;
	m_properties.gridWidth = 32;
	m_properties.gridX = 6;
	m_properties.gridY = 12 + 3; // Always add 3 extra layers
	m_properties.scaleX = 1; // Local scale: only for local effects (bouncing and stuff)
	m_properties.scaleY = 1;
	m_properties.angle = 0;

	// Initialize objects (place fields, movepuyo, nextpuyo etc)
	playerSetup(m_properties, m_playerNum, totalPlayers);
	m_properties = m_fieldNormal.getProperties();

	// Resize sprite to fieldsize
	m_fieldSprite.setSize(m_fieldNormal.getFieldSize()); // Useless
	m_fieldFeverSprite.setImage(m_data->imgFieldFever);
	m_fieldFeverSprite.setSize(m_fieldNormal.getFieldSize());

	// Set chainword
	m_chainWord = new ChainWord(m_data);
	m_chainWord->setScale(m_globalScale);

	// Initialize movePuyos at least once
	m_movePuyo.prepare(MovePuyoType::DOUBLET, this, 0, 0);

	// Initialize garbage tray
	m_normalTray.align(m_properties.offsetX, m_properties.offsetY - (32) * m_globalScale, m_globalScale);
	m_feverTray.align(m_properties.offsetX, m_properties.offsetY - (32) * m_globalScale, m_globalScale);
	m_feverTray.setVisible(false);

	// Initialize scoreCounter
	m_scoreCounter.init(m_data, m_properties.offsetX, m_fieldNormal.getBottomCoordinates().y + static_cast<float>(kPuyoY) / 4.f * m_globalScale, m_globalScale);

	// Load other sprites
	m_allClearSprite.setImage(m_data->imgAllClear);
	m_allClearSprite.setCenter();
	m_crossSprite.setImage(m_data->imgPuyo);
	m_crossSprite.setSubRect(7 * kPuyoX, 12 * kPuyoY, kPuyoX, kPuyoY);
	m_crossSprite.setCenter(0, 0);
	m_winSprite.setImage(m_data->imgWin);
	m_winSprite.setCenter();
	m_loseSprite.setImage(m_data->imgLose);
	m_loseSprite.setCenter();

	// Set border
	m_borderSprite.setCenter();
	m_borderSprite.setPosition(m_properties.centerX,
		m_properties.centerY / 2);
	m_borderSprite.setScale(1);

	// Set origin of field to bottom of image
	m_fieldSprite.setCenterBottom();
	m_fieldFeverSprite.setCenterBottom();

	// Initialize character animation object
	setCharacter(m_character);
	m_pickingCharacter = false;

	// Color menu objects
	for (int i = 0; i < 9; i++) {
		m_colorMenuBorder[i].setImage(m_data->imgPlayerBorder);
		m_colorMenuBorder[i].setVisible(false);
		m_colorMenuBorder[i].setSubRect(i % 3 * 24, i / 3 * 24, 24, 24);
	}

	for (int i = 0; i < 5; i++) {
		m_spice[i].setImage(m_data->imgSpice);
		m_spice[i].setSubRect(0, i * 50, 138, 50);
		m_spice[i].setVisible(false);
		m_spice[i].setCenter();
	}

	m_spiceSelect = 2;

	// Online
	// Needs a black image to draw over field
	m_overlaySprite.setImage(nullptr);
	m_overlaySprite.setTransparency(0.5f);
	m_overlaySprite.setScale(2 * 192, 336);
	m_overlaySprite.setColor(0, 0, 0);
	m_overlaySprite.setPosition(-192.f / 2.f, -336.f / 4.f);

	m_charHolderSprite.setImage(m_data->imgCharHolder);
	m_charHolderSprite.setCenter();
	m_charHolderSprite.setPosition(
		m_properties.offsetX + static_cast<float>(m_properties.gridWidth * m_properties.gridX) / 2 * m_globalScale,
		m_properties.offsetY + static_cast<float>(m_properties.gridHeight * m_properties.gridY) / 2 * m_globalScale);
	m_charHolderSprite.setVisible(false);
	m_charHolderSprite.setScale(m_globalScale);
	m_currentCharacterSprite.setVisible(true);
	for (auto& i : m_dropSet) {
		i.setVisible(true);
	}
	m_showCharacterTimer = 0;

	m_rematchIcon.setImage(m_data->imgCheckMark);
	m_rematchIcon.setCenter();
	m_rematchIcon.setPosition(
		m_properties.offsetX + static_cast<float>(m_properties.gridWidth * m_properties.gridX) / 2 * m_globalScale,
		m_properties.offsetY + static_cast<float>(m_properties.gridHeight) * (static_cast<float>(m_properties.gridY) / 2 + 3) * m_globalScale);
	m_rematchIcon.setVisible(false);
	m_rematchIcon.setScale(m_globalScale);
	m_rematchIconTimer = 1000;

	// Set text
	// Text messages on screen
	m_statusFont = m_data->front->loadFont("Arial", 14);
	m_statusText = nullptr;

	if (m_currentGame->m_settings->recording != RecordState::REPLAYING) {
		setStatusText(m_currentGame->m_translatableStrings.waitingForPlayer.c_str());
	}
}

void Player::reset()
{
	// Release all controls
	m_controls.release();

	// Set colors
	if (!m_currentGame->m_settings->pickColors && m_currentGame->m_settings->recording != RecordState::REPLAYING
		&& m_currentGame->m_settings->ruleSetInfo.colors >= 3 && m_currentGame->m_settings->ruleSetInfo.colors <= 5)
		m_colors = m_currentGame->m_settings->ruleSetInfo.colors;

	// Reset fields
	m_fieldNormal.clearField();
	m_fieldFever.clearField();
	m_fieldSprite.setVisible(true);
	m_fieldFeverSprite.setVisible(false);
	m_fieldNormal.setTransformScale(1);
	m_fieldFever.setTransformScale(1);

	m_movePuyo.prepare(MovePuyoType::DOUBLET, this, 0, 0);
	m_movePuyo.setVisible(false);

	// Reset fever mode
	m_activeField = &m_fieldNormal;
	m_activeGarbage = &m_normalGarbage;
	m_normalTray.align(m_activeField->getProperties().offsetX, m_activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
	m_normalTray.setDarken(false);
	m_normalTray.update(0);
	m_feverTray.update(0);

	// Reset replay values
	if (m_currentGame->m_settings->recording == RecordState::RECORDING) {
		m_recordMessages.clear();
	}
	m_activeAtStart = 0;

	// Reset others
	// TEMP random value
	if (m_currentGame->m_settings->useCpuPlayers) {
		initValues(ppvs::getRandom(1000));
	} else {
		initValues(int(m_currentGame->m_randomSeedNextList + m_onlineId));
	}

	if (!m_active) {
		return;
	}

	// TEMP immediate reset
	if (m_currentGame->m_settings->pickColors) {
		m_currentPhase = Phase::PICKCOLORS;
	} else {
		m_currentPhase = Phase::GETREADY;
		m_currentGame->m_readyGoObj.prepareAnimation("readygo");
		m_currentGame->m_data->matchTimer = 0;
	}
}

void Player::initValues(int randomSeed)
{
	m_useDropPattern = true;

	// Initialize RNG
	m_randomSeedFever = randomSeed;
	setRandomSeed(m_randomSeedFever, &m_rngFeverChain);
	setRandomSeed(m_randomSeedFever + 1, &m_rngFeverColor);
	m_calledRandomFeverChain = 0;
	m_nextPuyoActive = true;

	// Init nuisance drop pattern RNG
	m_rngNuisanceDrop->init_genrand(randomSeed);
	m_nuisanceList.clear();

	// Other values to initialize
	m_diacute = 0;
	m_playVoice = -1;
	m_stutterTimer = 0;
	m_currentFeverChainAmount = 5;
	m_feverMode = false;
	m_feverEnd = false;
	m_feverColor = 0;
	m_feverColorR = 0;
	m_feverColorG = 0;
	m_feverColorB = 1;
	m_feverGauge.m_seconds = 60 * 15;
	m_feverGauge.setCount(m_currentGame->m_currentRuleSet->m_initialFeverCount);
	m_divider = 2;

	m_poppedChain = false;
	m_garbageTimer = 0;
	m_garbageSpeed = 4.8f;
	m_chainPopSpeed = 25;
	m_garbageEndTime = 100;
	m_puyoBounceEnd = 50;
	m_puyoBounceSpeed = 2;
	m_lastAttacker = nullptr;
	m_gravity = 0.5f;
	m_attackState = NO_ATTACK;
	m_scoreVal = 0;
	m_currentScore = 0;
	m_createPuyo = false;
	m_forgiveGarbage = false;
	m_loseWin = LoseWinState::NOWIN;
	m_chain = 0;
	m_puyosPopped = 0;
	m_totalGroups = 0;
	m_groupR = 0;
	m_groupG = 0;
	m_groupB = 0;
	m_groupY = 0;
	m_groupP = 0;
	m_predictedChain = 0;
	m_point = 0;
	m_bonus = 0;
	m_linkBonus = 0;
	m_allClear = 0;
	m_rememberMaxY = 0;
	m_rememberX = 0;
	m_dropBonus = 0;
	m_foundChain = false;
	m_normalGarbage.cq = 0;
	m_normalGarbage.gq = 0;
	m_normalGarbage.accumulator.clear();
	m_feverGarbage.cq = 0;
	m_feverGarbage.gq = 0;
	m_feverGarbage.accumulator.clear();
	m_activeGarbage = &m_normalGarbage;
	m_eq = 0;
	m_tray = 0;
	m_garbageDropped = 0;
	m_garbageCycle = 0;
	m_hasMoved = false;
	m_destroyPuyosTimer = 0;
	m_marginTimer = 0;
	m_colorMenuTimer = 1;
	m_menuHeight = 24;
	m_pickedColor = false;
	m_bonusEq = false;
	m_debugCounter = 0;

	m_lightTimer = 0;
	m_loseWinTimer = 0;
	m_readyGoTimer = 0;
	m_transitionTimer = 0;
	m_allClearTimer = 0;
	m_transformScale = 1;
	m_feverSuccess = 0;

	// Stop any animation
	m_characterAnimation.prepareAnimation("");

	// Initialize or override values according to ruleset
	m_currentGame->m_currentRuleSet->onInit(this);

	// Add 4 initial 3-colored puyo
	initNextList();
	m_turns = 0;
	m_waitForConfirm = 0;
	m_loseConfirm = false;

	// Set next puyo
	if (m_useDropPattern) {
		m_nextPuyo.update(m_nextList, m_character, 0);
	} else {
		m_nextPuyo.update(m_nextList, ARLE, 0);
	}

	// Idle
	m_currentPhase = Phase::IDLE;

	updateTray();
}

void Player::initNextList()
{
	setRandomSeed(m_currentGame->m_randomSeedNextList, &m_rngNextList);
	m_nextList.clear();

	// nextList needs 3 pairs to start
	addNewColorPair(3);
	addNewColorPair(3);
	checkAllClearStart();
	addNewColorPair(m_colors);
	m_turns = 0;
}

void Player::checkAllClearStart()
{
	if (m_currentGame->m_currentRuleSet->m_allClearStart) {
		return;
	}

	if (m_nextList.empty()) {
		return;
	}

	int firstcolor = m_nextList.front();

	// At this point there should be 4 colors in m_nextlist
	for (const int& next : m_nextList) {
		if (firstcolor != next) {
			return; // Not the same color
		}
	}

	// Cleared all checks: list is filled with same colors
	// Pop last 2 and push back new one
	m_nextList.pop_back();
	m_nextList.pop_back();
	addNewColorPair(3);

	// Try again
	checkAllClearStart();
}


FeSound* Player::loadVoice(const SoundEffectToken token)
{
	return m_currentGame->m_assetManager->loadSound(token, getCharacter());
}

void Player::initVoices()
{
	m_characterVoices.chain[0].setBuffer(loadVoice(SoundEffectToken::charChain1));
	m_characterVoices.chain[1].setBuffer(loadVoice(SoundEffectToken::charChain2));
	m_characterVoices.chain[2].setBuffer(loadVoice(SoundEffectToken::charChain3));
	m_characterVoices.chain[3].setBuffer(loadVoice(SoundEffectToken::charChain4));
	m_characterVoices.chain[4].setBuffer(loadVoice(SoundEffectToken::charChain5));
	m_characterVoices.chain[5].setBuffer(loadVoice(SoundEffectToken::charSpell1));
	m_characterVoices.chain[6].setBuffer(loadVoice(SoundEffectToken::charSpell2));
	m_characterVoices.chain[7].setBuffer(loadVoice(SoundEffectToken::charSpell3));
	m_characterVoices.chain[8].setBuffer(loadVoice(SoundEffectToken::charSpell4));
	m_characterVoices.chain[9].setBuffer(loadVoice(SoundEffectToken::charSpell5));
	m_characterVoices.chain[10].setBuffer(loadVoice(SoundEffectToken::charCounter));
	m_characterVoices.chain[11].setBuffer(loadVoice(SoundEffectToken::charCounterSpell));
	m_characterVoices.damage1.setBuffer(loadVoice(SoundEffectToken::charDamage1));
	m_characterVoices.damage2.setBuffer(loadVoice(SoundEffectToken::charDamage2));
	m_characterVoices.choose.setBuffer(loadVoice(SoundEffectToken::charChoose));
	m_characterVoices.fever.setBuffer(loadVoice(SoundEffectToken::charFever));
	m_characterVoices.feverSuccess.setBuffer(loadVoice(SoundEffectToken::charFeverSuccess));
	m_characterVoices.feverFail.setBuffer(loadVoice(SoundEffectToken::charFeverFail));
	m_characterVoices.lose.setBuffer(loadVoice(SoundEffectToken::charLose));
	m_characterVoices.win.setBuffer(loadVoice(SoundEffectToken::charWin));
}

void Player::playerSetup(FieldProp& properties, const int playerNum, const int playerTotal)
{
	// Right side square setup
	const int width = static_cast<int>(ceil(sqrt(static_cast<double>(playerTotal - 1))));

	if (playerNum == 1) {
		// Player 1
		m_globalScale = 1;
		properties.offsetX = 48;
		properties.offsetY = 84;
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);

		// Set next puyo
		m_nextPuyoOffsetX = properties.offsetX + (static_cast<float>(properties.gridX * properties.gridWidth) * properties.scaleX + 10) * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, true, m_data);

		// Set background
		m_fieldSprite.setImage(m_data->imgField1); // Can also be character related

		// Set up border
		m_borderSprite.setImage(m_data->imgBorder1);

		// Fever gauge
		m_feverGauge.init(-8.f, static_cast<float>(properties.gridHeight * (properties.gridY - 3 - 0)), 1.f, true, m_data);
	} else if (playerNum > 1) {
		// Other players
		m_globalScale = 1.0f / static_cast<float>(width);
		properties.offsetX = 400.f + static_cast<float>((playerNum - 2) % width) * 320.f * m_globalScale - 75.f * m_globalScale * static_cast<float>(width - 1);
		properties.offsetY = 84.f + static_cast<float>((playerNum - 2) / width) * 438.f * m_globalScale - 42.f * m_globalScale * static_cast<float>(width - 1); // NOLINT(bugprone-integer-division)
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);

		// Set next puyo
		m_nextPuyoOffsetX = properties.offsetX - 75 * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, false, m_data);

		// Set background
		m_fieldSprite.setImage(m_data->imgField2); // Can also be character related

		// Set up border
		m_borderSprite.setImage(m_data->imgBorder2);

		// Fever gauge
		m_feverGauge.init(76, static_cast<float>(properties.gridHeight * (properties.gridY - 3 - 0)), 1.f, false, m_data);
	}
}

Player::~Player()
{
	delete m_chainWord;
	delete m_cpuAi;
	// Delete light effects
	while (!m_lightEffect.empty()) {
		delete m_lightEffect.back();
		m_lightEffect.pop_back();
	}

	// Delete other
	while (!m_secondsObj.empty()) {
		delete m_secondsObj.back();
		m_secondsObj.pop_back();
	}

	delete m_statusText;
	delete m_statusFont;
	delete m_rngNuisanceDrop;
}

void Player::setPlayerType(const PlayerType playerType)
{
	m_type = playerType;
	m_controls.init(m_playerNum, m_type, m_currentGame->m_settings->recording);
}

void Player::setRandomSeed(unsigned long seedIn, PuyoRng** randomizer)
{
	delete *randomizer;
	*randomizer = makePuyoRng(m_currentGame->m_legacyRng == true ? "legacy" : "classic", seedIn, 0);
}

// RNG for next puyo list
int Player::getRandom(const int in, PuyoRng* rng)
{
	return rng->next(in);
}

// Set character and initialize all related values (voice, animation, background)
void Player::setCharacter(PuyoCharacter character, bool show)
{
	m_character = character;
	initVoices();
	const PosVectorFloat offset(
		m_activeField->getProperties().centerX * 1,
		m_activeField->getProperties().centerY / 2.0f * 1);
	const std::string currentCharacter = m_currentGame->m_settings->characterSetup[character];
	if (m_currentGame->m_players.size() <= 10)
		m_characterAnimation.init(m_data, offset, 1, m_currentGame->m_assetManager->getAnimationFolder(character));
	else
		m_characterAnimation.init(m_data, offset, 1, "");
	if (m_currentGame->m_settings->useCharacterField)
		setFieldImage(character);

	if (!show)
		return;

	m_currentCharacterSprite.setImage(m_currentGame->m_assetManager->loadImage(ImageToken::imgCharIcon, character));
	m_currentCharacterSprite.setCenter();
	m_currentCharacterSprite.setPosition(m_charHolderSprite.getPosition() + PosVectorFloat(1, 1) - PosVectorFloat(2, 4)); // Correct for shadow
	m_currentCharacterSprite.setVisible(true);
	m_currentCharacterSprite.setScale(m_globalScale);
	m_charHolderSprite.setVisible(true);
	m_showCharacterTimer = 5 * 60;
	for (auto& i : m_dropSet) {
		i.setImage(m_currentGame->m_assetManager->loadImage(ImageToken::imgDropSet));
	}
	setDropSetSprite(static_cast<int>(m_currentCharacterSprite.getX()), static_cast<int>(m_currentCharacterSprite.getY() + 60.f * m_globalScale), m_character);
}

void Player::setFieldImage(PuyoCharacter character)
{
	FeImage* im = m_data->imgCharField[static_cast<unsigned char>(character)];
	if (!im || im->error()) {
		m_fieldSprite.setImage(m_playerNum == 1 ? m_data->imgField1 : m_data->imgField2);
	} else {
		m_fieldSprite.setImage(im);
	}
	if (m_nextPuyo.getOrientation() < 0) {
		m_fieldSprite.setFlipX(true);
	}
}

// Game code
void Player::play()
{
	// Debugging
	if (debugMode == 1) {
		debugString = "";
		debugString += toString(static_cast<int>(m_currentPhase)) + "\n";
		debugString += "garbage: " + toString(m_forgiveGarbage) + "\n";
		if (m_activeGarbage == &m_normalGarbage)
			debugString += "normal: " + toString(m_activeGarbage->gq) + "\n";
		else
			debugString += "fever: " + toString(m_activeGarbage->gq) + "\n";

		if (!m_messages.empty())
			debugString += std::string("mes: ") + m_messages.front()[0] + "\n";
	}

	// Pick character (online)
	if (m_showCharacterTimer > 0) {
		m_showCharacterTimer--;
		m_charHolderSprite.setTransparency(1);
		m_currentCharacterSprite.setTransparency(1);
		for (auto& i : m_dropSet) {
			i.setTransparency(1);
		}
		if (m_showCharacterTimer < 120) {
			m_charHolderSprite.setTransparency(static_cast<float>(m_showCharacterTimer) / 120.0f);
			m_currentCharacterSprite.setTransparency(static_cast<float>(m_showCharacterTimer) / 120.0f);
			for (auto& i : m_dropSet) {
				i.setTransparency(static_cast<float>(m_showCharacterTimer) / 120.0f);
			}
		}
	}
	// Play animation for checkmark
	if (m_rematchIconTimer < 1000) {
		m_rematchIconTimer++;
		m_rematchIcon.setScale(m_globalScale * static_cast<float>(interpolate("elastic", 2, 1, m_rematchIconTimer / 60.0, -5, 2)));
	}
	m_rematchIcon.setVisible(m_rematch);

	// Process message
	if (m_type == ONLINE) {
		processMessage();
	}

	// ===== Global events

	// Animate particles
	m_activeField->animateParticle();

	// Animate chainword
	m_chainWord->move();

	// Animate light effects & secondsobj
	playLightEffect();

	// Do characterAnimation
	m_characterAnimation.playAnimation();

	// Move nextPuyo
	if (static_cast<int>(m_currentPhase) > 0 && m_nextPuyoActive)
		m_nextPuyo.play();

	// Animate garbage trays
	m_normalTray.play();
	m_feverTray.play();

	// Set scorecounter
	setScoreCounter();

	// Fever mode specific
	playFever();

	// Set Margin Time
	if (m_currentPhase != Phase::PICKCOLORS && m_currentPhase != Phase::IDLE) {
		setMarginTimer();
	}

	// =====PHASE -2: PICK COLORS
	if (m_currentPhase == Phase::PICKCOLORS) {
		chooseColor();
	}

	// =====PHASE -1: IDLE
	// Possiblity to enter menu? (maybe outside the player object)

	// =====PHASE 0: GETREADY
	if (m_currentPhase == Phase::GETREADY) {
		getReady();
	}

	// =====PHASE 1: PREPARE
	if (m_currentPhase == Phase::PREPARE) {
		prepare();
	}

	// =====Phase 10: MOVE (global), the movement phase
	m_movePuyo.move();
	if (m_currentPhase == Phase::MOVE) {
		m_nextPuyoActive = true; // Start moving nextpuyo if it doesnt yet
		m_hasMoved = true;
	}

	if (m_currentPhase == Phase::MOVE && m_cpuAi) {
		// Perform AI movement
		cpuMove();
	}

	// =====Phase 20 :CREATEPUYO
	if (m_currentPhase == Phase::CREATEPUYO) {
		// CREATEPUYO phase is dropping a colored puyo pair
		m_activeField->createPuyo();

		// Reset chain number
		m_chain = 0;
		m_predictedChain = 0;

		// Set target point
		if (m_currentGame->m_currentRuleSet->m_marginTime >= 0) {
			m_targetPoint = getTargetFromMargin(m_currentGame->m_currentRuleSet->m_targetPoint, m_currentGame->m_currentRuleSet->m_marginTime, m_marginTimer);
		}
	}

	// ==========Phase 21: DROPPUYO
	if (m_currentPhase == Phase::DROPPUYO) {
		m_activeField->dropPuyo();
	}

	//==========Phase 22: FALLPUYO (global)
	m_activeField->fallPuyo();
	m_activeField->bouncePuyo();
	if (m_currentPhase == Phase::FALLPUYO || m_currentPhase == Phase::FALLGARBAGE) {
		// End phase
		m_activeField->endFallPuyoPhase();
	}

	// ========== Phase 31: SEARCHCHAIN
	if (m_currentPhase == Phase::SEARCHCHAIN) {
		m_activeField->searchChain();
	}

	// ========== Phase 32: DESTROYPUYO
	if (m_currentPhase == Phase::DESTROYPUYO) {
		destroyPuyos();
	}

	// Global phase 32
	m_activeField->popPuyoAnim();

	// ========== Phase33-35: GARBAGE
	if (m_currentPhase == Phase::GARBAGE) {
		garbagePhase();
	}

	// Global: countergarbage, it's outisde garbage phase
	if (m_attackState == COUNTER_ATTACK) {
		counterGarbage();
	}

	// ========== Phase 40: CHECKALLCLEAR
	if (m_currentPhase == Phase::CHECKALLCLEAR) {
		checkAllClear();
	}

	// ========== Phase 41: DROPGARBAGE
	if (m_currentPhase == Phase::DROPGARBAGE) {
		m_activeField->dropGarbage();
	}

	// ========== Phase 43: CHECKLOSER
	if (m_currentPhase == Phase::CHECKLOSER) {
		checkLoser(true);
	}

	//==========Phase 45: WAITGARBAGE - it acts the same as dropGarbage, except it waits for a message
	if (m_currentPhase == Phase::WAITGARBAGE) {
		waitGarbage();
	}

	// ========== Phase 46: WAITCONFIRMGARBAGE
	if (m_currentPhase == Phase::WAITCONFIRMGARBAGE) {
		confirmGarbage();
	}

	// ========== Phase 50: CHECKFEVER
	if (m_currentPhase == Phase::CHECKFEVER) {
		checkFever();
	}

	// ========== Phase 51: PREPAREFEVER
	if (m_currentPhase == Phase::PREPAREFEVER) {
		startFever();
	}

	//==========Phase 52: DROPFEVERCHAIN
	if (m_currentPhase == Phase::DROPFEVER) {
		dropFeverChain();
	}

	//==========Phase 53: CHECKFEVER
	if (m_currentPhase == Phase::CHECKENDFEVER) {
		checkEndFever();
	}

	//==========Phase 54: ENDFEVER
	if (m_currentPhase == Phase::ENDFEVER) {
		endFever();
	}

	//==========Phase 54: CHECKENDFEVERONLINE
	if (m_currentPhase == Phase::CHECKENDFEVERONLINE) {
		checkEndFeverOnline();
	}

	//==========Phase 60: LOST
	if (m_currentPhase == Phase::LOSEDROP) {
		loseGame();
	}

	if (m_currentPhase == Phase::WAITLOSE) {
		waitLose();
	}

	if (m_loseWin == LoseWinState::WIN) {
		m_currentPhase = Phase::IDLE;
		winGame();
	}

	// Play voice (leave at end)
	playVoice();
}

void Player::endPhase()
{
	m_currentPhase = m_currentGame->m_currentRuleSet->endPhase(m_currentPhase, this);
}

// Update nuisance tray
void Player::updateTray(const GarbageCounter* c)
{
	if (!c) {
		// Automatic check of active garbage
		if (m_activeGarbage == &m_normalGarbage) {
			m_normalTray.update(m_normalGarbage.gq + m_normalGarbage.cq);
		} else {
			m_feverTray.update(m_feverGarbage.gq + m_feverGarbage.cq);
			if (m_feverGarbage.gq + m_feverGarbage.cq <= 0) {
				m_normalTray.update(m_normalGarbage.gq + m_normalGarbage.cq);
			}
		}
	} else {
		// Not active, but must still be updated
		if (c == &m_normalGarbage) {
			m_normalTray.update(c->cq + c->gq);
		} else if (c == &m_feverGarbage) {
			m_feverTray.update(c->cq + c->gq);
		}
	}
}

void Player::playGarbageSound()
{
	if (m_chain == 3) {
		m_data->snd.nuisanceHitS.play(m_data);
	} else if (m_chain == 4) {
		m_data->snd.nuisanceHitM.play(m_data);
	} else if (m_chain == 5) {
		m_data->snd.nuisanceHitL.play(m_data);
	} else if (m_chain >= 6) {
		m_data->snd.heavy.play(m_data);
	}
}

void Player::setScoreCounter()
{
	m_scoreCounter.setCounter(m_scoreVal);
}

void Player::setScoreCounterPB()
{
	m_scoreCounter.setPointBonus(m_point, m_bonus);
}

void Player::getReady()
{
	m_charHolderSprite.setVisible(false);
	m_currentCharacterSprite.setVisible(false);

	// Wait until timer hits go
	if (m_readyGoTimer >= 120) {
		if (m_currentGame->m_settings->recording == RecordState::RECORDING) {
			if (m_active) {
				m_activeAtStart = 1;
				m_previousName = m_onlineName;
			} else {
				m_previousName = "";
				m_activeAtStart = 0;
			}
			m_controls.m_recordEvents.clear();
		}
		// Reset controls
		m_controls.release();
		endPhase();
		m_readyGoTimer = 0;
		return;
	}

	m_readyGoTimer++;
}

void Player::chooseColor()
{
	m_currentGame->m_data->matchTimer = 0;

	if (m_colorMenuTimer == 0) {
		return;
	}

	// Add timer
	m_colorMenuTimer++;

	// Set border
	if (m_colorMenuTimer == 2) {
		m_menuHeight = 0;
		for (auto& i : m_colorMenuBorder) {
			i.setVisible(true);
		}
	}

	// Disappear
	if (m_colorMenuTimer == -26) {
		for (auto& i : m_colorMenuBorder) {
			i.setVisible(false);
		}
	}

	// Fade in/out
	if (m_colorMenuTimer < 25 && m_colorMenuTimer != 0) {
		if (m_colorMenuTimer > 0) {
			m_menuHeight = m_colorMenuTimer * 6;
		} else if (m_colorMenuTimer < -25) {
			m_menuHeight = 150 - (m_colorMenuTimer + 50) * 6;
		}

		m_colorMenuBorder[0].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f - static_cast<float>(m_menuHeight)));
		m_colorMenuBorder[1].setPosition((192.f / 2.f) - (88.f - 24.f) - 1, 336.f + (-168.f - static_cast<float>(m_menuHeight)));
		m_colorMenuBorder[2].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f - static_cast<float>(m_menuHeight)));

		m_colorMenuBorder[3].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f - static_cast<float>(m_menuHeight) + 24.f) - 4.f);
		m_colorMenuBorder[4].setPosition((192.f / 2.f) - (88.f - 24.f), 336.f + (-168.f - static_cast<float>(m_menuHeight) + 24.f) - 4.f);
		m_colorMenuBorder[5].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f - static_cast<float>(m_menuHeight) + 24.f) - 4.f);

		m_colorMenuBorder[6].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f + static_cast<float>(m_menuHeight)));
		m_colorMenuBorder[7].setPosition((192.f / 2.f) - (88.f - 24.f) - 1, 336.f + (-168.f + static_cast<float>(m_menuHeight)));
		m_colorMenuBorder[8].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f + static_cast<float>(m_menuHeight)));

		m_colorMenuBorder[1].setScaleX(-(m_colorMenuBorder[0].getX() - m_colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[4].setScaleX(-(m_colorMenuBorder[0].getX() - m_colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[7].setScaleX(-(m_colorMenuBorder[0].getX() - m_colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[1].setScaleY(1);
		m_colorMenuBorder[4].setScaleY(1);
		m_colorMenuBorder[7].setScaleY(1);

		m_colorMenuBorder[3].setScaleY(-(m_colorMenuBorder[0].getY() - m_colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[4].setScaleY(-(m_colorMenuBorder[0].getY() - m_colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[5].setScaleY(-(m_colorMenuBorder[0].getY() - m_colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		m_colorMenuBorder[3].setScaleX(1);
		m_colorMenuBorder[4].setScaleX(1);
		m_colorMenuBorder[5].setScaleX(1);

		m_colorMenuBorder[0].setScale(1);
		m_colorMenuBorder[2].setScale(1);
		m_colorMenuBorder[6].setScale(1);
		m_colorMenuBorder[8].setScale(1);
	}

	// Initialize
	if (m_colorMenuTimer == 25) {
		for (int i = 0; i < 5; i++) {
			m_spice[i].setVisible(true);
			m_spice[i].setTransparency(1);
			m_spice[i].setPosition(192.f / 2.f, 336.f + static_cast<float>(-168 - 125 + 37 + 50 * i));
		}
	}

	// Make choice
	for (int i = 0; i < 5; i++) {
		if (m_spiceSelect == i) {
			m_spice[i].setScale(m_spice[i].getScaleX() + (1 - m_spice[i].getScaleX()) / 2.0f);
			m_spice[i].setColor(255, 255, 255);
		} else {
			m_spice[i].setScale(0.75);
			m_spice[i].setColor(128, 128, 128);
		}
	}

	// Player 1 takes over CPU choice
	m_takeover = false;
	for (auto& player : m_currentGame->m_players) {
		// Find first CPU player
		if (player->m_takeover) // No other takeovers
			break;

		if (m_currentGame->m_players[0]->m_pickedColor && player->getPlayerType() == CPU && !player->m_pickedColor && player == this) {
			m_takeover = true;
			break;
		}
	}

	if (m_takeover) {
		if (m_currentGame->m_settings->swapABConfirm == false) {
			m_controls.m_a = m_currentGame->m_players[0]->m_controls.m_a;
		} else {
			m_controls.m_b = m_currentGame->m_players[0]->m_controls.m_b;
		}
		m_controls.m_down = m_currentGame->m_players[0]->m_controls.m_down;
		m_controls.m_up = m_currentGame->m_players[0]->m_controls.m_up;
	}

	// Select option
	if (m_colorMenuTimer > 0) {
		bool select = false;
		if (m_controls.m_down == 1 && m_spiceSelect < 4) {
			m_spiceSelect++;
			m_data->snd.cursor.play(m_data);
			select = true;
			// Send message
			if (m_currentGame->m_connected)
				m_currentGame->m_network->sendToChannel(CHANNEL_GAME, std::string("s|") + toString(m_spiceSelect), m_currentGame->m_channelName);
		}
		if (m_controls.m_up == 1 && m_spiceSelect > 0) {
			m_spiceSelect--;
			m_data->snd.cursor.play(m_data);
			select = true;
			// Send message
			if (m_currentGame->m_connected && m_type == HUMAN)
				m_currentGame->m_network->sendToChannel(CHANNEL_GAME, std::string("s|") + toString(m_spiceSelect), m_currentGame->m_channelName);
		}

		// Get online message
		if (m_type == ONLINE && !m_messages.empty() && m_messages.front()[0] == 's') {
			sscanf(m_messages.front().c_str(), "s|%i", &m_spiceSelect);
			select = true;
			m_messages.pop_front();
		}

		if (select || m_colorMenuTimer == 25) {
			// Put this in
			if (m_spiceSelect == 0) {
				m_colors = 3;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 1) {
				m_colors = 3;
				m_normalGarbage.cq = 12;
			} else if (m_spiceSelect == 2) {
				m_colors = 4;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 3) {
				m_colors = 5;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 4) {
				m_colors = 5;
				m_normalGarbage.cq = 12;
			}
			updateTray();
		}

		// Automatic choice
		if (m_currentGame->m_colorTimer == 1 && m_colorMenuTimer > 25 && m_pickedColor == false) {
			if (m_currentGame->m_settings->swapABConfirm == false) {
				m_controls.m_a = 1;
			} else {
				m_controls.m_b = 1;
			}
		}

		// Make choice
		if ((m_controls.m_a == 1 && m_currentGame->m_settings->swapABConfirm == false && m_colorMenuTimer > 25) || ((m_controls.m_b == 1 && m_currentGame->m_settings->swapABConfirm == true && m_colorMenuTimer > 25))) {
			m_normalGarbage.gq += m_normalGarbage.cq;
			m_normalGarbage.cq = 0;
			m_data->snd.decide.play(m_data);
			if (m_currentGame->m_settings->swapABConfirm == false) {
				m_controls.m_a++;
			} else {
				m_controls.m_b++;
			}
			if (m_takeover) {
				if (m_currentGame->m_settings->swapABConfirm == false) {
					m_currentGame->m_players[0]->m_controls.m_a++;
				} else {
					m_currentGame->m_players[0]->m_controls.m_b++;
				}
			}
			m_currentGame->m_colorTimer = 0;
			m_colorMenuTimer = -50;
			m_pickedColor = true;
			for (auto& i : m_spice) {
				i.setVisible(false);
			}

			// Re-init colors
			initNextList();

			// Reset takeover if necessary
			m_takeover = false;

			// Send message
			if (m_currentGame->m_connected && m_type == HUMAN) {
				m_currentGame->m_network->sendToChannel(CHANNEL_GAME, std::string("c|") + toString(m_spiceSelect), m_currentGame->m_channelName);
			}
		}
		// Get choice from online
		if (m_type == ONLINE && !m_messages.empty() && m_messages.front()[0] == 'c') {
			sscanf(m_messages.front().c_str(), "c|%i", &m_spiceSelect);
			if (m_spiceSelect == 0) {
				m_colors = 3;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 1) {
				m_colors = 3;
				m_normalGarbage.cq = 12;
			} else if (m_spiceSelect == 2) {
				m_colors = 4;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 3) {
				m_colors = 5;
				m_normalGarbage.cq = 0;
			} else if (m_spiceSelect == 4) {
				m_colors = 5;
				m_normalGarbage.cq = 12;
			}
			updateTray();

			m_messages.pop_front();
			m_normalGarbage.gq += m_normalGarbage.cq;
			m_normalGarbage.cq = 0;
			m_data->snd.decide.play(m_data);
			m_colorMenuTimer = -50;
			m_pickedColor = true;

			for (auto& i : m_spice) {
				i.setVisible(false);
			}

			// Re-init colors
			initNextList();
		}
	}

	// Check if everyone picked colors
	if (m_colorMenuTimer == -2) {
		bool start = true;
		for (auto& player : m_currentGame->m_players) {
			if (!player->m_active && !m_currentGame->m_settings->useCpuPlayers) {
				continue;
			}
			if (!player->m_pickedColor || player->m_colorMenuTimer < -2) {
				start = false;
				break;
			}
		}

		if (start) {
			m_colorMenuTimer = 0;
			for (auto& player : m_currentGame->m_players) {
				if (!player->m_active && !m_currentGame->m_settings->useCpuPlayers)
					continue;
				player->m_currentPhase = Phase::GETREADY;
			}
			m_currentGame->m_readyGoObj.prepareAnimation("readygo");
			m_currentGame->m_data->matchTimer = 0;
		}
	}
	// Fade out with flicker
	if (m_colorMenuTimer < 0) {
		m_spice[m_spiceSelect].setVisible(false);
	}
	if (m_colorMenuTimer < -20) {
		m_spice[m_spiceSelect].setVisible(true);
		m_spice[m_spiceSelect].setTransparency(static_cast<float>(interpolate("linear", 1, 0, (m_colorMenuTimer + 50) / 30.0, 0, 0)));
	}
}

void Player::prepare()
{
	// Read colors and pop front after reading
	const int color1 = *m_nextList.begin();
	const int color2 = *(m_nextList.begin() + 1);
	addNewColorPair(m_colors);
	popColor();

	MovePuyoType mpt = MovePuyoType::DOUBLET;
	if (m_useDropPattern) {
		mpt = getFromDropPattern(m_character, m_turns);
	}
	m_movePuyo.prepare(mpt, this, color1, color2);

	if (m_cpuAi) {
		// Determine best chain
		m_cpuAi->prepare(mpt, color1, color2);
		m_cpuAi->m_pinch = m_activeGarbage->gq > 0 ? true : false;
		m_cpuAi->findLargest();
	}

	// Release controls for ONLINE player
	if (m_type == ONLINE) {
		m_controls.release();
	}

	endPhase();
	m_poppedChain = false;
	m_createPuyo = false;
	m_forgiveGarbage = false;
}

void Player::cpuMove()
{
	// Get current rotation
	if (m_movePuyo.getType() == MovePuyoType::BIG) {
		if (m_movePuyo.getColorBig() != m_cpuAi->m_bestRot) {
			m_controls.m_a = m_cpuAi->m_timer % 12;
		}
	} else {
		if (static_cast<int>(m_movePuyo.getRotation()) != m_cpuAi->m_bestRot) {
			m_controls.m_a = m_cpuAi->m_timer % 12;
		}
	}

	if (m_cpuAi->m_timer > 30) {
		m_controls.m_down++;
	}

	m_cpuAi->m_timer++;

	if (m_movePuyo.getPosX1() == m_cpuAi->m_bestPos) {
		m_controls.m_a = 0;
		m_controls.m_left = 0;
		m_controls.m_right = 0;
		return;
	}

	if (m_movePuyo.getPosX1() < m_cpuAi->m_bestPos && m_cpuAi->m_timer > 10) {
		m_controls.m_right++;
	}

	if (m_movePuyo.getPosX1() > m_cpuAi->m_bestPos && m_cpuAi->m_timer > 10) {
		m_controls.m_left++;
	}

	if (m_controls.m_left > 0 && m_controls.m_right > 0) {
		m_controls.m_left = 0;
		m_controls.m_right = 0;
	}
}

void Player::destroyPuyos()
{
	// Phase 31
	m_destroyPuyosTimer++;
	if (m_destroyPuyosTimer == m_chainPopSpeed) {
		// Create "XX Chain" word
		const FieldProp prop = m_activeField->getProperties();
		m_chainWord->showAt(
			static_cast<float>(prop.gridWidth * m_rememberX) * prop.scaleX,
			static_cast<float>(prop.gridHeight * (prop.gridY - 3 - (m_rememberMaxY + 3))) * prop.scaleY, m_chain);

		// Play sound
		m_data->snd.chain[min(m_chain - 1, 6)].play(m_data);
	}

	if (m_destroyPuyosTimer == m_chainPopSpeed) {
		// Set EQ
		if (m_chain >= m_currentGame->m_currentRuleSet->m_requiredChain) {
			float power = 1;
			int div = 2;
			if (m_feverMode) {
				power = m_currentGame->m_currentRuleSet->m_feverPower;
			}
			if (m_currentGame->m_players.size() > 1) {
				div = max(2, m_divider);
			}

			m_eq = static_cast<int>(static_cast<float>(m_currentScore / m_targetPoint) * power * 3.f / static_cast<float>(div + 1)); // NOLINT(bugprone-integer-division)
			m_currentScore -= m_targetPoint * (m_currentScore / m_targetPoint);

			if (m_bonusEq) {
				m_eq += 1;
				m_bonusEq = false;
			}
		} else {
			m_currentScore = 0;
		}
		endPhase();
	}
}

// Light effect & secondsobj
void Player::playLightEffect()
{
	for (auto& i : m_lightEffect) {
		i->setTimer(m_garbageSpeed);
	}

	// Check if any should be removed
	for (size_t i = 0; i < m_lightEffect.size(); i++) {
		if (m_lightEffect[i]->getTimer() > 300) {
			delete m_lightEffect[i];
			m_lightEffect.erase(std::remove(m_lightEffect.begin(), m_lightEffect.end(), m_lightEffect[i]), m_lightEffect.end());
		}
	}
	// Play feverlight
	m_feverLight.setTimer(0.1f);

	// Update fevergauge
	m_feverGauge.update();

	// Secondsobj
	for (auto& i : m_secondsObj) {
		i->move();
	}

	// Check if any should be removed
	for (size_t i = 0; i < m_secondsObj.size(); i++) {
		if (m_secondsObj[i]->getTimer() > 50) {
			delete m_secondsObj[i];
			m_secondsObj.erase(std::remove(m_secondsObj.begin(), m_secondsObj.end(), m_secondsObj[i]), m_secondsObj.end());
		}
	}
}

void Player::addFeverCount()
{
	if (m_feverGauge.getCount() == 7) {
		return;
	}

	int dir = m_nextPuyo.getOrientation();
	PosVectorFloat startpv, middlepv, endpv;
	startpv = PosVectorFloat(m_properties.offsetX + (192.f / 2.f) * m_properties.scaleX * getGlobalScale(), m_properties.offsetY);
	endpv = PosVectorFloat(m_nextPuyoOffsetX + (m_feverGauge.getPos().x) * m_nextPuyoScale, m_nextPuyoOffsetY + (m_feverGauge.getPos().y) * m_nextPuyoScale);
	middlepv.x = startpv.x + static_cast<float>(dir * kPuyoX) * 1.5f;
	middlepv.y = startpv.y - static_cast<float>(kPuyoY) * 1.5f;

	m_feverLight.init(startpv, middlepv, endpv);
	m_feverGauge.addCount();
}

// Total garbage including fever garbage
int Player::getGarbageSum() const
{
	return m_normalGarbage.cq + m_normalGarbage.gq + m_feverGarbage.cq + m_feverGarbage.gq;
}

// Total attack amount
int Player::getAttackSum() const
{
	const int attack = m_eq - max(getGarbageSum(), 0); //----> do not count negative GQ
	return attack > 0 ? attack : 0;
}

void Player::addAttacker(GarbageCounter* gc, Player* pl) const
{
	if (const auto it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl); it != gc->accumulator.end()) {
		return;
	}
	gc->accumulator.push_back(pl);
}

void Player::removeAttacker(GarbageCounter* gc, Player* pl)
{
	// Found element
	if (const auto it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl); it != gc->accumulator.end()) {
		gc->accumulator.erase(it);
		m_lastAttacker = pl;
	}
}

// Check if any attackers left in accumulator
void Player::checkAnyAttacker(GarbageCounter* gc)
{
	// Loop through attackers and check their state to be sure
	std::vector<Player*> remove;
	for (auto& i : gc->accumulator) {
		// Player still alive?
		if (i->m_loseWin != LoseWinState::NOWIN || !i->m_active)
			remove.push_back(i);
	}

	while (!remove.empty()) {
		removeAttacker(gc, remove.back());
		remove.pop_back();
	}

	if (!gc->accumulator.empty()) {
		return;
	}

	// If none found, add CQ to GQ
	addToGQ(gc);
}

void Player::checkFeverGarbage()
{
	// Negative normal garbage during fever?!
	return;
}

void Player::addToGQ(GarbageCounter* gc)
{
	if (gc->accumulator.empty()) {
		gc->gq += gc->cq;
		gc->cq = 0;
	}
}

void Player::resetNuisanceDropPattern()
{
	const int fieldWidth = m_properties.gridX;
	m_nuisanceList.clear();
	for (int i = 0; i < fieldWidth; ++i) {
		m_nuisanceList.push_back(i);
	}
}

int Player::nuisanceDropPattern()
{
	// Refill if array is empty
	if (m_nuisanceList.empty()) {
		resetNuisanceDropPattern();
	}

	// Pick an index
	double rand = m_rngNuisanceDrop->genrand_real1();
	const int randomIndex = static_cast<int>(static_cast<double>(m_nuisanceList.size()) * rand);

	// Swap that index with the end and pop
	const int endValue = m_nuisanceList[m_nuisanceList.size() - 1];
	const int value = m_nuisanceList[randomIndex];
	m_nuisanceList[randomIndex] = endValue;
	m_nuisanceList.pop_back();

	return value;
}

void Player::startGarbage()
{
	m_garbageTimer = 0;

	// Calculate DEF (CQ+GQ+sum(EQ))
	int totalEQ = 0;
	for (size_t i = 0; i < m_currentGame->m_players.size(); i++) {
		// Sum opponents attack
		if (m_currentGame->m_players[i] != this)
			totalEQ += m_currentGame->m_players[i]->getAttackSum();
	}
	const int DEF = getGarbageSum() + totalEQ;

	// Set targets on the first run of startgarbage phase
	for (auto& player : m_currentGame->m_players) {
		if (player != this && m_chain == 1) {
			m_targetGarbage[player] = player->m_activeGarbage;
		}
	}

	// Attack or defend?
	if (DEF <= 0 && m_eq > 0) {
		m_attackState = ATTACK;
		for (auto& player : m_currentGame->m_players) {
			if (player != this) {
				// Add any negative GQ
				if (m_normalGarbage.gq + m_feverGarbage.gq < 0) {
					m_eq += -1 * (m_normalGarbage.gq + m_feverGarbage.gq);
					m_normalGarbage.gq = 0;
					m_feverGarbage.gq = 0;
					updateTray();
				}

				// Create light effect animation for each opponent
				PosVectorFloat startPv, middlePv, endPv;
				startPv = m_activeField->getGlobalCoordinates(m_rememberX, m_rememberMaxY);
				endPv = player->m_activeField->getTopCoordinates(-1);

				// Calculate a middle
				const int dir = m_nextPuyo.getOrientation();
				middlePv.x = startPv.x - static_cast<float>(dir) * kPuyoX * 3.f;
				middlePv.y = startPv.y - kPuyoY * 3;

				m_lightEffect.push_back(new LightEffect(m_data, startPv, middlePv, endPv));

				// Add self to garbage accumulator
				player->addAttacker(m_targetGarbage[player], this);
			}
		}
	} else if (DEF > 0) {
		// Calculate COUNTER = CQ+GQ+sum(EQ) - EQ
		if (const int COUNTER = getGarbageSum() + totalEQ - m_eq; COUNTER >= 0) {
			// Normal defense
			m_attackState = DEFEND;

			// Remove self from opponent garbage accumulator (if present)
			for (auto& player : m_currentGame->m_players) {
				if (player == this)
					continue;
				player->removeAttacker(m_targetGarbage[player], this);
				player->checkAnyAttacker(m_targetGarbage[player]);
			}
		} else {
			// Countering defense
			// Wait for defense to end before making next move
			m_attackState = COUNTER_DEFEND;

			// Counter attack voice
			if (m_playVoice > 4 && m_chain >= 3 && m_currentGame->m_currentRuleSet->m_voicePatternFever == true) {
				m_playVoice = 11;
			}
			if (m_playVoice <= 4 && m_chain >= 3 && m_currentGame->m_currentRuleSet->m_voicePatternFever == true) {
				m_playVoice = 10;
			}
		}

		// Create light effect for defense
		const PosVectorFloat startPv = m_activeField->getGlobalCoordinates(m_rememberX, m_rememberMaxY);
		const PosVectorFloat endPv = m_activeField->getTopCoordinates(-1);

		// Calculate a middle
		const int dir = m_nextPuyo.getOrientation();
		PosVectorFloat middlePv;
		middlePv.x = startPv.x - static_cast<float>(dir) * kPuyoX * 3.f;
		middlePv.y = startPv.y - kPuyoY * 3;

		m_lightEffect.push_back(new LightEffect(m_data, startPv, middlePv, endPv));
	} else {
		m_attackState = NO_ATTACK;
	}
}

void Player::garbagePhase()
{
	// Phase 33
	// Stop chaining
	if (m_currentGame->m_stopChaining) {
		return;
	}

	if (m_garbageTimer == 0) { // NOLINT(clang-diagnostic-float-equal)
		m_hasMoved = false;

		// Trigger voice
		if (m_destroyPuyosTimer == m_chainPopSpeed) {
			prepareVoice(m_chain, m_predictedChain);
		}

		startGarbage();

		// Trigger animation
		if (m_destroyPuyosTimer == m_chainPopSpeed && m_playVoice >= 5) {
			if (m_playVoice < 10)
				m_characterAnimation.prepareAnimation(std::string("spell") + toString(m_playVoice - 4));
			else if (m_playVoice == 11)
				m_characterAnimation.prepareAnimation(std::string("counter"));
		}
	}

	// Change garbageTimer
	m_garbageTimer += m_garbageSpeed;

	if (m_garbageTimer > static_cast<float>(m_garbageEndTime)) {
		endGarbage();
		m_garbageTimer = 0;
		endPhase();
	}
}

void Player::counterGarbage()
{
	// Almost the same as normal garbage phase, except no endphase
	m_garbageTimer += m_garbageSpeed;
	if (m_garbageTimer > static_cast<float>(m_garbageEndTime)) {
		endGarbage();
		m_garbageTimer = 0;
	}
}

void Player::endGarbage()
{
	// Play sound
	if (m_eq > 0 && !m_lightEffect.empty()) {
		m_data->snd.hit.play(m_data);
	}

	// Attack animation ends: check if any garbage sneaked past after all
	if (m_attackState == ATTACK || m_attackState == COUNTER_ATTACK) {
		// Bad prediction of attack: there is garbage on your side after all and must be defended
		if (getGarbageSum() > 0 && (m_normalGarbage.gq > 0 || m_feverGarbage.gq > 0)) {
			// Defend
			m_activeGarbage->gq -= m_eq;
			// Push defense through normal GQ if necessary?
			// Be careful here! don't let normal.GQ go negative
			if (m_activeGarbage == &m_feverGarbage && m_normalGarbage.gq > 0 && m_activeGarbage->gq < 0) {
				m_normalGarbage.gq += m_feverGarbage.gq;
				m_feverGarbage.gq = 0;
				if (m_normalGarbage.gq < 0) {
					// Don't let normal GQ go negative!
					m_feverGarbage.gq += m_normalGarbage.gq;
					m_normalGarbage.gq = 0;
				}
				updateTray(&m_normalGarbage);
			}
			updateTray();
			// This part is different from real defense
			if (getGarbageSum() >= 0) {
				// Garbage left: all is OK, defend ends.
				m_eq = 0;
				m_attackState = NO_ATTACK;
			} else {
				// Garbage is countered: put leftover back in EQ and continue attack, do not create counterattack animation!
				m_eq = -1 * (getGarbageSum());
				m_normalGarbage.gq = 0;
				m_normalGarbage.cq = 0;
				m_feverGarbage.gq = 0;
				m_feverGarbage.cq = 0;
				updateTray();
			}
		}
	}

	// Normal Attack
	// Add EQ to opponents CQ
	if (m_attackState == ATTACK || m_attackState == COUNTER_ATTACK) {
		for (auto& player : m_currentGame->m_players) {
			if (player == this)
				continue;

			m_targetGarbage[player]->cq += m_eq;
			player->updateTray(m_targetGarbage[player]);
			playGarbageSound();
			// End of attack
			if (m_chain == m_predictedChain || (m_attackState == COUNTER_ATTACK && m_hasMoved)) {
				// Remove as accumulator
				player->removeAttacker(m_targetGarbage[player], this);

				// Force player to check if zero (there should always be a last player)
				player->checkAnyAttacker(m_targetGarbage[player]);
			}
			player->checkFeverGarbage();
		}

		// End of attack
		m_currentGame->m_currentRuleSet->onAttack(this);
		m_attackState = NO_ATTACK;
	}

	// Defense end
	if (m_attackState == DEFEND || m_attackState == COUNTER_DEFEND) {
		m_activeGarbage->gq -= m_eq;

		// Push defense through normal GQ if necessary?
		// Be careful here! don't let normal.GQ go negative
		if (m_activeGarbage == &m_feverGarbage && m_normalGarbage.gq > 0 && m_activeGarbage->gq < 0) {
			m_normalGarbage.gq += m_feverGarbage.gq;
			m_feverGarbage.gq = 0;
			if (m_normalGarbage.gq < 0) {
				// Don't let normal GQ go negative!
				m_feverGarbage.gq += m_normalGarbage.gq;
				m_normalGarbage.gq = 0;
			}
			updateTray(&m_normalGarbage);
		}
		updateTray();

		// Online players will not offset (?)
		if (m_type != ONLINE) {
			m_currentGame->m_currentRuleSet->onOffset(this);
		}

		// Send message
		if (m_currentGame->m_connected && m_type == HUMAN) {
			m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "fo", m_currentGame->m_channelName);
		}

		// THERE IS NO SUCH THING AS DEFENSE AND COUNTERDEFENSE
		// Always check if defense can be countered at end!!
		m_attackState = COUNTER_DEFEND;
	}

	// Counter defense
	if (m_attackState == COUNTER_DEFEND) {
		// Calculate DEF again
		int totalEQ = 0;
		for (const auto& player : m_currentGame->m_players) {
			if (player == this)
				continue;

			// Sum opponents EQ
			totalEQ += player->getAttackSum();
		}

		if (const int DEF = getGarbageSum() + totalEQ; DEF >= 0) {
			// Ineffective counter -> just end counter
			m_attackState = NO_ATTACK;
		} else {
			// Turn into attack
			m_attackState = COUNTER_ATTACK;
			for (auto& player : m_currentGame->m_players) {
				if (player == this)
					continue;

				if (getGarbageSum() < 0) // No need to check again?
				{
					m_eq = -1 * (getGarbageSum());
					m_normalGarbage.gq = 0;
					m_normalGarbage.cq = 0;
					m_feverGarbage.gq = 0;
					m_feverGarbage.cq = 0;
					updateTray();
				}

				// Create new light effect animation for each opponent
				PosVectorFloat startPv, middlePv, endPv;
				startPv = m_activeField->getTopCoordinates(-1);
				endPv = player->m_activeField->getTopCoordinates(-1);

				// Calculate a middle
				const int dir = m_nextPuyo.getOrientation();
				middlePv.x = startPv.x - static_cast<float>(dir) * kPuyoX * 3;
				middlePv.y = startPv.y - kPuyoY * 3;
				m_lightEffect.push_back(new LightEffect(m_data, startPv, middlePv, endPv));

				// Add self to garbage accumulator
				player->addAttacker(m_targetGarbage[player], this);
			}
		}
	}

	// Reset EQ
	if (m_attackState != COUNTER_ATTACK) {
		m_eq = 0;
	}
}

// Adds 2 new colors to nextlist and update nextpuyo
void Player::addNewColorPair(int n)
{
	int color1 = this->getRandom(n, m_rngNextList);
	int color2 = this->getRandom(n, m_rngNextList);

	// Perform TGM randomizer algorithm?
	if (m_currentGame->m_legacyRng) {
		color1 = TGMR(color1, n);
		color2 = TGMR(color2, n);
	}

	m_nextList.push_back(color1);

	// Regenerate color 2 if type is quadruple and colors are the same
	if (m_useDropPattern) {
		while (color1 == color2 && getFromDropPattern(m_character, m_turns + 3) == MovePuyoType::QUADRUPLET) {
			color2 = this->getRandom(n, m_rngNextList);
		}
	}
	m_nextList.push_back(color2);

	// Update nextPuyo
	m_nextPuyo.update(m_nextList, m_useDropPattern ? m_character : ARLE, m_turns);
}

// Clear front of deque
void Player::popColor()
{
	if (m_nextList.size() <= 6) {
		return;
	}
	m_nextList.pop_front();
	m_nextList.pop_front();
}

// Check if color exists in last generated colors
int Player::TGMR(int color, int n)
{
	// Do not apply TGMR if nextlist on the first 4 colors
	if (m_nextList.size() >= 4) {
		for (auto i = m_nextList.begin(); i != m_nextList.begin() + 4; ++i) {
			if (*i == color) {
				color = this->getRandom(n, m_rngNextList);
				break;
			}
		}
	}

	return color;
}

void Player::checkAllClear()
{
	// Check if you're the winner here
	checkWinner();

	if (m_activeField->count() == 0) {
		// Play sound
		m_data->snd.allClearDrop.play(m_data);

		// Do something according to rules
		m_currentGame->m_currentRuleSet->onAllClear(this);
	}

	endPhase();
}

void Player::checkLoser(bool endphase)
{
	// Check if player places puyo on spawnpoint
	// Doesn't necessarily mean losing, but the phase will change according to rules
	PosVectorInt spawn = m_movePuyo.getSpawnPoint();
	bool lose2 = false;
	bool lose = m_activeField->isPuyo(spawn.x, spawn.y);

	// Is there a 2nd spawnpoint?
	if (m_currentGame->m_currentRuleSet->m_doubleSpawn) {
		lose2 = m_activeField->isPuyo(spawn.x + 1, spawn.y);
	}

	if (lose || lose2) {
		// Player loses
		if (m_type != ONLINE) {
			setLose();
			// Online game: send message to others
			if (m_currentGame->m_connected) {
				m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "l", m_currentGame->m_channelName.c_str());
			}
		} else {
			// Wait for message to arrive
			m_currentPhase = Phase::WAITLOSE;
		}
	} else if (endphase) {
		endPhase();
	}
}

void Player::checkWinner()
{
	// Player wins if all others have lost
	int Nlosers = 0;
	for (size_t i = 0; i < m_currentGame->m_players.size(); i++) {
		if (m_currentGame->m_players[i] != this && m_currentGame->m_players[i]->m_loseWin == LoseWinState::LOSE) {
			if (!m_currentGame->m_players[i]->m_active)
				continue;
			Nlosers++;
		}
	}

	// A winner is you
	if (m_loseWin == LoseWinState::NOWIN && Nlosers == m_currentGame->countActivePlayers() - 1 && Nlosers != 0 && m_active) {
		// Save replay
		m_currentGame->saveReplay();
		m_wins++;

		m_currentGame->m_currentRuleSet->onWin(this);
		m_characterAnimation.prepareAnimation("win");

		// Ranked match: prepare to disconnect
		prepareDisconnect();
	}

	// Only player left after everyone else left the game
	if (m_currentGame->countActivePlayers() == 1 && m_currentGame->m_players.size() > 1 && m_active) {
		m_currentGame->m_currentRuleSet->onWin(this);
		m_characterAnimation.prepareAnimation("win");
	}
}

void Player::setLose()
{
	m_currentGame->m_currentRuleSet->onLose(this);

	// Play lose sound sound
	int activeplayers = 0;
	for (const auto& player : m_currentGame->m_players) {
		// Check if all players are in win or lose state
		if (player->m_loseWin != LoseWinState::NOWIN)
			continue;

		activeplayers++;
	}

	if (m_currentGame->m_players.size() == 1 || activeplayers > 1 && !m_currentGame->m_players.empty()) {
		m_data->snd.lose.play(m_data);
	}
}

void Player::loseGame()
{
	// Force others to check if winner
	// Unless he's in the middle of chaining
	if (m_loseWinTimer == 0) {
		for (auto& player : m_currentGame->m_players) {
			if (player != this) {
				if (!(static_cast<int>(player->m_currentPhase) > 10 && static_cast<int>(player->m_currentPhase) < 40))
					player->checkWinner();
			}
		}
	}

	// Drop puyos
	m_activeField->loseDrop();

	// Increment losetimer
	m_loseWinTimer++;

	if (m_loseWinTimer == 60) {
		m_characterVoices.lose.play(m_data);
		m_characterAnimation.prepareAnimation("lose");
	}

	// Online: wait for confirm message from everyone
	if (m_loseWin == LoseWinState::LOSEWAIT) {
		// Count confirmed
		int count = 0;
		for (const auto& player : m_currentGame->m_players) {
			if (player != this && player->m_active && player->m_loseConfirm)
				count++;
		}
		// Everybody confirmed your loss-> your state goes to lose
		if (count == m_currentGame->countActivePlayers() - 1) {
			m_loseWin = LoseWinState::LOSE;

			// Again, check anyone else if winner
			for (const auto& player : m_currentGame->m_players) {
				if (player == this) {
					continue;
				}

				if (static_cast<int>(player->m_currentPhase) > 10 && static_cast<int>(player->m_currentPhase) < 40) {
					continue;
				}

				player->checkWinner();
			}

			// Ranked match: report loss
			if (m_currentGame->m_settings->rankedMatch && m_type == HUMAN && m_currentGame->m_connected) {
				m_currentGame->m_network->sendToServer(9, "score");
				// Max wins reached
				prepareDisconnect();
			}
		}
	}
}

void Player::winGame()
{
	m_loseWinTimer++;
	if (m_loseWinTimer == 120) {
		m_characterVoices.win.play(m_data);
	}
}

void Player::checkFever()
{
	// Check if fevercount == 7
	if (m_feverGauge.getCount() == 7 && !m_feverMode) {
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);

		// Clear field
		m_fieldFever.clearField();

		// All clear bonus: add feverchainamount and add time
		if (m_allClear == 1) {
			m_allClear = 0;
			m_allClearTimer = 1;
			m_currentFeverChainAmount += 2;
			if (m_feverGauge.m_seconds < 60 * m_feverGauge.m_maxSeconds) {
				m_feverGauge.addTime(5 * 60);
				showSecondsObj(5 * 60);
			}
		}
	}

	// Wait until light has reached fevergauge
	if (m_feverGauge.getCount() == 7 && m_feverGauge.fullGauge()) {
		m_feverMode = true;
		m_feverEnd = false;
		endPhase();
	} else if (m_feverGauge.getCount() != 7) {
		// No fever mode
		// All clear bonus: drop a 4 chain
		if (m_allClear == 1) {
			m_activeField->dropField(getFeverChain(getRandom(m_currentGame->m_currentRuleSet->m_nFeverChains, m_rngNextList), m_colors, 4, getRandom(m_colors, m_rngFeverColor)));
			m_forgiveGarbage = true;
			m_allClearTimer = 1;
			if (m_feverGauge.m_seconds < 60 * m_feverGauge.m_maxSeconds) {
				m_feverGauge.addTime(5 * 60);
				showSecondsObj(5 * 60);
			}
		}
		endPhase();
	}
}

// Animation transition into fever
void Player::startFever()
{
	// First 50 frames is to wait for light to hit fevergauge
	if (m_transitionTimer >= 50 && m_transitionTimer < 150) {
		m_transitionTimer += 3;
	}
	if (m_transitionTimer < 50) {
		m_transitionTimer += 2;
	}

	if (m_transitionTimer > 150) {
		m_poppedChain = false;
		m_forgiveGarbage = true;
		m_transitionTimer = 150;
		m_transformScale = 1;
		m_chainPopSpeed = 11;
		m_garbageSpeed = 8.0;
		m_garbageEndTime = 80;
		m_puyoBounceEnd = 40;
		m_puyoBounceSpeed = 2;
		m_normalTray.align(m_activeField->getProperties().offsetX - (16) * m_globalScale, m_activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		m_normalTray.setDarken(true);
		m_feverSuccess = 0;
		endPhase();
	}

	// Sound & animation
	if (m_transitionTimer == 50) {
		m_data->snd.fever.play(m_data);
		m_characterVoices.fever.play(m_data);
		m_characterAnimation.prepareAnimation("fever");
		if (m_data->windowFocus) {
			m_data->front->musicEvent(MusicEnterFever);
			m_currentGame->m_currentVolumeFever = 0;
		}
	}

	// Switch fields now
	if (m_transitionTimer == 101) {
		m_activeField = &m_fieldFever;
		m_activeGarbage = &m_feverGarbage;
	}

	// Rotation animation
	if (m_transitionTimer > 50 && m_transitionTimer <= 100) {
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		// Rotate out normal field
		m_transformScale = 1.f - static_cast<float>(m_transitionTimer - 50) / 50.f;
		m_activeField->setTransformScale(1.f - static_cast<float>(m_transitionTimer - 50) / 50.f);

	} else if (m_transitionTimer > 100 && m_transitionTimer <= 150) {
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Rotate in fever field
		m_transformScale = static_cast<float>(m_transitionTimer - 100) / 50.f;
		m_activeField->setTransformScale(static_cast<float>(m_transitionTimer - 100) / 50.f);
	}
}
void Player::checkEndFeverOnline()
{
	// Online player gets stuck in this state until message is received
	if (!m_messages.empty() && m_messages.front() == "fe") {
		// Fever ends
		m_messages.pop_front();
		m_feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Check loser at the last moment
		if (m_currentGame->m_currentRuleSet->m_feverDeath) {
			checkLoser(false); // Do not end phase in this function
		}
		endPhase();
	} else if (!m_messages.empty() && m_messages.front() == "fc") {
		// Fever continues;
		m_messages.pop_front();
		endPhase();
	}
}
void Player::checkEndFever()
{
	// Check if seconds == 0
	if (m_feverGauge.m_seconds == 0 && m_feverMode && m_type != ONLINE) // Online players end fever when they give a message
	{
		m_feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);

		// Send message BEFORE sending possible lose message
		if (m_type == HUMAN && m_currentGame->m_connected) {
			m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "fe", m_currentGame->m_channelName);
		}

		// Check loser at the last moment?
		if (m_currentGame->m_currentRuleSet->m_feverDeath) {
			checkLoser(false); // Do not end phase in this function
		}
	}

	// Not the end?
	if (!m_feverEnd && m_type == HUMAN && m_currentGame->m_connected) {
		m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "fc", m_currentGame->m_channelName);
	}

	// Check if player popped a chain
	if (m_poppedChain) {
		// Update chains
		if (m_predictedChain >= m_currentFeverChainAmount) {
			// Success
			m_currentFeverChainAmount = m_predictedChain + 1;
			m_feverSuccess = 1;
		} else if (m_predictedChain != m_currentFeverChainAmount - 1) {
			// Failure
			m_currentFeverChainAmount -= m_currentFeverChainAmount - m_predictedChain > 2 ? 2 : 1;
			m_feverSuccess = 2;
		}

		// Time bonus
		if (m_predictedChain > 2 && m_feverGauge.m_seconds != 0 && m_feverGauge.m_seconds < 60 * m_feverGauge.m_maxSeconds) {
			m_feverGauge.addTime((m_predictedChain - 2) * 30);
			showSecondsObj((m_predictedChain - 2) * 30);
		}

		// All clear bonus
		if (m_allClear == 1) {
			m_allClear = 0;
			m_allClearTimer = 1;
			m_currentFeverChainAmount += 2;
			if (m_feverGauge.m_seconds < 60 * 30) {
				// Do not add time if fever is actually ending
				if (m_feverGauge.m_seconds != 0) {
					m_feverGauge.addTime(5 * 60);
					showSecondsObj(5 * 60);
				}
			}
		}
		// Minimum and maximum fever chain
		if (m_currentFeverChainAmount < 3) {
			m_currentFeverChainAmount = 3;
		}
		if (m_currentFeverChainAmount > 15) {
			m_currentFeverChainAmount = 15;
		}

		// Clear field
		if (!m_feverEnd) {
			m_activeField->throwAwayField();
		}
	}
	endPhase();
}

// Transition out of fever
void Player::endFever()
{
	if (m_transitionTimer < 100) {
		m_transitionTimer += 3;
	} else {
		m_transitionTimer = 100;
		m_transformScale = 1;
		m_chainPopSpeed = 25;
		m_garbageSpeed = 4.8f;
		m_garbageEndTime = 100;
		m_puyoBounceEnd = 2;
		m_puyoBounceEnd = 50;
		endPhase();
	}

	// Switch fields now
	if (m_transitionTimer == 51) {
		m_feverMode = false;

		// Music: check if anyone is in fever
		bool stopMusic = true;
		for (const auto& player : m_currentGame->m_players) {
			if (player->m_feverMode == true && player->m_loseWin == LoseWinState::NOWIN && player->m_active) {
				stopMusic = false;
				break;
			}
		}
		if (stopMusic) {
			if (m_data->windowFocus)
				m_data->front->musicEvent(MusicExitFever);
			m_currentGame->m_currentVolumeNormal = 0;
		}
		m_activeField = &m_fieldNormal;
		m_activeGarbage = &m_normalGarbage;

		// Add fevergarbage to normalgarbage, move accumulator
		// And switch any targets to fevergarbage to normalgarbage
		m_normalGarbage.cq += m_feverGarbage.cq;
		m_feverGarbage.cq = 0;
		m_normalGarbage.gq += m_feverGarbage.gq;
		m_feverGarbage.gq = 0;
		for (auto i : m_feverGarbage.accumulator) {
			// Move accumulators to normalgarbage
			m_normalGarbage.accumulator.push_back(i);
		}
		m_feverGarbage.accumulator.clear();
		for (auto& player : m_currentGame->m_players) {
			// Set opponents targets to normalgarbage
			if (player != this) {
				player->m_targetGarbage[this] = &m_normalGarbage;
			}
		}
		updateTray(&m_normalGarbage);
		updateTray(&m_feverGarbage);

		// Reset seconds and gauge
		m_feverGauge.m_seconds = 60 * 15;
		m_feverGauge.setCount(m_currentGame->m_currentRuleSet->m_initialFeverCount);

		// Reset garbage tray
		m_normalTray.align(m_activeField->getProperties().offsetX, m_activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
		m_normalTray.setDarken(false);
	}

	// Rotation animation
	if (m_transitionTimer > 0 && m_transitionTimer <= 50) {
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Rotate out fever field
		m_transformScale = 1.f - static_cast<float>(m_transitionTimer) / 50.f;
		m_activeField->setTransformScale(1.f - static_cast<float>(m_transitionTimer) / 50.f);

	} else if (m_transitionTimer > 50 && m_transitionTimer <= 100) {
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		// Rotate in fever field
		m_transformScale = static_cast<float>(m_transitionTimer - 50) / 50.f;
		m_activeField->setTransformScale(static_cast<float>(m_transitionTimer - 50) / 50.f);
	}
}

void Player::playFever()
{
	if (!m_feverMode)
		return;

	// Play sounds
	if (m_feverGauge.m_seconds / 60 > 0 && m_feverGauge.m_seconds / 60 < 6 && m_feverGauge.m_seconds % 60 == 0 && m_loseWin == LoseWinState::NOWIN) {
		m_data->snd.feverTimeCount.play(m_data);
	}
	if (m_feverGauge.m_seconds == 1 && m_loseWin == LoseWinState::NOWIN) {
		m_data->snd.feverTimeEnd.play(m_data);
	}

	// Subtract seconds
	if (m_feverGauge.m_seconds > 0 && m_feverGauge.m_endless == false && m_loseWin == LoseWinState::NOWIN && !m_currentGame->m_stopChaining) {
		m_feverGauge.m_seconds--;
	}

	// Display
	m_feverGauge.update();
}

void Player::dropFeverChain()
{
	// Cycle background color
	const int old = m_feverColor;
	while (old == m_feverColor) {
		m_feverColor = ppvs::getRandom(5);
	}

	// Drop field
	m_activeField->dropField(getFeverChain(getRandom(m_currentGame->m_currentRuleSet->m_nFeverChains, m_rngFeverChain), m_colors, m_currentFeverChainAmount, getRandom(m_colors, m_rngFeverColor)));
	m_calledRandomFeverChain++;

	// Garbage should not drop after this
	m_forgiveGarbage = true;

	endPhase();
}

void Player::showSecondsObj(int n)
{
	const PosVectorFloat pv = m_feverGauge.getPositionSeconds();
	m_secondsObj.push_back(new SecondsObject(m_data));
	m_secondsObj.back()->setScale(m_globalScale);
	m_secondsObj.back()->showAt(m_nextPuyoOffsetX + (pv.x - 16) * m_nextPuyoScale * m_globalScale, m_nextPuyoOffsetY + (pv.y + 16) * m_nextPuyoScale * m_globalScale * static_cast<float>(m_secondsObj.size()), n);
}

void Player::setMarginTimer()
{
	m_marginTimer++;
}

void Player::prepareVoice(int chain, int predictedChain)
{
	m_playVoice = getVoicePattern(chain, predictedChain, m_currentGame->m_currentRuleSet->m_voicePatternFever);

	// Count diacutes
	if (m_playVoice == 4)
		m_diacute++;

	m_stutterTimer = 0;
}

void Player::playVoice()
{
	if (m_playVoice < 0)
		return;

	m_stutterTimer++;
	if (m_stutterTimer == 1) {
		m_characterVoices.chain[m_playVoice].stop(m_data);
		m_characterVoices.chain[m_playVoice].play(m_data);
	}
	if (m_diacute > 0 && m_stutterTimer == 1 && m_playVoice > 4) {
		m_stutterTimer = -10;
		m_diacute--;
	}
	if (m_diacute == 0) {
		m_playVoice = -1;
	}
}

#pragma region Online stuff

void Player::bindPlayer(const std::string& name, unsigned int id, bool setActive)
{
	m_onlineName = name;
	m_active = setActive;
	m_onlineId = id;
	// Reset stuff
	m_messages.clear();
	m_wins = 0;
	m_loseConfirm = false;
}

void Player::unbindPlayer()
{
	m_onlineName = "";
	m_onlineId = 0;
	m_active = false;
	m_prepareActive = false;
	m_rematch = false;
	m_rematchIcon.setVisible(false);
	m_messages.clear();

	// Still playing
	if (m_loseWin == LoseWinState::NOWIN && m_currentPhase != Phase::IDLE) {
		setLose();

		// Add to replay
		if (m_currentGame->m_settings->recording == RecordState::RECORDING) {
			MessageEvent me = { m_data->matchTimer, "exit" };
			m_recordMessages.push_back(me);
		}
	}
	setStatusText(m_currentGame->m_translatableStrings.disconnected.c_str());

	// Was player in choose color status?
	if (!m_pickedColor) {
		bool start = true;
		for (auto& player : m_currentGame->m_players) {
			if (!player->m_active && !m_currentGame->m_settings->useCpuPlayers)
				continue;
			if (!player->m_pickedColor || player->m_colorMenuTimer < -2) {
				start = false;
				break;
			}
		}
		if (start) {
			// Players should be in pickcolor phase

			m_colorMenuTimer = 0;
			for (const auto& player : m_currentGame->m_players) {
				if (!player->m_active && !m_currentGame->m_settings->useCpuPlayers)
					continue;

				if (player->m_currentPhase == Phase::PICKCOLORS)
					player->m_currentPhase = Phase::GETREADY;
				else
					return;
			}
			if (m_currentGame->getActivePlayers() > 0) {
				m_currentGame->m_readyGoObj.prepareAnimation("readygo");
				m_currentGame->m_data->matchTimer = 0;
			}
		}
	}
}

void Player::processMessage()
{
	// Skip move message if it's a big multiplayer match
	if (m_currentGame->m_players.size() > 10 && !m_messages.empty() && m_messages.front()[0] == 'm') {
		m_messages.pop_front();
	}

	// Receive offset message
	if (m_type == ONLINE && !m_messages.empty() && m_messages.front() == "fo") {
		m_currentGame->m_currentRuleSet->onOffset(this);
		m_messages.pop_front();
	}

	// Replay: player exits
	if (m_currentGame->m_settings->recording == RecordState::REPLAYING && m_type == ONLINE && !m_messages.empty() && m_messages.front() == "exit") {
		setLose();
	}
}

void Player::addMessage(std::string mes)
{
	// Add to replay
	if (m_currentGame->m_settings->recording == RecordState::RECORDING) {
		MessageEvent me = { m_data->matchTimer, "" };
		m_recordMessages.push_back(me);
		if (mes.length() < 64) {
			strcpy(m_recordMessages.back().message, mes.c_str());
		}
	}

	// Process immediately?
	if (m_type == ONLINE && mes == "d") {
		m_waitForConfirm--;
		return;
	}

	// Check if lose confirm was received
	if (m_type == ONLINE && mes == "o") {
		// This player confirms your loss
		m_loseConfirm = true;
		return;
	}

	// Add message
	m_messages.push_back(mes);
}

void Player::confirmGarbage()
{
	// Moves < Min(4, Turns+1) -> Moves should be less than 4, except the first turns

	// Read everyone's confirm count, if there's anyone with confirm>4, you can't move on
	// (alternatively, we could count the total)
	for (const auto& player : m_currentGame->m_players) {
		if (player != this && player->m_waitForConfirm > 3) {
			// Also see garbagephase
			m_currentGame->m_stopChaining = true;
			return;
		}
	}

	m_currentGame->m_stopChaining = false;
	endPhase();
}

void Player::waitGarbage()
{
	// ONLINE player gets stuck here until message about garbage arrives
	bool receive = false;

	// Receive n: dropped nothing
	if (m_type == ONLINE && !m_messages.empty() && m_messages.front()[0] == 'n') {
		receive = true;
		m_messages.pop_front();
	}

	if (m_type == ONLINE && !m_messages.empty() && m_messages.front()[0] == 'g') {
		// Drop garbage
		int dropAmount = 0;
		sscanf(m_messages.front().c_str(), "g|%i", &dropAmount);
		m_activeField->dropGarbage(false, dropAmount);
		receive = true;
		m_messages.pop_front();
	}

	if (receive == true) {
		// Send message dO back
		if (m_currentGame->m_connected && m_type == ONLINE) {
			m_currentGame->m_network->sendToChannel(CHANNEL_GAME, "d", m_currentGame->m_channelName.c_str());
		}
		endPhase();
	}
}

// Ignore all messages except the lose message
void Player::waitLose()
{
	if (m_messages.empty())
		return;

	if (m_messages.front()[0] == 'l') {
		// Lose
		setLose();

		// Send confirmation back to this particular player
		if (m_currentGame->m_connected) {
			m_currentGame->m_network->sendToPeer(CHANNEL_GAME, "o", m_currentGame->m_channelName, m_onlineId);
		}
	}

	// Discard any message
	m_messages.pop_front();
}

void Player::getUpdate(std::string str)
{
	char fieldstring[500] = {};
	char feverstring[500] = {};

	// 0[spectate]1[currentphase]2[fieldstringnormal]3[fevermode]4[fieldfever]5[fevercount]
	// 6[rng seed]7[fever rng called]8[turns]9[colors]
	// 10[margintimer]11[chain]12[currentFeverChainAmount]13[nGQ]14[fGQ]
	int ph = -1;
	int fm = 0;
	int fc = 0;
	int rngseed = 0;
	int rngcalled = 0;
	int trns = 0;
	int clrs = 0;
	int mrgntmr = 0;
	int chn = 0;
	int crntfvrchn = 0;
	int prdctchn = 0;
	int allclr = 0;
	int nGQ = 0;
	int fGQ = 0;
	sscanf(str.c_str(), "spectate|%i|%s |%i|%s |%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|", &ph, fieldstring, &fm, feverstring, &fc, &rngseed, &rngcalled, &trns, &clrs, &mrgntmr, &chn, &crntfvrchn, &nGQ, &fGQ, &prdctchn, &allclr);

	// Initialize
	setRandomSeed(rngseed, &m_rngNextList);
	m_colors = clrs;
	initValues(static_cast<int>(rngseed + m_onlineId));

	// Set other stuff
	m_currentPhase = static_cast<Phase>(ph);
	m_fieldNormal.setFieldFromString(fieldstring);
	m_feverMode = fm == 0 ? false : true;
	m_fieldFever.setFieldFromString(feverstring);
	m_feverGauge.setCount(fc);
	m_normalGarbage.gq = nGQ;
	m_feverGarbage.gq = fGQ;
	updateTray();
	m_calledRandomFeverChain = rngcalled;
	for (int i = 0; i < rngcalled; i++) {
		getRandom(0, m_rngFeverChain);
		getRandom(0, m_rngFeverColor);
	}
	m_turns = trns;

	// Call nextlist random
	for (int i = 0; i < m_turns; i++) {
		getRandom(0, m_rngNextList);
		getRandom(0, m_rngNextList);
	}
	m_marginTimer = mrgntmr;
	m_chain = chn;
	m_predictedChain = prdctchn;
	m_allClear = allclr;

	// Set target garbage, otherwise it may crash
	for (auto& player : m_currentGame->m_players) {
		if (player != this) {
			m_targetGarbage[player] = player->m_activeGarbage;
		}
	}

	m_currentFeverChainAmount = crntfvrchn;

	if (m_feverMode) {
		m_activeField = &m_fieldFever;
		m_activeGarbage = &m_feverGarbage;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		m_chainPopSpeed = 11;
		m_garbageSpeed = 8.0;
		m_garbageEndTime = 80;
		m_puyoBounceEnd = 40;
		m_puyoBounceSpeed = 2;
		m_normalTray.align(m_activeField->getProperties().offsetX - 16 * m_globalScale, m_activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		m_normalTray.setDarken(true);
	}
}

void Player::prepareDisconnect()
{
	if (m_currentGame->m_settings->rankedMatch && m_type == HUMAN && m_currentGame->m_connected) {
		// Max wins reached
		bool disconnect = false;
		for (auto& player : m_currentGame->m_players) {
			if (player->m_wins == m_currentGame->m_settings->maxWins) {
				// Prepare to disconnect channel
				disconnect = true;
			}
		}
		if (disconnect) {
			m_currentGame->m_rankedState = 1;
			m_currentGame->m_network->requestChannelDescription(m_currentGame->m_channelName, std::string(""));
		}
	}
}

#pragma endregion

#pragma region Rendering

void Player::reloadAllAssets()
{
	m_fieldFeverSprite.setImage(m_data->imgFieldFever);
	m_allClearSprite.setImage(m_data->imgAllClear);
	m_crossSprite.setImage(m_data->imgPuyo);
	m_crossSprite.setSubRect(7 * kPuyoX, 12 * kPuyoY, kPuyoX, kPuyoY);
	m_crossSprite.setCenter(0, 0);
	m_winSprite.setImage(m_data->imgWin);
	m_loseSprite.setImage(m_data->imgLose);
	for (auto & i : m_colorMenuBorder) {
		i.setImage(m_data->imgPlayerBorder);
	}
	for (auto & i : m_spice) {
		i.setImage(m_data->imgSpice);
	}
	m_charHolderSprite.setImage(m_data->imgCharHolder);
	m_rematchIcon.setImage(m_data->imgCheckMark);
	if (m_playerNum==1) {
		m_fieldSprite.setImage(m_data->imgField1);
		m_borderSprite.setImage(m_data->imgBorder1);
	}
	else if (m_playerNum>1) {
		m_fieldSprite.setImage(m_data->imgField2);
		m_borderSprite.setImage(m_data->imgBorder2);
	}
	m_currentCharacterSprite.setImage(m_currentGame->m_assetManager->loadImage(ImageToken::imgCharIcon, m_character));
	for (auto& i : m_dropSet) {
		i.setImage(m_currentGame->m_assetManager->loadImage(ImageToken::imgDropSet));
	}
	setFieldImage(m_character);
	const PosVectorFloat offset(
		m_activeField->getProperties().centerX * 1,
		m_activeField->getProperties().centerY / 2.0f * 1);
	if (m_currentGame->m_players.size() <= 10)
		m_characterAnimation.init(m_data, offset, 1, m_currentGame->m_assetManager->getAnimationFolder(m_character));
	else
		m_characterAnimation.init(m_data, offset, 1, "");
	for (auto* sec : m_secondsObj) {
		sec->reload();
	}
	if (m_chainWord!=nullptr) {
		m_chainWord->reload();
	}

	m_scoreCounter.reload();

	m_activeField->hotReload();

}

// Draw everything from the player
void Player::draw()
{
	// Field coordinates
	// -----------------
	m_data->front->pushMatrix();
	m_data->front->translate(m_properties.offsetX, m_properties.offsetY, 0);
	m_data->front->scale(m_properties.scaleX * getGlobalScale(), m_properties.scaleY * getGlobalScale(), 1);

	m_data->front->clearDepth();
	m_data->front->setDepthFunction(DepthFunction::LessOrEqual);
	m_data->front->setBlendMode(BlendingMode::AlphaBlending);
	m_data->front->setColor(0, 0, 0, 0);
	m_data->front->drawRect(nullptr, 0, 0, 192, 336);

	// Fever flip coordinates
	// ----------------------
	m_data->front->pushMatrix();
	m_data->front->translate(192.f / 2.f, 0, 0);
	m_data->front->scale(m_transformScale, 1, 1);
	m_data->front->translate(-192.f / 2.f, 0, 0);
	m_activeField->draw();
	m_data->front->setDepthFunction(DepthFunction::Equal);
	m_movePuyo.draw();
	m_data->front->setDepthFunction(DepthFunction::Always);
	m_data->front->popMatrix();
	// ----------------------
	m_borderSprite.draw(m_data->front);
	m_chainWord->draw(m_data->front);
	m_characterAnimation.draw();
	// -----------------
	m_data->front->popMatrix();

	// Next coordinates
	// ----------------
	m_data->front->pushMatrix();
	m_data->front->translate(m_nextPuyoOffsetX, m_nextPuyoOffsetY, 0);
	m_data->front->scale(m_nextPuyoScale, m_nextPuyoScale, 1);
	m_nextPuyo.draw();
	m_feverGauge.draw();
	// ----------------
	m_data->front->popMatrix();

	m_normalTray.draw();
	m_feverTray.draw();
	m_scoreCounter.draw();

	// Field coordinates
	// -----------------
	m_data->front->pushMatrix();
	m_data->front->translate(m_properties.offsetX, m_properties.offsetY, 0);
	m_data->front->scale(m_properties.scaleX * getGlobalScale(), m_properties.scaleY * getGlobalScale(), 1);
	drawColorMenu();
	drawWin();
	drawLose();

	// Online; darken screen if player is not bound
	if (!m_currentGame->m_settings->useCpuPlayers && (m_onlineName.empty() || !m_active)) {
		m_overlaySprite.draw(m_data->front);
	}

	// Draw status text
	if (m_statusText) {
		m_data->front->setColor(255, 255, 255, 255);
		if (m_currentGame->m_forceStatusText
			|| (m_type == ONLINE || m_type == HUMAN)
				&& m_currentGame->m_currentGameStatus != GameStatus::PLAYING
				&& m_currentGame->m_currentGameStatus != GameStatus::SPECTATING
				&& !m_currentGame->m_settings->useCpuPlayers) {
			m_statusText->draw(0, 0);
		}
	}
	// -----------------
	m_data->front->popMatrix();

	if (m_showCharacterTimer > 0 && m_currentGame->m_currentGameStatus != GameStatus::PLAYING) {
		m_charHolderSprite.draw(m_data->front);
		m_currentCharacterSprite.draw(m_data->front);
		for (auto& sprite : m_dropSet)
			sprite.draw(m_data->front);
	}

	if (m_currentGame->m_currentGameStatus != GameStatus::PLAYING && m_currentGame->m_currentGameStatus != GameStatus::SPECTATING) {
		m_rematchIcon.draw(m_data->front);
	}
}

// Draw light effect, secondsObj
void Player::drawEffect()
{
	for (const auto& lightEffect : m_lightEffect) {
		lightEffect->draw(m_data->front);
	}

	m_feverLight.draw(m_data->front);

	for (const auto& secondsObj : m_secondsObj) {
		secondsObj->draw(m_data->front);
	}
}

void Player::drawFieldBack(PosVectorFloat /*position*/, const float rotation)
{
	m_fieldSprite.setPosition(0, 0);
	m_fieldSprite.setCenter(0, 0);
	m_fieldSprite.setScale(1, 1);
	m_fieldSprite.setRotation(rotation);
	m_fieldSprite.draw(m_data->front);
}

// C99 round
float round(const float r) { return (r > 0.0f) ? floor(r + 0.5f) : ceil(r - 0.5f); }

void Player::drawFieldFeverBack(PosVectorFloat /*position*/, float rotation)
{
	if (!m_feverMode) {
		return;
	}

	m_fieldFeverSprite.setPosition(0, 0);
	m_fieldFeverSprite.setCenter(0, 0);
	m_fieldFeverSprite.setScale(1, 1);
	m_fieldFeverSprite.setRotation(rotation);

	// Transition to new color
	round(m_feverColorR * 100.f) < round(tunnelShaderColor[m_feverColor][0] * 100.f) ? m_feverColorR += 0.01f : m_feverColorR -= 0.01f;
	round(m_feverColorG * 100.f) < round(tunnelShaderColor[m_feverColor][1] * 100.f) ? m_feverColorG += 0.01f : m_feverColorG -= 0.01f;
	round(m_feverColorB * 100.f) < round(tunnelShaderColor[m_feverColor][2] * 100.f) ? m_feverColorB += 0.01f : m_feverColorB -= 0.01f;

	if (m_data->tunnelShader) {
		m_data->tunnelShader->setParameter("cl", static_cast<double>(m_feverColorR), static_cast<double>(m_feverColorG), static_cast<double>(m_feverColorB), 1.0);
		m_fieldFeverSprite.setImage(m_data->imgFieldFever);
		m_fieldFeverSprite.draw(m_data->front, m_data->tunnelShader);
	} else {
		m_fieldFeverSprite.setImage(m_data->imgFeverBack[m_data->globalTimer / 2 % 30]);
		m_fieldFeverSprite.setColor(m_feverColorR * 255, m_feverColorG * 255, m_feverColorB * 255);
		m_fieldFeverSprite.draw(m_data->front);
	}
}

void Player::drawAllClear(const PosVectorFloat pos, const float scaleX, const float scaleY, const float rotation)
{
	const float dist = m_activeField->getFieldSize().y * 0.67f;
	const float x = pos.x - dist * sin(rotation * kPiF / 180.f);
	const float y = pos.y - dist * cos(rotation * kPiF / 180.f);

	// Tsu type
	if (m_allClear == 1) {
		// Just draw it until it clears
		m_allClearSprite.setVisible(true);
		m_allClearSprite.setPosition(x, y);
		m_allClearSprite.setScale(1, 1);
		m_allClearSprite.setRotation(rotation);
		m_allClearSprite.draw(m_data->front);
	}

	// Fever type: use timer
	if (m_allClearTimer >= 1) {
		m_allClearTimer++;
		m_allClearSprite.setPosition(x, y);
		m_allClearSprite.setScale(1, 1);
		m_allClearSprite.setRotation(rotation);
		if (m_allClearTimer >= 60) {
			// Flicker
			m_allClearSprite.setScale(scaleX + scaleX * static_cast<float>(m_allClearTimer - 60) / 10.f, scaleY - scaleY * static_cast<float>(m_allClearTimer - 60) / 10.f);
			m_allClearSprite.setTransparency(1.f - static_cast<float>(m_allClearTimer - 60) / 10.f);
		} else if (m_allClearTimer > 1 && m_allClearTimer < 60) {
			// Show normally
			m_allClearSprite.setVisible(true);
			m_allClearSprite.setTransparency(1);
		}

		m_allClearSprite.draw(m_data->front);

		if (m_allClearTimer >= 70) {
			m_allClearTimer = 0;
		}
	}
}

void Player::drawCross(FeRenderTarget* r)
{
	// Place at spawn point
	PosVectorInt spv = m_movePuyo.getSpawnPoint();
	spv.y += 1;
	PosVectorFloat rpv = m_activeField->getLocalCoordinates(spv.x, spv.y);

	m_crossSprite.setPosition(rpv.x, rpv.y);
	m_crossSprite.draw(r);

	// Check if 2nd spawn point
	if (m_currentGame->m_currentRuleSet->m_doubleSpawn) {
		rpv = m_activeField->getLocalCoordinates(spv.x + 1, spv.y);
		m_crossSprite.setPosition(rpv.x, rpv.y);
		m_crossSprite.draw(r);
	}
}

void Player::drawLose()
{
	PosVectorFloat position = m_activeField->getBottomCoordinates(true);
	position.y -= m_activeField->getFieldSize().y * 0.75f;

	if (m_loseWin == LoseWinState::LOSE && m_loseWinTimer > 60) {
		m_loseSprite.setPosition(position);
		m_loseSprite.setScale(1 + 0.1f * sin(static_cast<float>(m_loseWinTimer - 60) / 20.f));
		m_loseSprite.draw(m_data->front);
	}
}

void Player::drawWin()
{
	PosVectorFloat position = m_activeField->getBottomCoordinates(true);
	position.y -= m_activeField->getFieldSize().y * 0.75f;

	if (m_loseWin == LoseWinState::WIN) {
		m_winSprite.setPosition(position.x, position.y + 10.f * sin(static_cast<float>(m_loseWinTimer) / 20.f));
		m_winSprite.setScale(1.f);
		m_winSprite.draw(m_data->front);
	}
}

void Player::drawColorMenu()
{
	for (auto& i : m_colorMenuBorder) {
		i.draw(m_data->front);
	}

	for (auto& i : m_spice) {
		i.draw(m_data->front);
	}
}

void Player::setStatusText(const char* utf8)
{
	if (utf8 == m_lastText) {
		return;
	}
	if (!m_statusFont) {
		return;
	}
	delete m_statusText;

	m_statusText = m_statusFont->render(utf8);
	m_lastText = utf8;
}

void Player::setDropSetSprite(int x, int y, PuyoCharacter pc)
{
	// Get total width and center drop set
	float length = 0;
	const float scale = m_globalScale * 0.75f;
	for (int j = 0; j < 16; j++) {
		const MovePuyoType mpt = getFromDropPattern(pc, j);
		length += mpt == MovePuyoType::DOUBLET ? 10 : 18;
	}
	float xx = -length / 2.f - 5.f;

	for (int j = 0; j < 16; j++) {
		const MovePuyoType mpt = getFromDropPattern(pc, j);
		m_dropSet[j].setPosition(static_cast<float>(x) + xx * scale, static_cast<float>(y));
		m_dropSet[j].setScale(scale);
		switch (mpt) {
		case MovePuyoType::DOUBLET:
			m_dropSet[j].setSubRect(0, 0, 16, 24);
			m_dropSet[j].setCenter(0, 24);
			xx += 10;
			break;
		case MovePuyoType::TRIPLET:
			m_dropSet[j].setSubRect(16, 0, 24, 24);
			m_dropSet[j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::TRIPLET_R:
			m_dropSet[j].setSubRect(40, 0, 24, 24);
			m_dropSet[j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::QUADRUPLET:
			m_dropSet[j].setSubRect(64, 0, 24, 24);
			m_dropSet[j].setCenter(0, 24);
			xx += 18;
			break;
		case MovePuyoType::BIG:
			m_dropSet[j].setSubRect(88, 0, 24, 24);
			m_dropSet[j].setCenter(0, 24);
			xx += 18;
			break;
		}
	}
}


#pragma endregion

}
