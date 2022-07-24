#include <algorithm>
#include "Player.h"
#include "Game.h"
#include "../PVS_ENet/PVS_Client.h"
#include "RNG/PuyoRNG.h"
using namespace std;

namespace ppvs
{

Player::Player(PlayerType type, int playernum, int totalPlayers, Game* g)
	: feverGauge(g->data), m_feverLight(g->data)
{
	currentgame = g;
	data = currentgame->data;
	debug = 0;
	debugCounter = 0;

	m_randomizerNextList = nullptr;
	m_randomizerFeverChain = nullptr;
	m_randomizerFeverColor = nullptr;
	m_randomizerNuisanceDrop = new MersenneTwister();

	// Types of players: human, CPU, network etc
	m_type = type;
	m_playernum = playernum;
	active = false;
	prepareActive = false;
	activeAtStart = 0;
	rematch = false;
	onlineID = 0;
	wins = 0;
	proposedRandomSeed = 0;
	waitForConfirm = 0;
	loseConfirm = false;
	lastAttacker = nullptr;

	forceStatusText = false;

	feverMode = false;
	feverEnd = false;
	calledRandomFeverChain = 0;
	bonusEQ = false;
	hasMoved = false;

	// Default active field: normal field
	activeField = &m_fieldNormal;

	// Controller
	controls.init(m_playernum, m_type, currentgame->settings->recording);

	// Mover
	movePuyos.init(data);
	dropspeed = 2;

	// Tray
	normalTray.init(data);
	feverTray.init(data);

	// Initialize player values
	colors = 4;

	// Player 1 abides default character
	m_character = playernum != 1 ? ARLE : currentgame->settings->defaultPuyoCharacter;
	cpuAI = nullptr;
	divider = 2;

	// Give CPU an AI
	if (m_type == CPU)
	{
		cpuAI = new AI(this);
	}

	// TEMP: in online version, other players must inform eachother from their randomseedFever
	if (currentgame->settings->useCPUplayers)
	{
		initValues(ppvs::getRandom(1000));
		// All player are active
		active = true;
	}
	else initValues(currentgame->randomSeed_NextList + onlineID);

	// TEMP
	// Field size may be set up by ruleset. For now, use standard field size
	properties.gridHeight = 28;
	properties.gridWidth = 32;
	properties.gridX = 6;
	properties.gridY = 12 + 3; // Always add 3 extra layers
	properties.scaleX = 1; // Local scale: only for local effects (bouncing and stuff)
	properties.scaleY = 1;
	properties.angle = 0;

	// Initialize objects (place fields, movepuyo, nextpuyo etc)
	playerSetup(properties, m_playernum, totalPlayers);
	properties = m_fieldNormal.getProperties();

	// Resize sprite to fieldsize
	m_fieldSprite.setSize(m_fieldNormal.getFieldSize()); // Useless
	m_fieldFeverSprite.setImage(data->imgFieldFever);
	m_fieldFeverSprite.setSize(m_fieldNormal.getFieldSize());

	// Set chainword
	pchainWord = new ChainWord(data);
	pchainWord->setScale(m_globalScale);
	
	// Initialize movePuyos at least once
	movePuyos.prepare(DOUBLET, this, 0, 0);

	// Initialize garbagetray
	normalTray.align(properties.offsetX, properties.offsetY - (32) * m_globalScale, m_globalScale);
	feverTray.align(properties.offsetX, properties.offsetY - (32) * m_globalScale, m_globalScale);
	feverTray.setVisible(false);

	// Initialize scoreCounter
	m_scoreCounter.init(data, properties.offsetX, m_fieldNormal.getBottomCoord().y + PUYOY / 4 * m_globalScale, m_globalScale);

	// Load other sprites
	m_allclearSprite.setImage(data->imgAllClear);
	m_allclearSprite.setCenter();
	m_crossSprite.setImage(data->imgPuyo);
	m_crossSprite.setSubRect(7 * PUYOX, 12 * PUYOY, PUYOX, PUYOY);
	m_crossSprite.setCenter(0, 0);
	m_winSprite.setImage(data->imgWin);
	m_winSprite.setCenter();
	m_loseSprite.setImage(data->imgLose);
	m_loseSprite.setCenter();

	// Set border
	m_borderSprite.setCenter();
	m_borderSprite.setPosition(properties.centerX,
		properties.centerY / 2);
	m_borderSprite.setScale(1);

	// Set origin of field to bottom of image
	m_fieldSprite.setCenterBottom();
	m_fieldFeverSprite.setCenterBottom();

	// Initialize character animation object
	setCharacter(m_character);
	pickingCharacter = false;

	// Color menu objects
	for (int i = 0; i < 9; i++)
	{
		colorMenuBorder[i].setImage(data->imgPlayerBorder);
		colorMenuBorder[i].setVisible(false);
		colorMenuBorder[i].setSubRect(i % 3 * 24, i / 3 * 24, 24, 24);
	}

	for (int i = 0; i < 5; i++)
	{
		spice[i].setImage(data->imgSpice);
		spice[i].setSubRect(0, i * 50, 138, 50);
		spice[i].setVisible(false);
		spice[i].setCenter();
	}

	spiceSelect = 2;

	// Online
	// Needs a black image to draw over field
	overlaySprite.setImage(nullptr);
	overlaySprite.setTransparency(0.5f);
	overlaySprite.setScale(2 * 192, 336);
	overlaySprite.setColor(0, 0, 0);
	overlaySprite.setPosition(-192 / 2, -336 / 4);

	charHolderSprite.setImage(data->imgCharHolder);
	charHolderSprite.setCenter();
	charHolderSprite.setPosition(
		properties.offsetX + properties.gridWidth * properties.gridX / 2 * m_globalScale,
		properties.offsetY + properties.gridHeight * properties.gridY / 2 * m_globalScale
	);
	charHolderSprite.setVisible(false);
	charHolderSprite.setScale(m_globalScale);
	currentCharacterSprite.setVisible(true);
	for (auto& i : dropset)
	{
		i.setVisible(true);
	}
	showCharacterTimer = 0;

	rematchIcon.setImage(data->imgCheckmark);
	rematchIcon.setCenter();
	rematchIcon.setPosition(
		properties.offsetX + properties.gridWidth * properties.gridX / 2 * m_globalScale,
		properties.offsetY + properties.gridHeight * (properties.gridY / 2 + 3) * m_globalScale
	);
	rematchIcon.setVisible(false);
	rematchIcon.setScale(m_globalScale);
	rematchIconTimer = 1000;

	// Set text
	// Text messages on screen
	statusFont = data->front->loadFont("Arial", 14);
	statusText = nullptr;

	if (currentgame->settings->recording != PVS_REPLAYING)
	{
		setStatusText(currentgame->translatableStrings.waitingForPlayer.c_str());
	}
}

void Player::reset()
{
	// Release all controls
	controls.release();

	// Set colors
	if (!currentgame->settings->pickColors && currentgame->settings->recording != PVS_REPLAYING
		&& currentgame->settings->rulesetInfo.colors >= 3 && currentgame->settings->rulesetInfo.colors <= 5)
		colors = currentgame->settings->rulesetInfo.colors;

	// Reset fields
	m_fieldNormal.clearField();
	m_fieldFever.clearField();
	m_fieldSprite.setVisible(true);
	m_fieldFeverSprite.setVisible(false);
	m_fieldNormal.setTransformScale(1);
	m_fieldFever.setTransformScale(1);

	movePuyos.prepare(DOUBLET, this, 0, 0);
	movePuyos.setVisible(false);

	// Reset fever mode
	activeField = &m_fieldNormal;
	activeGarbage = &normalGarbage;
	normalTray.align(activeField->getProperties().offsetX, activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
	normalTray.setDarken(false);
	normalTray.update(0);
	feverTray.update(0);

	// Reset replay values
	if (currentgame->settings->recording == PVS_RECORDING)
	{
		recordMessages.clear();
	}
	activeAtStart = 0;

	// Reset others
	// TEMP random value
	if (currentgame->settings->useCPUplayers)
	{
		initValues(ppvs::getRandom(1000));
	}
	else
	{
		initValues(currentgame->randomSeed_NextList + onlineID);
	}

	if (!active)
	{
		return;
	}

	// TEMP immediate reset
	if (currentgame->settings->pickColors)
	{
		currentphase = PICKCOLORS;
	}
	else
	{
		currentphase = GETREADY;
		currentgame->readyGoObj.prepareAnimation("readygo");
		currentgame->data->matchTimer = 0;
	}
}

void Player::initValues(int randomseed)
{
	useDropPattern = true;

	// Initialize randomizer
	randomSeedFever = randomseed;
	setRandomSeed(randomSeedFever, &m_randomizerFeverChain);
	setRandomSeed(randomSeedFever + 1, &m_randomizerFeverColor);
	calledRandomFeverChain = 0;
	nextPuyoActive = true;

	// Init nuisance drop pattern randomizer
	m_randomizerNuisanceDrop->init_genrand(randomseed);
	m_nuisanceList.clear();

	// Other values to initialize
	diacute = 0;
	playvoice = -1; stutterTimer = 0;
	currentFeverChainAmount = 5;
	feverMode = false;
	feverEnd = false;
	feverColor = 0;
	feverColorR = 0; feverColorG = 0; feverColorB = 1;
	feverGauge.seconds = 60 * 15;
	feverGauge.setCount(currentgame->currentruleset->initialFeverCount);
	divider = 2;

	poppedChain = false;
	garbageTimer = 0;
	garbageSpeed = 4.8f;
	chainPopSpeed = 25;
	garbageEndTime = 100;
	puyoBounceEnd = 50;
	puyoBounceSpeed = 2;
	lastAttacker = nullptr;
	gravity = 0.5f;
	attdef = NOATTACK;
	scoreVal = 0; currentScore = 0;
	createPuyo = false;
	forgiveGarbage = false;
	losewin = NOWIN;
	chain = 0; puyosPopped = 0; totalGroups = 0; groupR = 0; groupG = 0; groupB = 0; groupY = 0; groupP = 0;
	predictedChain = 0; point = 0; bonus = 0; linkBonus = 0; allClear = 0; rememberMaxY = 0; rememberX = 0;
	dropBonus = 0;
	foundChain = false;
	normalGarbage.CQ = 0; normalGarbage.GQ = 0;
	normalGarbage.accumulator.clear();
	feverGarbage.CQ = 0; feverGarbage.GQ = 0;
	feverGarbage.accumulator.clear();
	activeGarbage = &normalGarbage;
	EQ = 0; tray = 0;
	garbageDropped = 0;
	garbageCycle = 0;
	hasMoved = false;
	destroyPuyosTimer = 0;
	margintimer = 0;
	colorMenuTimer = 1;
	menuHeight = 24;
	pickedColor = false;
	bonusEQ = false;
	debugCounter = 0;

	m_lightTimer = 0;
	m_loseWinTimer = 0;
	m_readyGoTimer = 0;
	m_transitionTimer = 0;
	allclearTimer = 0;
	m_transformScale = 1;
	feverSuccess = 0;

	// Stop any animation
	characterAnimation.prepareAnimation("");

	// Initialize or override values according to ruleset
	currentgame->currentruleset->onInit(this);

	// Add 4 initial 3-colored puyo
	initNextList();
	turns = 0;
	waitForConfirm = 0;
	loseConfirm = false;

	// Set nextpuyo
	if (useDropPattern)
	{
		m_nextPuyo.update(m_nextList, m_character, 0);
	}
	else
	{
		m_nextPuyo.update(m_nextList, ARLE, 0);
	}

	// Idle
	currentphase = IDLE;

	updateTray();
}

void Player::initNextList()
{
	setRandomSeed(currentgame->randomSeed_NextList, &m_randomizerNextList);
	m_nextList.clear();

	// nextList needs 3 pairs to start
	addNewColorPair(3);
	addNewColorPair(3);
	checkAllClearStart();
	addNewColorPair(colors);
	turns = 0;
}

void Player::checkAllClearStart()
{
	if (currentgame->currentruleset->allClearStart)
	{
		return;
	}

	if (m_nextList.empty())
	{
		return;
	}

	int firstcolor = m_nextList.front();

	// At this point there should be 4 colors in m_nextlist
	for (const int& next : m_nextList)
	{
		if (firstcolor != next)
		{
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

FeSound* Player::loadVoice(const std::string& folder, const char* sound)
{
	return data->front->loadSound((folder + sound).c_str());
}

void Player::initVoices()
{
	const std::string currentCharacter = currentgame->settings->characterSetup[getCharacter()];
	const std::string folder = folder_user_character + currentCharacter + std::string("/Voice/");
	characterVoices.chain[0].setBuffer(loadVoice(folder, "chain1.wav"));
	characterVoices.chain[1].setBuffer(loadVoice(folder, "chain2.wav"));
	characterVoices.chain[2].setBuffer(loadVoice(folder, "chain3.wav"));
	characterVoices.chain[3].setBuffer(loadVoice(folder, "chain4.wav"));
	characterVoices.chain[4].setBuffer(loadVoice(folder, "chain5.wav"));
	characterVoices.chain[5].setBuffer(loadVoice(folder, "spell1.wav"));
	characterVoices.chain[6].setBuffer(loadVoice(folder, "spell2.wav"));
	characterVoices.chain[7].setBuffer(loadVoice(folder, "spell3.wav"));
	characterVoices.chain[8].setBuffer(loadVoice(folder, "spell4.wav"));
	characterVoices.chain[9].setBuffer(loadVoice(folder, "spell5.wav"));
	characterVoices.chain[10].setBuffer(loadVoice(folder, "counter.wav"));
	characterVoices.chain[11].setBuffer(loadVoice(folder, "counterspell.wav"));
	characterVoices.damage1.setBuffer(loadVoice(folder, "damage1.wav"));
	characterVoices.damage2.setBuffer(loadVoice(folder, "damage2.wav"));
	characterVoices.choose.setBuffer(loadVoice(folder, "choose.wav"));
	characterVoices.fever.setBuffer(loadVoice(folder, "fever.wav"));
	characterVoices.feversuccess.setBuffer(loadVoice(folder, "feversuccess.wav"));
	characterVoices.feverfail.setBuffer(loadVoice(folder, "feverfail.wav"));
	characterVoices.lose.setBuffer(loadVoice(folder, "lose.wav"));
	characterVoices.win.setBuffer(loadVoice(folder, "win.wav"));
}

void Player::playerSetup(FieldProp& properties, int playernum, int playerTotal)
{
	// Right side square setup
	int width = static_cast<int>(ceil(sqrt(static_cast<double>(playerTotal - 1))));
	
	if (playernum == 1)
	{
		// Player 1
		m_globalScale = 1;
		properties.offsetX = 48;
		properties.offsetY = 84;
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);

		// Set nextpuyo
		m_nextPuyoOffsetX = properties.offsetX + (properties.gridX * properties.gridWidth * properties.scaleX + 10) * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, true, data);

		// Set background
		m_fieldSprite.setImage(data->imgField1); // Can also be character related

		// Set up border
		m_borderSprite.setImage(data->imgBorder1);

		// Fever gauge
		feverGauge.init(-8.f, static_cast<float>(properties.gridHeight * (properties.gridY - 3 - 0)), 1.f, true, data);
	}
	else if (playernum > 1)
	{
		// Other players
		m_globalScale = 1.0f / static_cast<float>(width);
		properties.offsetX = 400.f + static_cast<float>((playernum - 2) % width) * 320.f * m_globalScale - 75.f * m_globalScale * static_cast<float>(width - 1);
		properties.offsetY = 84.f + static_cast<float>((playernum - 2) / width) * 438.f * m_globalScale - 42.f * m_globalScale * static_cast<float>(width - 1);
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);

		// Set nextpuyo
		m_nextPuyoOffsetX = properties.offsetX - 75 * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, false, data);

		// Set background
		m_fieldSprite.setImage(data->imgField2); // Can also be character related

		// Set up border
		m_borderSprite.setImage(data->imgBorder2);

		// Fever gauge
		feverGauge.init(76, static_cast<float>(properties.gridHeight * (properties.gridY - 3 - 0)), 1.f, false, data);
	}
}

Player::~Player()
{
	delete pchainWord;
	delete cpuAI;
	// Delete light effects
	while (!m_lightEffect.empty())
	{
		delete m_lightEffect.back();
		m_lightEffect.pop_back();
	}

	// Delete other
	while (!m_secondsObj.empty())
	{
		delete m_secondsObj.back();
		m_secondsObj.pop_back();
	}

	delete statusText;
	delete statusFont;
	delete m_randomizerNuisanceDrop;
}

void Player::setPlayerType(const PlayerType playertype)
{
	m_type = playertype;
	controls.init(m_playernum, m_type, currentgame->settings->recording);
}

void Player::setRandomSeed(unsigned long seed_in, PuyoRNG** randomizer)
{
	delete *randomizer;
	*randomizer = MakePuyoRNG(currentgame->legacyRandomizer == true ? "legacy" : "classic", seed_in, 0);
}

// Randomizer for next puyo list
int Player::getRandom(int in, PuyoRNG* randomizer)
{
	return randomizer->next(in);
}

// Set character and initialize all related values (voice, animation, background)
void Player::setCharacter(PuyoCharacter pc, bool show)
{
	m_character = pc;
	initVoices();
	const PosVectorFloat offset(
		activeField->getProperties().centerX * 1,
		activeField->getProperties().centerY / 2.0f * 1
	);
	const std::string currentCharacter = currentgame->settings->characterSetup[pc];
	if (currentgame->players.size() <= 10)
		characterAnimation.init(data, offset, 1, folder_user_character + currentCharacter + std::string("/Animation/"));
	else
		characterAnimation.init(data, offset, 1, "");
	if (currentgame->settings->useCharacterField)
		setFieldImage(pc);

	if (!show) return;

	currentCharacterSprite.setImage(data->front->loadImage((folder_user_character + currentgame->settings->characterSetup[pc] + "/icon.png").c_str()));
	currentCharacterSprite.setCenter();
	currentCharacterSprite.setPosition(charHolderSprite.getPosition() + PosVectorFloat(1, 1) - PosVectorFloat(2, 4)); // Correct for shadow
	currentCharacterSprite.setVisible(true);
	currentCharacterSprite.setScale(m_globalScale);
	charHolderSprite.setVisible(true);
	showCharacterTimer = 5 * 60;
	for (auto& i : dropset)
	{
		i.setImage(data->front->loadImage("Data/CharSelect/dropset.png"));
	}
	setDropsetSprite(static_cast<int>(currentCharacterSprite.getX()), static_cast<int>(currentCharacterSprite.getY() + 60.f * m_globalScale), m_character);
}

void Player::setFieldImage(PuyoCharacter pc)
{
	FeImage* im = data->imgCharField[static_cast<unsigned char>(pc)];
	if (!im || im->error())
	{
		m_fieldSprite.setImage(m_playernum == 1 ? data->imgField1 : data->imgField2);
	}
	else
	{
		m_fieldSprite.setImage(im);
	}
	if (m_nextPuyo.getOrientation() < 0)
	{
		m_fieldSprite.setFlipX(true);
	}
}

// Game code
void Player::play()
{
	// Debugging
	if (debugMode == 1)
	{
		if (m_playernum == 1)
			debugstring = "";
		debugstring += to_string(currentphase) + "\n";
		debugstring += "garbage: " + to_string(forgiveGarbage) + "\n";
		if (activeGarbage == &normalGarbage)
			debugstring += "normal: " + to_string(activeGarbage->GQ) + "\n";
		else
			debugstring += "fever: " + to_string(activeGarbage->GQ) + "\n";

		if (!messages.empty())
			debugstring += std::string("mes: ") + messages.front()[0] + "\n";
	}

	// Pick character (online)
	if (showCharacterTimer > 0)
	{
		showCharacterTimer--;
		charHolderSprite.setTransparency(1);
		currentCharacterSprite.setTransparency(1);
		for (auto& i : dropset)
		{
			i.setTransparency(1);
		}
		if (showCharacterTimer < 120)
		{
			charHolderSprite.setTransparency(static_cast<float>(showCharacterTimer) / 120.0f);
			currentCharacterSprite.setTransparency(static_cast<float>(showCharacterTimer) / 120.0f);
			for (auto& i : dropset)
			{
				i.setTransparency(static_cast<float>(showCharacterTimer) / 120.0f);
			}
		}
	}
	// Play animation for checkmark
	if (rematchIconTimer < 1000)
	{
		rematchIconTimer++;
		rematchIcon.setScale(m_globalScale * static_cast<float>(interpolate("elastic", 2, 1, rematchIconTimer / 60.0, -5, 2)));
	}
	rematchIcon.setVisible(rematch);

	// Process message
	if (m_type == ONLINE)
	{
		processMessage();
	}

	// ===== Global events

	// Animate particles
	activeField->animateParticle();

	// Animate chainword
	pchainWord->move();

	// Animate light effects & secondsobj
	playLightEffect();

	// Do characterAnimation
	characterAnimation.playAnimation();

	// Move nextPuyo
	if (currentphase > 0 && nextPuyoActive)
		m_nextPuyo.play();

	// Animate garbage trays
	normalTray.play();
	feverTray.play();

	// Set scorecounter
	setScoreCounter();

	// Fever mode specific
	playFever();

	// Set Margin Time
	if (currentphase != PICKCOLORS && currentphase != IDLE)
	{
		setMarginTimer();
	}

	// =====PHASE -2: PICK COLORS
	if (currentphase == PICKCOLORS)
	{
		chooseColor();
	}

	// =====PHASE -1: IDLE
	// Possiblity to enter menu? (maybe outside the player object)

	// =====PHASE 0: GETREADY
	if (currentphase == GETREADY)
	{
		getReady();
	}

	// =====PHASE 1: PREPARE
	if (currentphase == PREPARE)
	{
		prepare();
	}

	// =====Phase 10: MOVE (global), the movement phase
	movePuyos.move();
	if (currentphase == MOVE)
	{
		nextPuyoActive = true; // Start moving nextpuyo if it doesnt yet
		hasMoved = true;
	}

	if (currentphase == MOVE && cpuAI)
	{
		// Perform AI movement
		cpuMove();
	}

	// =====Phase 20 :CREATEPUYO
	if (currentphase == CREATEPUYO)
	{
		// CREATEPUYO phase is dropping a colored puyo pair
		activeField->createPuyo();

		// Reset chain number
		chain = 0;
		predictedChain = 0;

		// Set target point
		if (currentgame->currentruleset->marginTime >= 0)
		{
			targetPoint = getTargetFromMargin(currentgame->currentruleset->targetPoint, currentgame->currentruleset->marginTime, margintimer);
		}
	}

	// ==========Phase 21: DROPPUYO
	if (currentphase == DROPPUYO)
	{
		activeField->dropPuyo();
	}

	//==========Phase 22: FALLPUYO (global)
	activeField->fallPuyo();
	activeField->bouncePuyo();
	if (currentphase == FALLPUYO || currentphase == FALLGARBAGE)
	{
		// End phase
		activeField->endFallPuyoPhase();
	}

	// ========== Phase 31: SEARCHCHAIN
	if (currentphase == SEARCHCHAIN)
	{
		activeField->searchChain();
	}

	// ========== Phase 32: DESTROYPUYO
	if (currentphase == DESTROYPUYO)
	{
		destroyPuyos();
	}

	// Global phase 32
	activeField->popPuyoAnim();

	// ========== Phase33-35: GARBAGE
	if (currentphase == GARBAGE)
	{
		garbagePhase();
	}

	// Global: countergarbage, it's outisde garbage phase
	if (attdef == COUNTERATTACK)
	{
		counterGarbage();
	}

	// ========== Phase 40: CHECKALLCLEAR
	if (currentphase == CHECKALLCLEAR)
	{
		checkAllClear();
	}

	// ========== Phase 41: DROPGARBAGE
	if (currentphase == DROPGARBAGE)
	{
		activeField->dropGarbage();
	}

	// ========== Phase 43: CHECKLOSER
	if (currentphase == CHECKLOSER)
	{
		checkLoser(true);
	}

	//==========Phase 45: WAITGARBAGE - it acts the same as dropGarbage, except it waits for a message
	if (currentphase == WAITGARBAGE)
	{
		waitGarbage();
	}

	// ========== Phase 46: WAITCONFIRMGARBAGE
	if (currentphase == WAITCONFIRMGARBAGE)
	{
		confirmGarbage();
	}

	// ========== Phase 50: CHECKFEVER
	if (currentphase == CHECKFEVER)
	{
		checkFever();
	}

	// ========== Phase 51: PREPAREFEVER
	if (currentphase == PREPAREFEVER)
	{
		startFever();
	}

	//==========Phase 52: DROPFEVERCHAIN
	if (currentphase == DROPFEVER)
	{
		dropFeverChain();
	}

	//==========Phase 53: CHECKFEVER
	if (currentphase == CHECKENDFEVER)
	{
		checkEndFever();
	}

	//==========Phase 54: ENDFEVER
	if (currentphase == ENDFEVER)
	{
		endFever();
	}

	//==========Phase 54: CHECKENDFEVERONLINE
	if (currentphase == CHECKENDFEVERONLINE)
	{
		checkEndFeverOnline();
	}

	//==========Phase 60: LOST
	if (currentphase == LOSEDROP)
	{
		loseGame();
	}

	if (currentphase == WAITLOSE)
	{
		waitLose();
	}

	if (losewin == WIN)
	{
		currentphase = IDLE;
		winGame();
	}

	// Play voice (leave at end)
	playVoice();
}

void Player::endPhase()
{
	currentphase = currentgame->currentruleset->endPhase(currentphase, this);
}

// Update nuisance tray
void Player::updateTray(const GarbageCounter* c)
{
	if (!c)
	{
		// Automatic check of active garbage
		if (activeGarbage == &normalGarbage)
		{
			normalTray.update(normalGarbage.GQ + normalGarbage.CQ);
		}
		else
		{
			feverTray.update(feverGarbage.GQ + feverGarbage.CQ);
			if (feverGarbage.GQ + feverGarbage.CQ <= 0)
			{
				normalTray.update(normalGarbage.GQ + normalGarbage.CQ);
			}
		}
	}
	else
	{
		// Not active, but must still be updated
		if (c == &normalGarbage)
		{
			normalTray.update(c->CQ + c->GQ);
		}
		else if (c == &feverGarbage)
		{
			feverTray.update(c->CQ + c->GQ);
		}
	}
}

void Player::playGarbageSound()
{
	if (chain == 3)
	{
		data->snd.nuisance_hitS.play(data);
	}
	else if (chain == 4)
	{
		data->snd.nuisance_hitM.play(data);
	}
	else if (chain == 5)
	{
		data->snd.nuisance_hitL.play(data);
	}
	else if (chain >= 6)
	{
		data->snd.heavy.play(data);
	}
}

void Player::setScoreCounter()
{
	m_scoreCounter.setCounter(scoreVal);
}

void Player::setScoreCounterPB()
{
	m_scoreCounter.setPointBonus(point, bonus);
}

void Player::getReady()
{
	charHolderSprite.setVisible(false);
	currentCharacterSprite.setVisible(false);

	// Wait until timer hits go
	if (m_readyGoTimer >= 120)
	{
		if (currentgame->settings->recording == PVS_RECORDING)
		{
			if (active)
			{
				activeAtStart = 1;
				previousName = onlineName;
			}
			else
			{
				previousName = "";
				activeAtStart = 0;
			}
			controls.recordEvents.clear();
		}
		// Reset controls
		controls.release();
		endPhase();
		m_readyGoTimer = 0;
		return;
	}

	m_readyGoTimer++;
}

void Player::chooseColor()
{
	currentgame->data->matchTimer = 0;

	if (colorMenuTimer == 0)
	{
		return;
	}

	// Add timer
	colorMenuTimer++;

	// Set border
	if (colorMenuTimer == 2)
	{
		menuHeight = 0;
		for (auto& i : colorMenuBorder)
		{
			i.setVisible(true);
		}
	}

	// Disappear
	if (colorMenuTimer == -26)
	{
		for (auto& i : colorMenuBorder)
		{
			i.setVisible(false);
		}
	}

	// Fade in/out
	if (colorMenuTimer < 25 && colorMenuTimer != 0)
	{
		if (colorMenuTimer > 0)
		{
			menuHeight = colorMenuTimer * 6;
		}
		else if (colorMenuTimer < -25)
		{
			menuHeight = 150 - (colorMenuTimer + 50) * 6;
		}

		colorMenuBorder[0].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f - static_cast<float>(menuHeight)));
		colorMenuBorder[1].setPosition((192.f / 2.f) - (88.f - 24.f) - 1, 336.f + (-168.f - static_cast<float>(menuHeight)));
		colorMenuBorder[2].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f - static_cast<float>(menuHeight)));

		colorMenuBorder[3].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f - static_cast<float>(menuHeight) + 24.f) - 4.f);
		colorMenuBorder[4].setPosition((192.f / 2.f) - (88.f - 24.f), 336.f + (-168.f - static_cast<float>(menuHeight) + 24.f) - 4.f);
		colorMenuBorder[5].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f - static_cast<float>(menuHeight) + 24.f) - 4.f);

		colorMenuBorder[6].setPosition((192.f / 2.f) - (88.f + 00.f), 336.f + (-168.f + static_cast<float>(menuHeight)));
		colorMenuBorder[7].setPosition((192.f / 2.f) - (88.f - 24.f) - 1, 336.f + (-168.f + static_cast<float>(menuHeight)));
		colorMenuBorder[8].setPosition((192.f / 2.f) + (88.f - 24.f), 336.f + (-168.f + static_cast<float>(menuHeight)));
		
		colorMenuBorder[1].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[4].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[7].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[1].setScaleY(1);
		colorMenuBorder[4].setScaleY(1);
		colorMenuBorder[7].setScaleY(1);

		colorMenuBorder[3].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[4].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[5].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.f);
		colorMenuBorder[3].setScaleX(1);
		colorMenuBorder[4].setScaleX(1);
		colorMenuBorder[5].setScaleX(1);

		colorMenuBorder[0].setScale(1);
		colorMenuBorder[2].setScale(1);
		colorMenuBorder[6].setScale(1);
		colorMenuBorder[8].setScale(1);
	}

	// Initialize
	if (colorMenuTimer == 25)
	{
		for (int i = 0; i < 5; i++)
		{
			spice[i].setVisible(true);
			spice[i].setTransparency(1);
			spice[i].setPosition(192.f / 2.f, 336.f + static_cast<float>(-168 - 125 + 37 + 50 * i));
		}
	}

	// Make choice
	for (int i = 0; i < 5; i++)
	{
		if (spiceSelect == i)
		{
			spice[i].setScale(spice[i].getScaleX() + (1 - spice[i].getScaleX()) / 2.0f);
			spice[i].setColor(255, 255, 255);
		}
		else
		{
			spice[i].setScale(0.75);
			spice[i].setColor(128, 128, 128);
		}
	}

	// Player 1 takes over CPU choice
	takeover = false;
	for (auto& player : currentgame->players)
	{
		// Find first CPU player
		if (player->takeover) // No other takeovers
			break;

		if (currentgame->players[0]->pickedColor &&
			player->getPlayerType() == CPU &&
			!player->pickedColor &&
			player == this)
		{
			takeover = true;
			break;
		}
	}

	if (takeover)
	{
		controls.A = currentgame->players[0]->controls.A;
		controls.Down = currentgame->players[0]->controls.Down;
		controls.Up = currentgame->players[0]->controls.Up;
	}

	// Select option
	if (colorMenuTimer > 0)
	{
		bool select = false;
		if (controls.Down == 1 && spiceSelect < 4)
		{
			spiceSelect++;
			data->snd.cursor.play(data);
			select = true;
			// Send message
			if (currentgame->connected)
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("s|") + to_string(spiceSelect), currentgame->channelName);
		}
		if (controls.Up == 1 && spiceSelect > 0)
		{
			spiceSelect--;
			data->snd.cursor.play(data);
			select = true;
			// Send message
			if (currentgame->connected && m_type == HUMAN)
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("s|") + to_string(spiceSelect), currentgame->channelName);
		}

		// Get online message
		if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 's')
		{
			sscanf(messages.front().c_str(), "s|%i", &spiceSelect);
			select = true;
			messages.pop_front();
		}

		if (select || colorMenuTimer == 25)
		{
			// Put this in
			select = false;
			if (spiceSelect == 0)
			{
				colors = 3;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 1)
			{
				colors = 3;
				normalGarbage.CQ = 12;
			}
			else if (spiceSelect == 2)
			{
				colors = 4;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 3)
			{
				colors = 5;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 4)
			{
				colors = 5;
				normalGarbage.CQ = 12;
			}
			updateTray();
		}

		// Automatic choice
		if (currentgame->colorTimer == 1 && colorMenuTimer > 25 && pickedColor == false)
		{
			controls.A = 1;
		}

		// Make choice
		if (controls.A == 1 && colorMenuTimer > 25)
		{
			normalGarbage.GQ += normalGarbage.CQ;
			normalGarbage.CQ = 0;
			data->snd.decide.play(data);
			controls.A++;
			if (takeover)
			{
				currentgame->players[0]->controls.A++;
			}
			currentgame->colorTimer = 0;
			colorMenuTimer = -50;
			pickedColor = true;
			for (auto& i : spice)
			{
				i.setVisible(false);
			}

			// Re-init colors
			initNextList();

			// Reset takeover if necessary
			takeover = false;

			// Send message
			if (currentgame->connected && m_type == HUMAN)
			{
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("c|") + to_string(spiceSelect), currentgame->channelName);
			}
		}
		// Get choice from online
		if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'c')
		{
			sscanf(messages.front().c_str(), "c|%i", &spiceSelect);
			if (spiceSelect == 0)
			{
				colors = 3;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 1)
			{
				colors = 3;
				normalGarbage.CQ = 12;
			}
			else if (spiceSelect == 2)
			{
				colors = 4;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 3)
			{
				colors = 5;
				normalGarbage.CQ = 0;
			}
			else if (spiceSelect == 4)
			{
				colors = 5;
				normalGarbage.CQ = 12;
			}
			updateTray();

			messages.pop_front();
			normalGarbage.GQ += normalGarbage.CQ;
			normalGarbage.CQ = 0;
			data->snd.decide.play(data);
			colorMenuTimer = -50;
			pickedColor = true;

			for (auto& i : spice)
			{
				i.setVisible(false);
			}

			// Re-init colors
			initNextList();
		}
	}

	// Check if everyone picked colors
	if (colorMenuTimer == -2)
	{
		bool start = true;
		for (auto& player : currentgame->players)
		{
			if (!player->active && !currentgame->settings->useCPUplayers)
			{
				continue;
			}
			if (!player->pickedColor || player->colorMenuTimer < -2)
			{
				start = false;
				break;
			}
		}

		if (start)
		{
			colorMenuTimer = 0;
			for (auto& player : currentgame->players)
			{
				if (!player->active && !currentgame->settings->useCPUplayers)
					continue;
				player->currentphase = GETREADY;
			}
			currentgame->readyGoObj.prepareAnimation("readygo");
			currentgame->data->matchTimer = 0;
		}
	}
	// Fade out with flicker
	if (colorMenuTimer < 0)
	{
		spice[spiceSelect].setVisible(false);
	}
	if (colorMenuTimer < -20)
	{
		spice[spiceSelect].setVisible(true);
		spice[spiceSelect].setTransparency(static_cast<float>(interpolate("linear", 1, 0, (colorMenuTimer + 50) / 30.0, 0, 0)));
	}
}

void Player::prepare()
{
	// Read colors and pop front after reading
	const int color1 = *m_nextList.begin();
	const int color2 = *(m_nextList.begin() + 1);
	addNewColorPair(colors);
	popColor();

	MovePuyoType mpt = DOUBLET;
	if (useDropPattern)
	{
		mpt = getFromDropPattern(m_character, turns);
	}
	movePuyos.prepare(mpt, this, color1, color2);

	if (cpuAI)
	{
		// Determine best chain
		cpuAI->prepare(mpt, color1, color2);
		cpuAI->pinch = activeGarbage->GQ > 0 ? true : false;
		cpuAI->findLargest();
	}

	// Release controls for ONLINE player
	if (m_type == ONLINE)
	{
		controls.release();
	}

	endPhase();
	poppedChain = false;
	createPuyo = false;
	forgiveGarbage = false;
}

void Player::cpuMove()
{
	// Get current rotation
	if (movePuyos.getType() == BIG)
	{
		if (movePuyos.getColorBig() != cpuAI->bestRot)
		{
			controls.A = cpuAI->timer % 12;
		}
	}
	else
	{
		if (movePuyos.getRotation() != cpuAI->bestRot)
		{
			controls.A = cpuAI->timer % 12;
		}
	}

	if (cpuAI->timer > 30)
	{
		controls.Down++;
	}

	cpuAI->timer++;

	if (movePuyos.getPosX1() == cpuAI->bestPos)
	{
		controls.A = 0;
		controls.Left = 0;
		controls.Right = 0;
		return;
	}

	if (movePuyos.getPosX1() < cpuAI->bestPos && cpuAI->timer > 10)
	{
		controls.Right++;
	}

	if (movePuyos.getPosX1() > cpuAI->bestPos && cpuAI->timer > 10)
	{
		controls.Left++;
	}

	if (controls.Left > 0 && controls.Right > 0)
	{
		controls.Left = 0;
		controls.Right = 0;
	}

}

void Player::destroyPuyos()
{
	// Phase 31
	destroyPuyosTimer++;
	if (destroyPuyosTimer == chainPopSpeed)
	{
		// Create "XX Chain" word
		FieldProp prop = activeField->getProperties();
		pchainWord->showAt(
			prop.gridWidth * rememberX * prop.scaleX,
			prop.gridHeight * (prop.gridY - 3 - (rememberMaxY + 3)) * prop.scaleY, chain
		);

		// Play sound
		data->snd.chain[min(chain - 1, 6)].play(data);
	}

	if (destroyPuyosTimer == chainPopSpeed)
	{
		// Set EQ
		if (chain >= currentgame->currentruleset->requiredChain)
		{
			float power = 1;
			int div = 2;
			if (feverMode)
			{
				power = currentgame->currentruleset->feverPower;
			}
			if (currentgame->players.size() > 1)
			{
				div = max(2, divider);
			}

			EQ = static_cast<int>(static_cast<float>(currentScore / targetPoint) * power * 3.f / static_cast<float>(div + 1));
			currentScore -= targetPoint * (currentScore / targetPoint);

			if (bonusEQ)
			{
				EQ += 1;
				bonusEQ = false;
			}
		}
		else
		{
			currentScore = 0;
		}
		endPhase();
	}
}

// Light effect & secondsobj
void Player::playLightEffect()
{
	for (auto& i : m_lightEffect)
	{
		i->setTimer(garbageSpeed);
	}

	// Check if any should be removed
	for (size_t i = 0; i < m_lightEffect.size(); i++)
	{
		if (m_lightEffect[i]->getTimer() > 300)
		{
			delete m_lightEffect[i];
			m_lightEffect.erase(std::remove(m_lightEffect.begin(), m_lightEffect.end(), m_lightEffect[i]), m_lightEffect.end());
		}
	}
	// Play feverlight
	m_feverLight.setTimer(0.1f);

	// Update fevergauge
	feverGauge.update();

	// Secondsobj
	for (auto& i : m_secondsObj)
	{
		i->move();
	}

	// Check if any should be removed
	for (size_t i = 0; i < m_secondsObj.size(); i++)
	{
		if (m_secondsObj[i]->getTimer() > 50)
		{
			delete m_secondsObj[i];
			m_secondsObj.erase(std::remove(m_secondsObj.begin(), m_secondsObj.end(), m_secondsObj[i]), m_secondsObj.end());
		}
	}


}

void Player::addFeverCount()
{
	if (feverGauge.getCount() == 7)
	{
		return;
	}

	int dir = m_nextPuyo.getOrientation();
	PosVectorFloat startpv, middlepv, endpv;
	startpv = PosVectorFloat(properties.offsetX + (192 / 2) * properties.scaleX * getGlobalScale(), properties.offsetY);
	endpv = PosVectorFloat(m_nextPuyoOffsetX + (feverGauge.getPV().x) * m_nextPuyoScale, m_nextPuyoOffsetY + (feverGauge.getPV().y) * m_nextPuyoScale);
	middlepv.x = startpv.x + static_cast<float>(dir * PUYOX) * 1.5f;
	middlepv.y = startpv.y - static_cast<float>(PUYOY) * 1.5f;

	m_feverLight.init(startpv, middlepv, endpv);
	feverGauge.addCount();
}

// Total garbage including fever garbage
int Player::getGarbageSum() const
{
	return normalGarbage.CQ + normalGarbage.GQ + feverGarbage.CQ + feverGarbage.GQ;
}

// Total attack amount
int Player::getAttackSum() const
{
	const int attack = EQ - max(getGarbageSum(), 0);  //----> do not count negative GQ
	return attack > 0 ? attack : 0;
}

void Player::addAttacker(GarbageCounter* gc, Player* pl) const
{
	if (const auto it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl); it != gc->accumulator.end())
	{
		return;
	}
	gc->accumulator.push_back(pl);
}

void Player::removeAttacker(GarbageCounter* gc, Player* pl)
{
	// Found element
	if (const auto it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl); it != gc->accumulator.end())
	{
		gc->accumulator.erase(it);
		lastAttacker = pl;
	}
}

// Check if any attackers left in accumulator
void Player::checkAnyAttacker(GarbageCounter* gc)
{
	// Loop through attackers and check their state to be sure
	std::vector<Player*> remove;
	for (auto& i : gc->accumulator)
	{
		// Player still alive?
		if (i->losewin != NOWIN || !i->active)
			remove.push_back(i);
	}

	while (!remove.empty())
	{
		removeAttacker(gc, remove.back());
		remove.pop_back();
	}

	if (!gc->accumulator.empty())
	{
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
	if (gc->accumulator.empty())
	{
		gc->GQ += gc->CQ;
		gc->CQ = 0;
	}
}

void Player::resetNuisanceDropPattern()
{
	const int fieldWidth = properties.gridX;
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
	double rand = m_randomizerNuisanceDrop->genrand_real1();
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
	garbageTimer = 0;

	// Calculate DEF (CQ+GQ+sum(EQ))
	int totalEQ = 0;
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		// Sum opponents attack
		if (currentgame->players[i] != this)
			totalEQ += currentgame->players[i]->getAttackSum();
	}
	const int DEF = getGarbageSum() + totalEQ;

	// Set targets on the first run of startgarbage phase
	for (auto& player : currentgame->players)
	{
		if (player != this && chain == 1)
		{
			targetGarbage[player] = player->activeGarbage;
		}
	}

	// Attack or defend?
	if (DEF <= 0 && EQ > 0)
	{
		attdef = ATTACK;
		for (auto& player : currentgame->players)
		{
			if (player != this)
			{
				// Add any negative GQ
				if (normalGarbage.GQ + feverGarbage.GQ < 0)
				{
					EQ += -1 * (normalGarbage.GQ + feverGarbage.GQ);
					normalGarbage.GQ = 0;
					feverGarbage.GQ = 0;
					updateTray();
				}

				// Create light effect animation for each opponent
				PosVectorFloat startpv, middlepv, endpv;
				startpv = activeField->getGlobalCoord(rememberX, rememberMaxY);
				endpv = player->activeField->getTopCoord(-1);
				// Calculate a middle
				const int dir = m_nextPuyo.getOrientation();
				middlepv.x = startpv.x - dir * PUYOX * 3;
				middlepv.y = startpv.y - PUYOY * 3;
				m_lightEffect.push_back(new LightEffect(data, startpv, middlepv, endpv));

				// Add self to garbage accumulator
				player->addAttacker(targetGarbage[player], this);
			}
		}
	}
	else if (DEF > 0)
	{
		// Calculate COUNTER = CQ+GQ+sum(EQ) - EQ
		if (const int COUNTER = getGarbageSum() + totalEQ - EQ; COUNTER >= 0)
		{
			// Normal defense
			attdef = DEFEND;

			// Remove self from opponent garbage accumulator (if present)
			for (auto& player : currentgame->players)
			{
				if (player == this) continue;
				player->removeAttacker(targetGarbage[player], this);
				player->checkAnyAttacker(targetGarbage[player]);
			}
		}
		else
		{
			// Countering defense
			// Wait for defense to end before making next move
			attdef = COUNTERDEFEND;

			// Counter attack voice
			if (playvoice > 4 && chain >= 3 && currentgame->currentruleset->voicePatternFever == true)
			{
				playvoice = 11;
			}
			if (playvoice <= 4 && chain >= 3 && currentgame->currentruleset->voicePatternFever == true)
			{
				playvoice = 10;
			}
		}
		// Create light effect for defense
		PosVectorFloat startpv = activeField->getGlobalCoord(rememberX, rememberMaxY);
		PosVectorFloat endpv = activeField->getTopCoord(-1);
		// Calculate a middle
		const int dir = m_nextPuyo.getOrientation();
		PosVectorFloat middlepv;
		middlepv.x = startpv.x - dir * PUYOX * 3;
		middlepv.y = startpv.y - PUYOY * 3;
		m_lightEffect.push_back(new LightEffect(data, startpv, middlepv, endpv));

	}
	else
	{
		attdef = NOATTACK;
	}
}

void Player::garbagePhase()
{
	// Phase 33
	// Stop chaining
	if (currentgame->stopChaining)
	{
		return;
	}

	if (garbageTimer == 0)
	{
		hasMoved = false;

		// Trigger voice
		if (destroyPuyosTimer == chainPopSpeed)
		{
			prepareVoice(chain, predictedChain);
		}

		startGarbage();

		// Trigger animation
		if (destroyPuyosTimer == chainPopSpeed && playvoice >= 5)
		{
			if (playvoice < 10)
				characterAnimation.prepareAnimation(std::string("spell") + to_string(playvoice - 4));
			else if (playvoice == 11)
				characterAnimation.prepareAnimation(std::string("counter"));

		}
	}

	// Change garbageTimer
	garbageTimer += garbageSpeed;

	if (garbageTimer > garbageEndTime)
	{
		endGarbage();
		garbageTimer = 0;
		endPhase();
	}
}

void Player::counterGarbage()
{
	// Almost the same as normal garbage phase, except no endphase
	garbageTimer += garbageSpeed;
	if (garbageTimer > garbageEndTime)
	{
		endGarbage();
		garbageTimer = 0;
	}
}

void Player::endGarbage()
{
	// Play sound
	if (EQ > 0 && !m_lightEffect.empty())
	{
		data->snd.hit.play(data);
	}
	
	// Attack animation ends: check if any garbage sneaked past after all
	if (attdef == ATTACK || attdef == COUNTERATTACK)
	{
		// Bad prediction of attack: there is garbage on your side after all and must be defended
		if (getGarbageSum() > 0 && (normalGarbage.GQ > 0 || feverGarbage.GQ > 0))
		{
			// Defend
			activeGarbage->GQ -= EQ;
			// Push defense through normal GQ if necessary?
			// Be careful here! don't let normal.GQ go negative
			if (activeGarbage == &feverGarbage && normalGarbage.GQ > 0 && activeGarbage->GQ < 0)
			{
				normalGarbage.GQ += feverGarbage.GQ;
				feverGarbage.GQ = 0;
				if (normalGarbage.GQ < 0)
				{
					// Don't let normal GQ go negative!
					feverGarbage.GQ += normalGarbage.GQ;
					normalGarbage.GQ = 0;
				}
				updateTray(&normalGarbage);
			}
			updateTray();
			// This part is different from real defense
			if (getGarbageSum() >= 0)
			{
				// Garbage left: all is OK, defend ends.
				EQ = 0;
				attdef = NOATTACK;
			}
			else
			{
				// Garbage is countered: put leftover back in EQ and continue attack, do not create counterattack animation!
				EQ = -1 * (getGarbageSum());
				normalGarbage.GQ = 0; normalGarbage.CQ = 0;
				feverGarbage.GQ = 0; feverGarbage.CQ = 0;
				updateTray();
			}
		}
	}

	// Normal Attack
	// Add EQ to opponents CQ
	if (attdef == ATTACK || attdef == COUNTERATTACK)
	{
		for (auto& player : currentgame->players)
		{
			if (player == this) continue;

			targetGarbage[player]->CQ += EQ;
			player->updateTray(targetGarbage[player]);
			playGarbageSound();
			// End of attack
			if (chain == predictedChain || (attdef == COUNTERATTACK && hasMoved))
			{
				// Remove as accumulator
				player->removeAttacker(targetGarbage[player], this);

				// Force player to check if zero (there should always be a last player)
				player->checkAnyAttacker(targetGarbage[player]);
			}
			player->checkFeverGarbage();
		}

		// End of attack
		currentgame->currentruleset->onAttack(this);
		attdef = NOATTACK;
	}

	// Defense end
	if (attdef == DEFEND || attdef == COUNTERDEFEND)
	{
		activeGarbage->GQ -= EQ;

		// Push defense through normal GQ if necessary?
		// Be careful here! don't let normal.GQ go negative
		if (activeGarbage == &feverGarbage && normalGarbage.GQ > 0 && activeGarbage->GQ < 0)
		{
			normalGarbage.GQ += feverGarbage.GQ;
			feverGarbage.GQ = 0;
			if (normalGarbage.GQ < 0)
			{
				// Don't let normal GQ go negative!
				feverGarbage.GQ += normalGarbage.GQ;
				normalGarbage.GQ = 0;
			}
			updateTray(&normalGarbage);
		}
		updateTray();

		// Online players will not offset (?)
		if (m_type != ONLINE)
		{
			currentgame->currentruleset->onOffset(this);
		}

		// Send message
		if (currentgame->connected && m_type == HUMAN)
		{
			currentgame->network->sendToChannel(CHANNEL_GAME, "fo", currentgame->channelName);
		}

		// THERE IS NO SUCH THING AS DEFENSE AND COUNTERDEFENSE
		// Always check if defense can be countered at end!!
		attdef = COUNTERDEFEND;
	}

	// Counter defense
	if (attdef == COUNTERDEFEND)
	{
		// Calculate DEF again
		int totalEQ = 0;
		for (const auto& player : currentgame->players)
		{
			if (player == this) continue;

			// Sum opponents EQ
			totalEQ += player->getAttackSum();
		}

		if (const int DEF = getGarbageSum() + totalEQ; DEF >= 0)
		{
			// Ineffective counter -> just end counter
			attdef = NOATTACK;
		}
		else
		{
			// Turn into attack
			attdef = COUNTERATTACK;
			for (auto& player : currentgame->players)
			{
				if (player == this) continue;

				if (getGarbageSum() < 0) // No need to check again?
				{
					EQ = -1 * (getGarbageSum());
					normalGarbage.GQ = 0; normalGarbage.CQ = 0;
					feverGarbage.GQ = 0; feverGarbage.CQ = 0;
					updateTray();
				}

				// Create new light effect animation for each opponent
				PosVectorFloat startpv, middlepv, endpv;
				startpv = activeField->getTopCoord(-1);
				endpv = player->activeField->getTopCoord(-1);

				// Calculate a middle
				const int dir = m_nextPuyo.getOrientation();
				middlepv.x = startpv.x - dir * PUYOX * 3;
				middlepv.y = startpv.y - PUYOY * 3;
				m_lightEffect.push_back(new LightEffect(data, startpv, middlepv, endpv));

				// Add self to garbage accumulator
				player->addAttacker(targetGarbage[player], this);
			}
		}
	}

	// Reset EQ
	if (attdef != COUNTERATTACK)
	{
		EQ = 0;
	}

}

// Adds 2 new colors to nextlist and update nextpuyo
void Player::addNewColorPair(int n)
{
	int color1 = this->getRandom(n, m_randomizerNextList);
	int color2 = this->getRandom(n, m_randomizerNextList);

	// Perform TGM randomizer algorithm?
	if (currentgame->legacyRandomizer)
	{
		color1 = TGMR(color1, n);
		color2 = TGMR(color2, n);
	}

	m_nextList.push_back(color1);

	// Regenerate color 2 if type is quadruple and colors are the same
	if (useDropPattern)
	{
		while (color1 == color2 && getFromDropPattern(m_character, turns + 3) == QUADRUPLET)
		{
			color2 = this->getRandom(n, m_randomizerNextList);
		}
	}
	m_nextList.push_back(color2);

	// Update nextPuyo
	m_nextPuyo.update(m_nextList, useDropPattern ? m_character : ARLE, turns);
}

// Clear front of deque
void Player::popColor()
{
	if (m_nextList.size() <= 6)
	{
		return;
	}
	m_nextList.pop_front();
	m_nextList.pop_front();
}

// Check if color exists in last generated colors
int Player::TGMR(int color, int n)
{
	// Do not apply TGMR if nextlist on the first 4 colors
	if (m_nextList.size() >= 4)
	{
		for (auto i = m_nextList.begin(); i != m_nextList.begin() + 4; ++i)
		{
			if (*i == color)
			{
				color = this->getRandom(n, m_randomizerNextList);
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

	if (activeField->count() == 0)
	{
		// Play sound
		data->snd.allcleardrop.play(data);

		// Do something according to rules
		currentgame->currentruleset->onAllClear(this);
	}
	
	endPhase();
}

void Player::checkLoser(bool endphase)
{
	// Check if player places puyo on spawnpoint
	// Doesn't necessarily mean losing, but the phase will change according to rules
	PosVectorInt spawn = movePuyos.getSpawnPoint();
	bool lose = false;
	bool lose2 = false;
	lose = activeField->isPuyo(spawn.x, spawn.y);

	// Is there a 2nd spawnpoint?
	if (currentgame->currentruleset->doubleSpawn)
	{
		lose2 = activeField->isPuyo(spawn.x + 1, spawn.y);
	}

	if (lose || lose2)
	{
		// Player loses
		if (m_type != ONLINE)
		{
			setLose();
			// Online game: send message to others
			if (currentgame->connected)
			{
				currentgame->network->sendToChannel(CHANNEL_GAME, "l", currentgame->channelName.c_str());
			}
		}
		else
		{
			// Wait for message to arrive
			currentphase = WAITLOSE;
		}
	}
	else if (endphase)
	{
		endPhase();
	}
}

void Player::checkWinner()
{
	// Player wins if all others have lost
	int Nlosers = 0;
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		if (currentgame->players[i] != this && currentgame->players[i]->losewin == LOSE)
		{
			if (!currentgame->players[i]->active)
				continue;
			Nlosers++;
		}
	}

	// A winner is you
	if (losewin == NOWIN && Nlosers == currentgame->countActivePlayers() - 1 && Nlosers != 0 && active)
	{
		// Save replay
		currentgame->saveReplay();
		wins++;

		currentgame->currentruleset->onWin(this);
		characterAnimation.prepareAnimation("win");

		// Ranked match: prepare to disconnect
		prepareDisconnect();

	}

	// Only player left after everyone else left the game
	if (currentgame->countActivePlayers() == 1 && currentgame->players.size() > 1 && active)
	{
		currentgame->currentruleset->onWin(this);
		characterAnimation.prepareAnimation("win");
	}

}

void Player::setLose()
{
	currentgame->currentruleset->onLose(this);
	characterAnimation.prepareAnimation("lose");

	// Play lose sound sound
	int activeplayers = 0;
	for (const auto& player : currentgame->players)
	{
		// Check if all players are in win or lose state
		if (player->losewin != NOWIN)
			continue;

		activeplayers++;
	}

	if (currentgame->players.size() == 1 || activeplayers > 1 && !currentgame->players.empty())
	{
		data->snd.lose.play(data);
	}
}

void Player::loseGame()
{
	// Force others to check if winner
	// Unless he's in the middle of chaining
	if (m_loseWinTimer == 0)
	{
		for (auto& player : currentgame->players)
		{
			if (player != this)
			{
				if (!(player->currentphase > 10 && player->currentphase < 40))
					player->checkWinner();
			}
		}
	}
	
	// Drop puyos
	activeField->loseDrop();

	// Increment losetimer
	m_loseWinTimer++;

	if (m_loseWinTimer == 60)
	{
		characterVoices.lose.play(data);
	}
	
	// Online: wait for confirm message from everyone
	if (losewin == LOSEWAIT)
	{
		// Count confirmed
		int count = 0;
		for (const auto& player : currentgame->players)
		{
			if (player != this && player->active && player->loseConfirm)
				count++;
		}
		// Everybody confirmed your loss-> your state goes to lose
		if (count == currentgame->countActivePlayers() - 1)
		{
			losewin = LOSE;

			// Again, check anyone else if winner
			for (const auto& player : currentgame->players)
			{
				if (player == this)
				{
					continue;
				}

				if (player->currentphase > 10 && player->currentphase < 40)
				{
					continue;
				}

				player->checkWinner();
			}
			
			// Ranked match: report loss
			if (currentgame->settings->rankedMatch && m_type == HUMAN && currentgame->connected)
			{
				currentgame->network->sendToServer(9, "score");
				// Max wins reached
				prepareDisconnect();
			}
		}
	}
}

void Player::winGame()
{
	m_loseWinTimer++;
	if (m_loseWinTimer == 120)
	{
		characterVoices.win.play(data);
	}
}

void Player::checkFever()
{
	// Check if fevercount == 7
	if (feverGauge.getCount() == 7 && !feverMode)
	{
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);

		// Clear field
		m_fieldFever.clearField();

		// All clear bonus: add feverchainamount and add time
		if (allClear == 1)
		{
			allClear = 0;
			allclearTimer = 1;
			currentFeverChainAmount += 2;
			if (feverGauge.seconds < 60 * feverGauge.maxSeconds)
			{
				feverGauge.addTime(5 * 60);
				showSecondsObj(5 * 60);
			}
		}
	}

	// Wait until light has reached fevergauge
	if (feverGauge.getCount() == 7 && feverGauge.fullGauge())
	{
		feverMode = true;
		feverEnd = false;
		endPhase();
	}
	else if (feverGauge.getCount() != 7)
	{
		// No fever mode
		// All clear bonus: drop a 4 chain
		if (allClear == 1)
		{
			activeField->dropField(getFeverChain(getRandom(currentgame->currentruleset->NFeverChains, m_randomizerNextList), colors, 4, getRandom(colors, m_randomizerFeverColor)));
			forgiveGarbage = true;
			allclearTimer = 1;
			if (feverGauge.seconds < 60 * feverGauge.maxSeconds)
			{
				feverGauge.addTime(5 * 60);
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
	if (m_transitionTimer >= 50 && m_transitionTimer < 150)
	{
		m_transitionTimer += 3;
	}
	if (m_transitionTimer < 50)
	{
		m_transitionTimer += 2;
	}

	if (m_transitionTimer > 150)
	{
		poppedChain = false;
		forgiveGarbage = true;
		m_transitionTimer = 150;
		m_transformScale = 1;
		chainPopSpeed = 11;
		garbageSpeed = 8.0;
		garbageEndTime = 80;
		puyoBounceEnd = 40;
		puyoBounceSpeed = 2;
		normalTray.align(activeField->getProperties().offsetX - (16) * m_globalScale, activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		normalTray.setDarken(true);
		feverSuccess = 0;
		endPhase();
	}

	// Sound & animation
	if (m_transitionTimer == 50)
	{
		data->snd.fever.play(data);
		characterVoices.fever.play(data);
		characterAnimation.prepareAnimation("fever");
		if (data->windowFocus)
		{
			data->front->musicEvent(MusicEnterFever);
			currentgame->currentVolumeFever = 0;
		}
	}

	// Switch fields now
	if (m_transitionTimer == 101)
	{
		activeField = &m_fieldFever;
		activeGarbage = &feverGarbage;
	}

	// Rotation animation
	if (m_transitionTimer > 50 && m_transitionTimer <= 100)
	{
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		// Rotate out normal field
		m_transformScale = 1.f - static_cast<float>(m_transitionTimer - 50) / 50.f;
		activeField->setTransformScale(1.f - static_cast<float>(m_transitionTimer - 50) / 50.f);

	}
	else if (m_transitionTimer > 100 && m_transitionTimer <= 150)
	{
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Rotate in fever field
		m_transformScale = static_cast<float>(m_transitionTimer - 100) / 50.f;
		activeField->setTransformScale(static_cast<float>(m_transitionTimer - 100) / 50.f);
	}
}
void Player::checkEndFeverOnline()
{
	// Online player gets stuck in this state until message is received
	if (!messages.empty() && messages.front() == "fe")
	{
		// Fever ends
		messages.pop_front();
		feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Check loser at the last moment
		if (currentgame->currentruleset->feverDeath)
		{
			checkLoser(false); // Do not end phase in this function
		}
		endPhase();
	}
	else if (!messages.empty() && messages.front() == "fc")
	{
		// Fever continues;
		messages.pop_front();
		endPhase();
	}
}
void Player::checkEndFever()
{
	// Check if seconds == 0
	if (feverGauge.seconds == 0 && feverMode && m_type != ONLINE) // Online players end fever when they give a message
	{
		feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);

		// Send message BEFORE sending possible lose message
		if (m_type == HUMAN && currentgame->connected)
		{
			currentgame->network->sendToChannel(CHANNEL_GAME, "fe", currentgame->channelName);
		}

		// Check loser at the last moment?
		if (currentgame->currentruleset->feverDeath)
		{
			checkLoser(false); // Do not end phase in this function
		}
	}

	// Not the end?
	if (!feverEnd && m_type == HUMAN && currentgame->connected)
	{
		currentgame->network->sendToChannel(CHANNEL_GAME, "fc", currentgame->channelName);
	}

	// Check if player popped a chain
	if (poppedChain)
	{
		// Update chains
		if (predictedChain >= currentFeverChainAmount)
		{
			// Success
			currentFeverChainAmount = predictedChain + 1;
			feverSuccess = 1;
		}
		else if (predictedChain != currentFeverChainAmount - 1)
		{
			// Failure
			currentFeverChainAmount -= currentFeverChainAmount - predictedChain > 2 ? 2 : 1;
			feverSuccess = 2;
		}

		// Time bonus
		if (predictedChain > 2 && feverGauge.seconds != 0 && feverGauge.seconds < 60 * feverGauge.maxSeconds)
		{
			feverGauge.addTime((predictedChain - 2) * 30);
			showSecondsObj((predictedChain - 2) * 30);
		}

		// All clear bonus
		if (allClear == 1)
		{
			allClear = 0;
			allclearTimer = 1;
			currentFeverChainAmount += 2;
			if (feverGauge.seconds < 60 * 30)
			{
				// Do not add time if fever is actually ending
				if (feverGauge.seconds != 0)
				{
					feverGauge.addTime(5 * 60);
					showSecondsObj(5 * 60);
				}
			}
		}
		// Minimum and maximum fever chain
		if (currentFeverChainAmount < 3)
		{
			currentFeverChainAmount = 3;
		}
		if (currentFeverChainAmount > 15)
		{
			currentFeverChainAmount = 15;
		}

		// Clear field
		if (!feverEnd)
		{
			activeField->throwAwayField();
		}
	}
	endPhase();
}

// Transition out of fever
void Player::endFever()
{
	if (m_transitionTimer < 100)
	{
		m_transitionTimer += 3;
	}
	else
	{
		m_transitionTimer = 100;
		m_transformScale = 1;
		chainPopSpeed = 25;
		garbageSpeed = 4.8f;
		garbageEndTime = 100;
		puyoBounceEnd = 2;
		puyoBounceEnd = 50;
		endPhase();
	}

	// Switch fields now
	if (m_transitionTimer == 51)
	{
		feverMode = false;

		// Music: check if anyone is in fever
		bool stopMusic = true;
		for (const auto& player : currentgame->players)
		{
			if (player->feverMode == true && player->losewin == NOWIN && player->active)
			{
				stopMusic = false;
				break;
			}
		}
		if (stopMusic)
		{
			if (data->windowFocus)
				data->front->musicEvent(MusicExitFever);
			currentgame->currentVolumeNormal = 0;
		}
		activeField = &m_fieldNormal;
		activeGarbage = &normalGarbage;

		// Add fevergarbage to normalgarbage, move accumulator
		// And switch any targets to fevergarbage to normalgarbage
		normalGarbage.CQ += feverGarbage.CQ; feverGarbage.CQ = 0;
		normalGarbage.GQ += feverGarbage.GQ; feverGarbage.GQ = 0;
		for (auto i : feverGarbage.accumulator)
		{
			// Move accumulators to normalgarbage
			normalGarbage.accumulator.push_back(i);
		}
		feverGarbage.accumulator.clear();
		for (auto& player : currentgame->players)
		{
			// Set opponents targets to normalgarbage
			if (player != this)
			{
				player->targetGarbage[this] = &normalGarbage;
			}
		}
		updateTray(&normalGarbage);
		updateTray(&feverGarbage);

		// Reset seconds and gauge
		feverGauge.seconds = 60 * 15;
		feverGauge.setCount(currentgame->currentruleset->initialFeverCount);

		// Reset garbage tray
		normalTray.align(activeField->getProperties().offsetX, activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
		normalTray.setDarken(false);
	}

	// Rotation animation
	if (m_transitionTimer > 0 && m_transitionTimer <= 50)
	{
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		// Rotate out fever field
		m_transformScale = 1.f - static_cast<float>(m_transitionTimer) / 50.f;
		activeField->setTransformScale(1.f - static_cast<float>(m_transitionTimer) / 50.f);

	}
	else if (m_transitionTimer > 50 && m_transitionTimer <= 100)
	{
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		// Rotate in fever field
		m_transformScale = static_cast<float>(m_transitionTimer - 50) / 50.f;
		activeField->setTransformScale(static_cast<float>(m_transitionTimer - 50) / 50.f);
	}

}

void Player::playFever()
{
	if (!feverMode) return;

	// Play sounds
	if (feverGauge.seconds / 60 > 0 && feverGauge.seconds / 60 < 6 && feverGauge.seconds % 60 == 0 && losewin == NOWIN)
	{
		data->snd.fevertimecount.play(data);
	}
	if (feverGauge.seconds == 1 && losewin == NOWIN)
	{
		data->snd.fevertimeend.play(data);
	}

	// Subtract seconds
	if (feverGauge.seconds > 0 && feverGauge.endless == false && losewin == NOWIN && !currentgame->stopChaining)
	{
		feverGauge.seconds--;
	}

	// Display
	feverGauge.update();
}

void Player::dropFeverChain()
{
	// Cycle background color
	const int old = feverColor;
	while (old == feverColor)
	{
		feverColor = ppvs::getRandom(5);
	}

	// Drop field
	activeField->dropField(getFeverChain(getRandom(currentgame->currentruleset->NFeverChains, m_randomizerFeverChain), colors, currentFeverChainAmount, getRandom(colors, m_randomizerFeverColor)));
	calledRandomFeverChain++;

	// Garbage should not drop after this
	forgiveGarbage = true;

	endPhase();
}

void Player::showSecondsObj(int n)
{
	const PosVectorFloat pv = feverGauge.getPositionSeconds();
	m_secondsObj.push_back(new SecondsObject(data));
	m_secondsObj.back()->setScale(m_globalScale);
	m_secondsObj.back()->showAt(m_nextPuyoOffsetX + (pv.x - 16) * m_nextPuyoScale * m_globalScale, m_nextPuyoOffsetY + (pv.y + 16) * m_nextPuyoScale * m_globalScale * m_secondsObj.size(), n);
}

void Player::setMarginTimer()
{
	margintimer++;
}

void Player::prepareVoice(int chain, int predictedChain)
{
	playvoice = getVoicePattern(chain, predictedChain, currentgame->currentruleset->voicePatternFever);

	// Count diacutes
	if (playvoice == 4)
		diacute++;

	stutterTimer = 0;
}

void Player::playVoice()
{
	if (playvoice < 0) return;

	stutterTimer++;
	if (stutterTimer == 1)
	{
		characterVoices.chain[playvoice].stop(data);
		characterVoices.chain[playvoice].play(data);
	}
	if (diacute > 0 && stutterTimer == 1 && playvoice > 4)
	{
		stutterTimer = -10;
		diacute--;
	}
	if (diacute == 0)
	{
		playvoice = -1;
	}
}

#pragma region Online stuff

void Player::bindPlayer(const std::string& name, unsigned int id, bool setActive)
{
	onlineName = name;
	active = setActive;
	onlineID = id;
	// Reset stuff
	messages.clear();
	wins = 0;
	loseConfirm = false;
}

void Player::unbindPlayer()
{
	onlineName = "";
	onlineID = 0;
	active = false;
	prepareActive = false;
	rematch = false;
	rematchIcon.setVisible(false);
	messages.clear();

	// Still playing
	if (losewin == NOWIN && currentphase != IDLE)
	{
		setLose();

		// Add to replay
		if (currentgame->settings->recording == PVS_RECORDING)
		{
			MessageEvent me = { data->matchTimer,"exit" };
			recordMessages.push_back(me);
		}

	}
	setStatusText(currentgame->translatableStrings.disconnected.c_str());

	// Was player in choose color status?
	if (!pickedColor)
	{
		bool start = true;
		for (auto& player : currentgame->players)
		{
			if (!player->active && !currentgame->settings->useCPUplayers)
				continue;
			if (!player->pickedColor || player->colorMenuTimer < -2)
			{
				start = false;
				break;
			}
		}
		if (start)
		{
			// Players should be in pickcolor phase

			colorMenuTimer = 0;
			for (const auto& player : currentgame->players)
			{
				if (!player->active && !currentgame->settings->useCPUplayers)
					continue;

				if (player->currentphase == PICKCOLORS)
					player->currentphase = GETREADY;
				else
					return;
			}
			if (currentgame->getActivePlayers() > 0)
			{
				currentgame->readyGoObj.prepareAnimation("readygo");
				currentgame->data->matchTimer = 0;
			}
		}

	}
}

void Player::processMessage()
{
	// Skip move message if it's a big multiplayer match
	if (currentgame->players.size() > 10 && !messages.empty() && messages.front()[0] == 'm')
	{
		messages.pop_front();
	}

	// Receive offset message
	if (m_type == ONLINE && !messages.empty() && messages.front().compare("fo") == 0)
	{
		currentgame->currentruleset->onOffset(this);
		messages.pop_front();
	}
	
	// Replay: player exits
	if (currentgame->settings->recording == PVS_REPLAYING && m_type == ONLINE &&
		!messages.empty() && messages.front().compare("exit") == 0)
	{
		setLose();
	}
}

void Player::addMessage(std::string mes)
{
	// Add to replay
	if (currentgame->settings->recording == PVS_RECORDING)
	{
		MessageEvent me = { data->matchTimer,"" };
		recordMessages.push_back(me);
		if (mes.length() < 64)
		{
			strcpy(recordMessages.back().message, mes.c_str());
		}
	}

	// Process immediately?
	if (m_type == ONLINE && mes == "d")
	{
		waitForConfirm--;
		return;
	}

	// Check if lose confirm was received
	if (m_type == ONLINE && mes == "o")
	{
		// This player confirms your loss
		loseConfirm = true;
		return;
	}

	// Add message
	messages.push_back(mes);
}

void Player::confirmGarbage()
{
	// Moves < Min(4, Turns+1) -> Moves should be less than 4, except the first turns
	
	// Read everyone's confirm count, if there's anyone with confirm>4, you can't move on
	// (alternatively, we could count the total)
	for (const auto& player : currentgame->players)
	{
		if (player != this && player->waitForConfirm > 3)
		{
			// Also see garbagephase
			currentgame->stopChaining = true;
			return;
		}
	}

	currentgame->stopChaining = false;
	endPhase();
}

void Player::waitGarbage()
{
	// ONLINE player gets stuck here until message about garbage arrives
	bool receive = false;

	// Receive n: dropped nothing
	if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'n')
	{
		receive = true;
		messages.pop_front();
	}

	if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'g')
	{
		// Drop garbage
		int dropAmount = 0;
		sscanf(messages.front().c_str(), "g|%i", &dropAmount);
		activeField->dropGarbage(false, dropAmount);
		receive = true;
		messages.pop_front();
	}

	if (receive == true)
	{
		// Send message dO back
		if (currentgame->connected && m_type == ONLINE)
		{
			currentgame->network->sendToChannel(CHANNEL_GAME, "d", currentgame->channelName.c_str());
		}
		endPhase();
	}
}

// Ignore all messages except the lose message
void Player::waitLose()
{
	if (messages.empty()) return;

	if (messages.front()[0] == 'l')
	{
		// Lose
		setLose();

		// Send confirmation back to this particular player
		if (currentgame->connected)
		{
			currentgame->network->sendToPeer(CHANNEL_GAME, "o", currentgame->channelName, onlineID);
		}
	}

	// Discard any message
	messages.pop_front();
}

void Player::getUpdate(std::string str)
{
	char fieldstring[500];
	char feverstring[500];

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
	int nGQ = 0; int fGQ = 0;
	sscanf(str.c_str(), "spectate|%i|%s |%i|%s |%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|"
		, &ph
		, fieldstring
		, &fm
		, feverstring
		, &fc
		, &rngseed
		, &rngcalled
		, &trns
		, &clrs
		, &mrgntmr
		, &chn
		, &crntfvrchn
		, &nGQ
		, &fGQ
		, &prdctchn
		, &allclr
	);

	// Initialize
	setRandomSeed(rngseed, &m_randomizerNextList);
	colors = clrs;
	initValues(rngseed + onlineID);

	// Set other stuff
	currentphase = Phase(ph);
	m_fieldNormal.setFieldFromString(fieldstring);
	feverMode = fm == 0 ? false : true;
	m_fieldFever.setFieldFromString(feverstring);
	feverGauge.setCount(fc);
	normalGarbage.GQ = nGQ;
	feverGarbage.GQ = fGQ;
	updateTray();
	calledRandomFeverChain = rngcalled;
	for (int i = 0; i < rngcalled; i++)
	{
		getRandom(0, m_randomizerFeverChain);
		getRandom(0, m_randomizerFeverColor);
	}
	turns = trns;

	// Call nextlist random
	for (int i = 0; i < turns; i++)
	{
		getRandom(0, m_randomizerNextList);
		getRandom(0, m_randomizerNextList);
	}
	margintimer = mrgntmr;
	chain = chn;
	predictedChain = prdctchn;
	allClear = allclr;

	// Set target garbage, otherwise it may crash
	for (auto& player : currentgame->players)
	{
		if (player != this)
		{
			targetGarbage[player] = player->activeGarbage;
		}
	}

	currentFeverChainAmount = crntfvrchn;

	if (feverMode)
	{
		activeField = &m_fieldFever;
		activeGarbage = &feverGarbage;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		chainPopSpeed = 11;
		garbageSpeed = 8.0;
		garbageEndTime = 80;
		puyoBounceEnd = 40;
		puyoBounceSpeed = 2;
		normalTray.align(activeField->getProperties().offsetX - 16 * m_globalScale, activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		normalTray.setDarken(true);
	}
}

void Player::prepareDisconnect()
{
	if (currentgame->settings->rankedMatch && m_type == HUMAN && currentgame->connected)
	{
		// Max wins reached
		bool disconnect = false;
		for (auto& player : currentgame->players)
		{
			if (player->wins == currentgame->settings->maxWins)
			{
				// Prepare to disconnect channel
				disconnect = true;
			}
		}
		if (disconnect)
		{
			currentgame->rankedState = 1;
			currentgame->network->requestChannelDescription(currentgame->channelName, std::string(""));
		}

	}
}

#pragma endregion

#pragma region Rendering

// Draw everything from the player
void Player::draw()
{
	// Field coordinates
	// -----------------
	data->front->pushMatrix();
	data->front->translate(properties.offsetX, properties.offsetY, 0);
	data->front->scale(properties.scaleX * getGlobalScale(), properties.scaleY * getGlobalScale(), 1);

	data->front->clearDepth();
	data->front->setDepthFunction(LessOrEqual);
	data->front->setBlendMode(AlphaBlending);
	data->front->setColor(0, 0, 0, 0);
	data->front->drawRect(nullptr, 0, 0, 192, 336);

	// Fever flip coordinates
	// ----------------------
	data->front->pushMatrix();
	data->front->translate(192.f / 2.f, 0, 0);
	data->front->scale(m_transformScale, 1, 1);
	data->front->translate(-192.f / 2.f, 0, 0);
	activeField->draw();
	data->front->setDepthFunction(Equal);
	movePuyos.draw();
	data->front->setDepthFunction(Always);
	data->front->popMatrix();
	// ----------------------
	m_borderSprite.draw(data->front);
	pchainWord->draw(data->front);
	characterAnimation.draw();
	// -----------------
	data->front->popMatrix();

	// Next coordinates
	// ----------------
	data->front->pushMatrix();
	data->front->translate(m_nextPuyoOffsetX, m_nextPuyoOffsetY, 0);
	data->front->scale(m_nextPuyoScale, m_nextPuyoScale, 1);
	m_nextPuyo.draw();
	feverGauge.draw();
	// ----------------
	data->front->popMatrix();

	normalTray.draw();
	feverTray.draw();
	m_scoreCounter.draw();

	// Field coordinates
	// -----------------
	data->front->pushMatrix();
	data->front->translate(properties.offsetX, properties.offsetY, 0);
	data->front->scale(properties.scaleX * getGlobalScale(), properties.scaleY * getGlobalScale(), 1);
	drawColorMenu();
	drawWin();
	drawLose();

	// Online; darken screen if player is not bound
	if (!currentgame->settings->useCPUplayers && (onlineName.empty() || !active))
	{
		overlaySprite.draw(data->front);
	}

	// Draw status text
	if (statusText)
	{
		data->front->setColor(255, 255, 255, 255);
		if ((m_type == ONLINE || m_type == HUMAN) && currentgame->currentGameStatus != GAMESTATUS_PLAYING
			&& currentgame->currentGameStatus != GAMESTATUS_SPECTATING && !currentgame->settings->useCPUplayers)
		{
			statusText->draw(0, 0);
		}
		else if (currentgame->forceStatusText)
		{
			statusText->draw(0, 0);
		}
	}
	// -----------------
	data->front->popMatrix();

	if (showCharacterTimer > 0 && currentgame->currentGameStatus != GAMESTATUS_PLAYING)
	{
		charHolderSprite.draw(data->front);
		currentCharacterSprite.draw(data->front);
		for (auto& sprite : dropset)
			sprite.draw(data->front);
	}

	if (currentgame->currentGameStatus != GAMESTATUS_PLAYING && currentgame->currentGameStatus != GAMESTATUS_SPECTATING)
	{
		rematchIcon.draw(data->front);
	}
}

// Draw light effect, secondsobj
void Player::drawEffect()
{
	for (const auto& lightEffect : m_lightEffect)
	{
		lightEffect->draw(data->front);
	}

	m_feverLight.draw(data->front);

	for (const auto& secondsObj : m_secondsObj)
	{
		secondsObj->draw(data->front);
	}

}

void Player::drawFieldBack(PosVectorFloat /*position*/, const float rotation)
{
	m_fieldSprite.setPosition(0, 0);
	m_fieldSprite.setCenter(0, 0);
	m_fieldSprite.setScale(1, 1);
	m_fieldSprite.setRotation(rotation);
	m_fieldSprite.draw(data->front);
}

// C99 round
double _round(double r) { return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5); }

void Player::drawFieldFeverBack(PosVectorFloat /*position*/, float rotation)
{
	if (!feverMode)
		return;

	m_fieldFeverSprite.setPosition(0, 0);
	m_fieldFeverSprite.setCenter(0, 0);
	m_fieldFeverSprite.setScale(1, 1);
	m_fieldFeverSprite.setRotation(rotation);

	// Transition to new color
	_round(feverColorR * 100) < _round(tunnelShaderColor[feverColor][0] * 100) ? feverColorR += 0.01f : feverColorR -= 0.01f;
	_round(feverColorG * 100) < _round(tunnelShaderColor[feverColor][1] * 100) ? feverColorG += 0.01f : feverColorG -= 0.01f;
	_round(feverColorB * 100) < _round(tunnelShaderColor[feverColor][2] * 100) ? feverColorB += 0.01f : feverColorB -= 0.01f;

	if (data->tunnelShader)
	{
		data->tunnelShader->setParameter("cl", feverColorR, feverColorG, feverColorB, 1.0f);
		m_fieldFeverSprite.setImage(data->imgFieldFever);
		m_fieldFeverSprite.draw(data->front, data->tunnelShader);
	}
	else
	{
		m_fieldFeverSprite.setImage(data->imgFeverBack[data->globalTimer / 2 % 30]);
		m_fieldFeverSprite.setColor(feverColorR * 255, feverColorG * 255, feverColorB * 255);
		m_fieldFeverSprite.draw(data->front);
	}
}

void Player::drawAllClear(PosVectorFloat pos, float scaleX, float scaleY, float rotation)
{
	FieldProp p = activeField->getProperties();
	const float dist = activeField->getFieldSize().y * 0.67f;
	const float x = pos.x - dist * sin(rotation * PI / 180.f);
	const float y = pos.y - dist * cos(rotation * PI / 180.f);

	// Tsu type
	if (allClear == 1)
	{
		// Just draw it until it clears
		m_allclearSprite.setVisible(true);
		m_allclearSprite.setPosition(x, y);
		m_allclearSprite.setScale(1, 1);
		m_allclearSprite.setRotation(rotation);
		m_allclearSprite.draw(data->front);
	}

	// Fever type: use timer
	if (allclearTimer >= 1)
	{
		allclearTimer++;
		m_allclearSprite.setPosition(x, y);
		m_allclearSprite.setScale(1, 1);
		m_allclearSprite.setRotation(rotation);
		if (allclearTimer >= 60)
		{
			// Flicker
			m_allclearSprite.setScale(scaleX + scaleX * static_cast<float>(allclearTimer - 60) / 10.f, scaleY - scaleY * static_cast<float>(allclearTimer - 60) / 10.f);
			m_allclearSprite.setTransparency(1.f - static_cast<float>(allclearTimer - 60) / 10.f);
		}
		else if (allclearTimer > 1 && allclearTimer < 60)
		{
			// Show normally
			m_allclearSprite.setVisible(true);
		}

		m_allclearSprite.draw(data->front);
		
		if (allclearTimer >= 70)
		{
			allclearTimer = 0;
		}
	}
}

void Player::drawCross(FeRenderTarget* r)
{
	// Place at spawnpoint
	PosVectorInt spv = movePuyos.getSpawnPoint();
	spv.y += 1;
	PosVectorFloat rpv = activeField->getLocalCoord(spv.x, spv.y);

	m_crossSprite.setPosition(rpv.x, rpv.y);
	m_crossSprite.draw(r);

	// Check if 2nd spawnpoint
	if (currentgame->currentruleset->doubleSpawn)
	{
		rpv = activeField->getLocalCoord(spv.x + 1, spv.y);
		m_crossSprite.setPosition(rpv.x, rpv.y);
		m_crossSprite.draw(r);
	}
}

void Player::drawLose()
{
	FieldProp p = activeField->getProperties();
	PosVectorFloat position = activeField->getBottomCoord(true);
	position.y -= activeField->getFieldSize().y * 0.75f;

	if (losewin == LOSE && m_loseWinTimer > 60)
	{
		m_loseSprite.setPosition(position);
		m_loseSprite.setScale(1 + 0.1f * sin(static_cast<float>(m_loseWinTimer - 60) / 20.f));
		m_loseSprite.draw(data->front);
	}
}

void Player::drawWin()
{
	FieldProp p = activeField->getProperties();
	PosVectorFloat position = activeField->getBottomCoord(true);
	position.y -= activeField->getFieldSize().y * 0.75f;

	if (losewin == WIN)
	{
		m_winSprite.setPosition(position.x, position.y + 10.f * sin(static_cast<float>(m_loseWinTimer) / 20.f));
		m_winSprite.setScale(1.f);
		m_winSprite.draw(data->front);
	}
}

void Player::drawColorMenu()
{
	for (auto& i : colorMenuBorder)
	{
		i.draw(data->front);
	}

	for (auto& i : spice)
	{
		i.draw(data->front);
	}
}

void Player::setStatusText(const char* utf8)
{
	if (utf8 == lastText) return;
	if (!statusFont) return;
	delete statusText;

	statusText = statusFont->render(utf8);
	lastText = utf8;
}

void Player::setDropsetSprite(int x, int y, PuyoCharacter pc)
{
	// Get total width and center dropset
	float length = 0;
	float xx = 0;
	const float scale = m_globalScale * 0.75f;
	for (int j = 0; j < 16; j++)
	{
		MovePuyoType mpt = getFromDropPattern(pc, j);
		length += mpt == DOUBLET ? 10 : 18;
	}
	xx = -length / 2.f - 5.f;

	for (int j = 0; j < 16; j++)
	{
		const MovePuyoType mpt = getFromDropPattern(pc, j);
		dropset[j].setPosition(static_cast<float>(x) + xx * scale, static_cast<float>(y));
		dropset[j].setScale(scale);
		switch (mpt)
		{
		case DOUBLET:
			dropset[j].setSubRect(0, 0, 16, 24);
			dropset[j].setCenter(0, 24);
			xx += 10;
			break;
		case TRIPLET:
			dropset[j].setSubRect(16, 0, 24, 24);
			dropset[j].setCenter(0, 24);
			xx += 18;
			break;
		case TRIPLETR:
			dropset[j].setSubRect(40, 0, 24, 24);
			dropset[j].setCenter(0, 24);
			xx += 18;
			break;
		case QUADRUPLET:
			dropset[j].setSubRect(64, 0, 24, 24);
			dropset[j].setCenter(0, 24);
			xx += 18;
			break;
		case BIG:
			dropset[j].setSubRect(88, 0, 24, 24);
			dropset[j].setCenter(0, 24);
			xx += 18;
			break;
		}
	}
}

#pragma endregion

}
