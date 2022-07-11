#include <algorithm>
#include "player.h"
#include "game.h"
#include "../PVS_ENet/PVS_Client.h"
#include "puyorng.h"
using namespace std;

namespace ppvs
{

player::player(playerType type, int playernum, int totalPlayers, game* g)
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

	//Types of players: human, CPU, network etc
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

	//Default active field: normal field
	activeField = &m_fieldNormal;

	//Controller
	controls.init(m_playernum, m_type, currentgame->settings->recording);

	//Mover
	movePuyos.init(data);
	dropspeed = 2;

	//tray
	normalTray.init(data);
	feverTray.init(data);

	//Initialize player values
	colors = 4;
	if (playernum == 1) //player 1 abides default character
		m_character = currentgame->settings->defaultPuyoCharacter;
	else
		m_character = ARLE;
	cpuAI = nullptr;
	divider = 2;

	//give CPU an AI
	if (m_type == CPU)
	{
		cpuAI = new AI(this);
	}

	//TEMP: in online version, other players must inform eachother from their randomseedFever
	if (currentgame->settings->useCPUplayers)
	{
		initValues(ppvs::getRandom(1000));
		//all player are active
		active = true;
	}
	else
		initValues(currentgame->randomSeed_NextList + onlineID);

	//TEMP
	//Field size may be set up by ruleset. For now, use standard field size
	properties.gridHeight = 28;
	properties.gridWidth = 32;
	properties.gridX = 6;
	properties.gridY = 12 + 3; //always add 3 extra layers
	properties.scaleX = 1; //local scale: only for local effects (bouncing and stuff)
	properties.scaleY = 1;
	properties.angle = 0;

	//Initialize objects (place fields, movepuyo, nextpuyo etc)
	playerSetup(properties, m_playernum, totalPlayers);
	properties = m_fieldNormal.getProperties();

	//resize sprite to fieldsize
	m_fieldSprite.setSize(m_fieldNormal.getFieldSize()); //useless
	m_fieldFeverSprite.setImage(data->imgFieldFever);
	m_fieldFeverSprite.setSize(m_fieldNormal.getFieldSize());

	//set chainword
	pchainWord = new chainWord(data);
	pchainWord->setScale(m_globalScale);

	//Initialize movePuyos at least once
	movePuyos.prepare(DOUBLET, this, 0, 0);

	//Initialize garbagetray
	normalTray.align(properties.offsetX, properties.offsetY - (32) * m_globalScale, m_globalScale);
	feverTray.align(properties.offsetX, properties.offsetY - (32) * m_globalScale, m_globalScale);
	feverTray.setVisible(false);

	//Initialize scoreCounter
	m_scoreCounter.init(data, properties.offsetX, m_fieldNormal.getBottomCoord().y + PUYOY / 4 * m_globalScale, m_globalScale);

	//Load other sprites
	m_allclearSprite.setImage(data->imgAllClear);
	m_allclearSprite.setCenter();
	m_crossSprite.setImage(data->imgPuyo);
	m_crossSprite.setSubRect(7 * PUYOX, 12 * PUYOY, PUYOX, PUYOY);
	m_crossSprite.setCenter(0, 0);
	m_winSprite.setImage(data->imgWin);
	m_winSprite.setCenter();
	m_loseSprite.setImage(data->imgLose);
	m_loseSprite.setCenter();

	//set border
	m_borderSprite.setCenter();
	m_borderSprite.setPosition(properties.centerX,
		properties.centerY / 2);
	m_borderSprite.setScale(1);

	//set origin of field to bottom of image
	m_fieldSprite.setCenterBottom();
	m_fieldFeverSprite.setCenterBottom();

	//Initialize character animation object
	setCharacter(m_character);
	pickingCharacter = false;

	//Color menu objects
	for (int i = 0; i < 9; i++)
	{
		colorMenuBorder[i].setImage(data->imgPlayerBorder);
		colorMenuBorder[i].setVisible(false);
		colorMenuBorder[i].setSubRect((i % 3) * 24, (i / 3) * 24, 24, 24);
	}
	for (int i = 0; i < 5; i++)
	{
		spice[i].setImage(data->imgSpice);
		spice[i].setSubRect(0, i * 50, 138, 50);
		spice[i].setVisible(false);
		spice[i].setCenter();
	}
	spiceSelect = 2;

	//online
	//needs a black image to draw over field
	overlaySprite.setImage(nullptr);
	overlaySprite.setTransparency(0.5f);
	overlaySprite.setScale(2 * 192, 336);
	overlaySprite.setColor(0, 0, 0);
	overlaySprite.setPosition(-192 / 2, -336 / 4);

	charHolderSprite.setImage(data->imgCharHolder);
	charHolderSprite.setCenter();
	//charHolderSprite.setScale(m_globalScale);
	charHolderSprite.setPosition(properties.offsetX + properties.gridWidth * properties.gridX / 2 * m_globalScale,
		properties.offsetY + properties.gridHeight * properties.gridY / 2 * m_globalScale);
	charHolderSprite.setVisible(false);
	charHolderSprite.setScale(m_globalScale);
	currentCharacterSprite.setVisible(true);
	for (int i = 0; i < 16; i++)
		dropset[i].setVisible(true);
	showCharacterTimer = 0;

	rematchIcon.setImage(data->imgCheckmark);
	rematchIcon.setCenter();
	rematchIcon.setPosition(properties.offsetX + properties.gridWidth * properties.gridX / 2 * m_globalScale,
		properties.offsetY + properties.gridHeight * (properties.gridY / 2 + 3) * m_globalScale);
	rematchIcon.setVisible(false);
	rematchIcon.setScale(m_globalScale);
	rematchIconTimer = 1000;

	//set text
	//text messages on screen
	statusFont = data->front->loadFont("Arial", 14);
	statusText = nullptr;

	if (currentgame->settings->recording != PVS_REPLAYING)
		setStatusText(currentgame->translatableStrings.waitingForPlayer.c_str());
}

void player::reset()
{
	//release all controls
	controls.release();

	//set colors
	if (!currentgame->settings->pickColors && currentgame->settings->recording != PVS_REPLAYING
		&& currentgame->settings->rulesetInfo.colors >= 3 && currentgame->settings->rulesetInfo.colors <= 5)
		colors = currentgame->settings->rulesetInfo.colors;

	//reset fields
	m_fieldNormal.clearField();
	m_fieldFever.clearField();
	m_fieldSprite.setVisible(true);
	m_fieldFeverSprite.setVisible(false);
	m_fieldNormal.setTransformScale(1);
	m_fieldFever.setTransformScale(1);

	movePuyos.prepare(DOUBLET, this, 0, 0);
	movePuyos.setVisible(false);

	//reset fever mode
	activeField = &m_fieldNormal;
	activeGarbage = &normalGarbage;
	normalTray.align(activeField->getProperties().offsetX, activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
	normalTray.setDarken(false);
	normalTray.update(0);
	feverTray.update(0);

	//reset replay values
	if (currentgame->settings->recording == PVS_RECORDING)
		recordMessages.clear();
	activeAtStart = 0;

	//reset others
	//TEMP random value
	if (currentgame->settings->useCPUplayers)
		initValues(ppvs::getRandom(1000));
	else
		initValues(currentgame->randomSeed_NextList + onlineID);

	if (!active)
		return;

	//TEMP immediate reset
	if (currentgame->settings->pickColors)
		currentphase = PICKCOLORS;
	else
	{
		currentphase = GETREADY;
		currentgame->readyGoObj.prepareAnimation("readygo");
		currentgame->data->matchTimer = 0;
	}
}

void player::initValues(int randomseed)
{
	useDropPattern = true;

	//Initialize randomizer
	randomSeedFever = randomseed;
	setRandomSeed(randomSeedFever, &m_randomizerFeverChain);
	setRandomSeed(randomSeedFever + 1, &m_randomizerFeverColor);
	calledRandomFeverChain = 0;
	nextPuyoActive = true;

	// init nuisance drop pattern randomizer
	m_randomizerNuisanceDrop->init_genrand(randomseed);
	m_nuisanceList.clear();

	//Other values to initialize
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
	//targetPoint=70; //may change per ruleset
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

	//stop any animation
	characterAnimation.prepareAnimation("");

	//Initialize or override values according to ruleset
	currentgame->currentruleset->onInit(this);

	//add 4 initial 3-colored puyo
	initNextList();
	turns = 0;
	waitForConfirm = 0;
	loseConfirm = false;

	//set nextpuyo
	if (useDropPattern)
		m_nextPuyo.update(m_nextList, m_character, 0);
	else
		m_nextPuyo.update(m_nextList, ARLE, 0);

	//Idle
	currentphase = IDLE;

	updateTray();
}

void player::initNextList()
{
	setRandomSeed(currentgame->randomSeed_NextList, &m_randomizerNextList);
	m_nextList.clear();
	//nextList needs 3 pairs to start
	addNewColorPair(3);
	addNewColorPair(3);
	checkAllClearStart();
	addNewColorPair(colors);
	turns = 0;

}

void player::checkAllClearStart()
{
	if (currentgame->currentruleset->allClearStart)
		return;
	//player is not allowed to start with all clear
	if (m_nextList.empty())
		return;
	int firstcolor = m_nextList.front();
	//at this point there should be 4 colors in m_nextlist
	for (std::deque<int>::iterator it = m_nextList.begin(); it != m_nextList.end(); it++)
	{
		if (firstcolor != (*it))
			return; //not the same color
	}

	//cleared all checks: list is filled with same colors
	//pop last 2 and push back new one
	m_nextList.pop_back();
	m_nextList.pop_back();
	addNewColorPair(3);
	//try again
	checkAllClearStart();
}

fsound* player::loadVoice(const std::string& folder, const char* sound)
{
	return data->front->loadSound((folder + sound).c_str());
}

void player::initVoices()
{
	std::string currentCharacter = currentgame->settings->characterSetup[getCharacter()];
	std::string folder = folder_user_character + currentCharacter + std::string("/Voice/");
	characterVoices.chain[0].SetBuffer(loadVoice(folder, "chain1.wav"));
	characterVoices.chain[1].SetBuffer(loadVoice(folder, "chain2.wav"));
	characterVoices.chain[2].SetBuffer(loadVoice(folder, "chain3.wav"));
	characterVoices.chain[3].SetBuffer(loadVoice(folder, "chain4.wav"));
	characterVoices.chain[4].SetBuffer(loadVoice(folder, "chain5.wav"));
	characterVoices.chain[5].SetBuffer(loadVoice(folder, "spell1.wav"));
	characterVoices.chain[6].SetBuffer(loadVoice(folder, "spell2.wav"));
	characterVoices.chain[7].SetBuffer(loadVoice(folder, "spell3.wav"));
	characterVoices.chain[8].SetBuffer(loadVoice(folder, "spell4.wav"));
	characterVoices.chain[9].SetBuffer(loadVoice(folder, "spell5.wav"));
	characterVoices.chain[10].SetBuffer(loadVoice(folder, "counter.wav"));
	characterVoices.chain[11].SetBuffer(loadVoice(folder, "counterspell.wav"));
	characterVoices.damage1.SetBuffer(loadVoice(folder, "damage1.wav"));
	characterVoices.damage2.SetBuffer(loadVoice(folder, "damage2.wav"));
	characterVoices.choose.SetBuffer(loadVoice(folder, "choose.wav"));
	characterVoices.fever.SetBuffer(loadVoice(folder, "fever.wav"));
	characterVoices.feversuccess.SetBuffer(loadVoice(folder, "feversuccess.wav"));
	characterVoices.feverfail.SetBuffer(loadVoice(folder, "feverfail.wav"));
	characterVoices.lose.SetBuffer(loadVoice(folder, "lose.wav"));
	characterVoices.win.SetBuffer(loadVoice(folder, "win.wav"));
}

void player::playerSetup(fieldProp& properties, int playernum, int playerTotal)
{
	//right side square setup
	int width = ceil(sqrt(double(playerTotal - 1)));

	//Player 1
	if (playernum == 1)
	{
		m_globalScale = 1;
		properties.offsetX = 48;
		properties.offsetY = 84;
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);
		//Set nextpuyo
		m_nextPuyoOffsetX = properties.offsetX + (properties.gridX * properties.gridWidth * properties.scaleX + 10) * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, true, data);
		//Set background
		m_fieldSprite.setImage(data->imgField1);//can also be character related
		//Set up border
		m_borderSprite.setImage(data->imgBorder1);
		//fever gauge
		feverGauge.init(-8, properties.gridHeight * (properties.gridY - 3 - 0), 1, true, data);
	}
	//other players
	else if (playernum > 1)
	{
		m_globalScale = 1.0f / float(width);
		properties.offsetX = 400 + ((playernum - 2) % width) * 320 * m_globalScale - 75 * m_globalScale * (width - 1);
		properties.offsetY = 84 + ((playernum - 2) / width) * 438 * m_globalScale - 42 * m_globalScale * (width - 1);
		m_fieldNormal.init(properties, this);
		m_fieldFever.init(properties, this);
		m_fieldTemp.init(properties, this);
		//Set nextpuyo
		m_nextPuyoOffsetX = properties.offsetX - 75 * m_globalScale;
		m_nextPuyoOffsetY = properties.offsetY;
		m_nextPuyoScale = m_globalScale;
		m_nextPuyo.init(0, 0, 1, false, data);
		//Set background
		m_fieldSprite.setImage(data->imgField2);//can also be character related
		//Set up Border
		m_borderSprite.setImage(data->imgBorder2);
		//feverGauge
		feverGauge.init(76, properties.gridHeight * (properties.gridY - 3 - 0), 1, false, data);
	}
	return;

}

player::~player()
{
	delete pchainWord;
	delete cpuAI;
	//delete light effects
	while (!m_lightEffect.empty())
	{
		delete m_lightEffect.back();
		m_lightEffect.pop_back();
	}
	//delete other
	while (!m_secondsObj.empty())
	{
		delete m_secondsObj.back();
		m_secondsObj.pop_back();
	}

	if (statusText)
		delete statusText;
	delete statusFont;
	delete m_randomizerNuisanceDrop;
}

void player::setPlayerType(playerType playertype)
{
	m_type = playertype;
	controls.init(m_playernum, m_type, currentgame->settings->recording);
}

void player::setRandomSeed(unsigned long seed_in, PuyoRNG** randomizer)
{
	// check if already exists
	if (*randomizer != nullptr)
	{
		delete (*randomizer);
	}

	if (currentgame->legacyRandomizer == true) {
		(*randomizer) = MakePuyoRNG("legacy", seed_in, 0);
	}
	else {
		(*randomizer) = MakePuyoRNG("classic", seed_in, 0);
	}
}

int player::getRandom(int in, PuyoRNG* randomizer)
{//randomizer for next puyo list
	Piece piece = randomizer->next(in);
	return int(piece);
}

void player::setCharacter(puyoCharacter pc, bool show)
{//set character and initialize all related values (voice,animation,background)
	m_character = pc;
	initVoices();
	posVectorFloat offset(activeField->getProperties().centerX * 1,
		activeField->getProperties().centerY / 2.0f * 1);
	std::string currentCharacter = currentgame->settings->characterSetup[pc];
	if (currentgame->players.size() <= 10)
		characterAnimation.init(data, offset, 1, folder_user_character + currentCharacter + std::string("/Animation/"));
	else
		characterAnimation.init(data, offset, 1, "");
	if (currentgame->settings->useCharacterField)
		setFieldImage(pc);

	if (show)
	{
		currentCharacterSprite.setImage(data->front->loadImage((folder_user_character + currentgame->settings->characterSetup[pc] + "/icon.png").c_str()));
		currentCharacterSprite.setCenter();
		currentCharacterSprite.setPosition(charHolderSprite.getPosition() + posVectorFloat(1, 1) - posVectorFloat(2, 4));//correct for shadow
		currentCharacterSprite.setVisible(true);
		currentCharacterSprite.setScale(m_globalScale);
		charHolderSprite.setVisible(true);
		showCharacterTimer = 5 * 60;
		for (int i = 0; i < 16; i++)
			dropset[i].setImage(data->front->loadImage("Data/CharSelect/dropset.png"));
		setDropsetSprite(currentCharacterSprite.getX(), currentCharacterSprite.getY() + 60 * m_globalScale, m_character);
	}
}

void player::setFieldImage(puyoCharacter pc)
{
	//in case of failure
	fimage* im = data->imgCharField[int(pc)];
	if (!im || im->error())
		if (m_playernum == 1)
			m_fieldSprite.setImage(data->imgField1);
		else
			m_fieldSprite.setImage(data->imgField2);
	else
		m_fieldSprite.setImage(im);
	if (m_nextPuyo.getOrientation() < 0)
		m_fieldSprite.setFlipX(true);
}


void player::play()
{//Game code
	//Debugging
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

	//pick character (online)
	if (showCharacterTimer > 0)
	{
		showCharacterTimer--;
		charHolderSprite.setTransparency(1);
		currentCharacterSprite.setTransparency(1);
		for (int i = 0; i < 16; i++)
			dropset[i].setTransparency(1);
		if (showCharacterTimer < 120)
		{
			charHolderSprite.setTransparency(showCharacterTimer / 120.0f);
			currentCharacterSprite.setTransparency(showCharacterTimer / 120.0f);
			for (int i = 0; i < 16; i++)
				dropset[i].setTransparency(showCharacterTimer / 120.0f);
		}
	}
	//play animation for checkmark
	if (rematchIconTimer < 1000)
	{
		rematchIconTimer++;
		rematchIcon.setScale(m_globalScale * interpolate("elastic", 2, 1, rematchIconTimer / 60.0, -5, 2));
	}
	rematchIcon.setVisible(rematch);

	//process message
	if (m_type == ONLINE)
		processMessage();

	//=====Global events
	//Animate particles
	activeField->animateParticle();
	//Animate chainword
	pchainWord->move();
	//Animate light effects & secondsobj
	playLightEffect();
	//Do characterAnimation
	characterAnimation.playAnimation();
	//Move nextPuyo
	if (currentphase > 0 && nextPuyoActive)
		m_nextPuyo.play();
	//Animate garbage trays
	normalTray.play();
	feverTray.play();
	//set scorecounter
	setScoreCounter();
	//Fever mode specific
	playFever();

	//set Margin Time
	if (currentphase != PICKCOLORS && currentphase != IDLE)
		setMarginTimer();

	//=====PHASE -2: PICK COLORS
	if (currentphase == PICKCOLORS)
		chooseColor();

	//=====PHASE -1: IDLE
	//possiblity to enter menu? (maybe outside the player object)

	//=====PHASE 0: GETREADY
	if (currentphase == GETREADY)
		getReady();

	//=====PHASE 1: PREPARE
	if (currentphase == PREPARE)
		prepare();
	//=====Phase 10: MOVE (global), the movement phase
	movePuyos.move();
	if (currentphase == MOVE)
	{
		nextPuyoActive = true;//start moving nextpuyo if it doesnt yet
		hasMoved = true;
	}

	if (currentphase == MOVE && cpuAI)
	{//perform AI movement
		cpuMove();
	}

	//=====Phase 20 :CREATEPUYO
	if (currentphase == CREATEPUYO)
	{//CREATEPUYO phase is dropping a colored puyo pair
		activeField->createPuyo();
		//reset chain number
		chain = 0; predictedChain = 0;
		//set target point
		if (currentgame->currentruleset->marginTime >= 0)
			targetPoint = getTargetFromMargin(currentgame->currentruleset->targetPoint, currentgame->currentruleset->marginTime, margintimer);
	}
	//==========Phase 21: DROPPUYO
	if (currentphase == DROPPUYO)
		activeField->dropPuyo();
	//==========Phase 22: FALLPUYO (global)
	activeField->fallPuyo();
	activeField->bouncePuyo();
	if (currentphase == FALLPUYO || currentphase == FALLGARBAGE)
	{
		//end phase
		activeField->endFallPuyoPhase();
	}
	//==========Phase 31: SEARCHCHAIN
	if (currentphase == SEARCHCHAIN)
		activeField->searchChain();
	//==========Phase 32: DESTROYPUYO
	if (currentphase == DESTROYPUYO)
		destroyPuyos();
	//global phase 32
	activeField->popPuyoAnim();
	//==========Phase33-35: GARBAGE
	if (currentphase == GARBAGE)
		garbagePhase();
	//global: countergarbage, it's outisde garbage phase
	if (attdef == COUNTERATTACK)
		counterGarbage();

	//==========Phase 40: CHECKALLCLEAR
	if (currentphase == CHECKALLCLEAR)
		checkAllClear();
	//==========Phase 41: DROPGARBAGE
	if (currentphase == DROPGARBAGE)
		activeField->dropGarbage();
	//==========Phase 43: CHECKLOSER
	if (currentphase == CHECKLOSER)
		checkLoser(true);

	//==========Phase 45: WAITGARBAGE - it acts the same as dropGarbage, except it waits for a message
	if (currentphase == WAITGARBAGE)
		waitGarbage();
	//==========Phase 46: WAITCONFIRMGARBAGE
	if (currentphase == WAITCONFIRMGARBAGE)
		confirmGarbage();

	//==========Phase 50: CHECKFEVER
	if (currentphase == CHECKFEVER)
		checkFever();
	//==========Phase 51: PREPAREFEVER
	if (currentphase == PREPAREFEVER)
		startFever();
	//==========Phase 52: DROPFEVERCHAIN
	if (currentphase == DROPFEVER)
		dropFeverChain();
	//==========Phase 53: CHECKFEVER
	if (currentphase == CHECKENDFEVER)
		checkEndFever();
	//==========Phase 54: ENDFEVER
	if (currentphase == ENDFEVER)
		endFever();
	//==========Phase 54: CHECKENDFEVERONLINE
	if (currentphase == CHECKENDFEVERONLINE)
		checkEndFeverOnline();


	//==========Phase 60: LOST
	if (currentphase == LOSEDROP)
		loseGame();

	if (currentphase == WAITLOSE)
		waitLose();

	if (losewin == WIN)
	{
		currentphase = IDLE;
		winGame();
	}

	//play voice (leave at end)
	playVoice();
}

void player::endPhase()
{
	currentphase = currentgame->currentruleset->endPhase(currentphase, this);
}

void player::updateTray(garbageCounter* c)
{//update nuisance tray
	if (!c)
	{//automatic check of active garbage
		if (activeGarbage == &normalGarbage)
			normalTray.update(normalGarbage.GQ + normalGarbage.CQ);
		else
		{
			feverTray.update(feverGarbage.GQ + feverGarbage.CQ);
			if (feverGarbage.GQ + feverGarbage.CQ <= 0)
				normalTray.update(normalGarbage.GQ + normalGarbage.CQ);
		}
	}
	else
	{//not active, but must still be updated
		if (c == &normalGarbage)
			normalTray.update(c->CQ + c->GQ);
		else if (c == &feverGarbage)
			feverTray.update(c->CQ + c->GQ);
	}
	//debugstring=to_string(normalGarbage.GQ+normalGarbage.CQ);
}

void player::playGarbageSound()
{
	if (chain == 3)
		data->snd.nuisance_hitS.Play(data);
	else if (chain == 4)
		data->snd.nuisance_hitM.Play(data);
	else if (chain == 5)
		data->snd.nuisance_hitL.Play(data);
	else if (chain >= 6)
		data->snd.heavy.Play(data);

}

void player::setScoreCounter()
{
	m_scoreCounter.setCounter(scoreVal);
}
void player::setScoreCounterPB()
{
	m_scoreCounter.setPointBonus(point, bonus);
}

void player::getReady()
{
	charHolderSprite.setVisible(false);
	currentCharacterSprite.setVisible(false);

	//wait until timer hits go
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
		//reset controls
		controls.release();
		endPhase();
		m_readyGoTimer = 0;
		return;
	}


	m_readyGoTimer++;
}

void player::chooseColor()
{
	currentgame->data->matchTimer = 0;

	//add timer
	if (colorMenuTimer != 0)
		colorMenuTimer++;
	else
		return;

	//set border
	if (colorMenuTimer == 2)
	{
		menuHeight = 0;
		for (int i = 0; i < 9; i++)
			colorMenuBorder[i].setVisible(true);
	}
	//disappear
	if (colorMenuTimer == -26)
	{
		for (int i = 0; i < 9; i++)
			colorMenuBorder[i].setVisible(false);
	}
	//fade in/out
	if (colorMenuTimer < 25 && colorMenuTimer != 0)
	{
		if (colorMenuTimer > 0)
			menuHeight = colorMenuTimer * 6;
		else if (colorMenuTimer < -25)
			menuHeight = 150 - (colorMenuTimer + 50) * 6;

		colorMenuBorder[0].setPosition((192 / 2) - (88 + 00), 336 + (-168 - menuHeight));
		colorMenuBorder[1].setPosition((192 / 2) - (88 - 24) - 1, 336 + (-168 - menuHeight));
		colorMenuBorder[2].setPosition((192 / 2) + (88 - 24), 336 + (-168 - menuHeight));

		colorMenuBorder[3].setPosition((192 / 2) - (88 + 00), 336 + (-168 - menuHeight + 24) - 4);
		colorMenuBorder[4].setPosition((192 / 2) - (88 - 24), 336 + (-168 - menuHeight + 24) - 4);
		colorMenuBorder[5].setPosition((192 / 2) + (88 - 24), 336 + (-168 - menuHeight + 24) - 4);

		colorMenuBorder[6].setPosition((192 / 2) - (88 + 00), 336 + (-168 + menuHeight));
		colorMenuBorder[7].setPosition((192 / 2) - (88 - 24) - 1, 336 + (-168 + menuHeight));
		colorMenuBorder[8].setPosition((192 / 2) + (88 - 24), 336 + (-168 + menuHeight));
		
		colorMenuBorder[1].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[4].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[7].setScaleX(-(colorMenuBorder[0].getX() - colorMenuBorder[8].getX() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[1].setScaleY(1);
		colorMenuBorder[4].setScaleY(1);
		colorMenuBorder[7].setScaleY(1);

		colorMenuBorder[3].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[4].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[5].setScaleY(-(colorMenuBorder[0].getY() - colorMenuBorder[6].getY() + 24 * 1 - 4) / 24.0);
		colorMenuBorder[3].setScaleX(1);
		colorMenuBorder[4].setScaleX(1);
		colorMenuBorder[5].setScaleX(1);

		colorMenuBorder[0].setScale(1);
		colorMenuBorder[2].setScale(1);
		colorMenuBorder[6].setScale(1);
		colorMenuBorder[8].setScale(1);

	}
	//initialize
	if (colorMenuTimer == 25)
	{
		for (int i = 0; i < 5; i++)
		{
			spice[i].setVisible(true);
			spice[i].setTransparency(1);
			spice[i].setPosition((192 / 2), 336 + (-168 - 125 + 37 + 50 * i));
		}
	}
	//make choice
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
	//player 1 takes over CPU choice
	takeover = false;
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{//find first CPU player
		if (currentgame->players[i]->takeover) //no other takeovers
			break;
		if (currentgame->players[0]->pickedColor &&
			currentgame->players[i]->getPlayerType() == CPU &&
			!currentgame->players[i]->pickedColor &&
			currentgame->players[i] == this)
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
	//select option
	if (colorMenuTimer > 0)
	{
		bool select = false;
		if (controls.Down == 1 && spiceSelect < 4)
		{
			spiceSelect++;
			data->snd.cursor.Play(data);
			select = true;
			//send message
			if (currentgame->connected)
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("s|") + to_string(spiceSelect), currentgame->channelName);
		}
		if (controls.Up == 1 && spiceSelect > 0)
		{
			spiceSelect--;
			data->snd.cursor.Play(data);
			select = true;
			//send message
			if (currentgame->connected && m_type == HUMAN)
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("s|") + to_string(spiceSelect), currentgame->channelName);
		}

		//get online message
		if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 's')
		{
			sscanf(messages.front().c_str(), "s|%i", &spiceSelect);
			select = true;
			//currentMessageStr="";
			messages.pop_front();
		}

		if (select || colorMenuTimer == 25)
		{//put this in
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

		//automatic choice
		if (currentgame->colorTimer == 1 && colorMenuTimer > 25 && pickedColor == false)
			controls.A = 1;

		//make choice
		if (controls.A == 1 && colorMenuTimer > 25)
		{
			normalGarbage.GQ += normalGarbage.CQ;
			normalGarbage.CQ = 0;
			data->snd.decide.Play(data);
			controls.A++;
			if (takeover)
				currentgame->players[0]->controls.A++;
			currentgame->colorTimer = 0;
			colorMenuTimer = -50;
			pickedColor = true;
			for (int i = 0; i < 5; i++)
				spice[i].setVisible(false);
			//re-init colors
			initNextList();
			//reset takeover if necessary
			takeover = false;
			//send message
			if (currentgame->connected && m_type == HUMAN)
				currentgame->network->sendToChannel(CHANNEL_GAME, std::string("c|") + to_string(spiceSelect), currentgame->channelName);
		}
		//get choice from online
		if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'c')
		{
			sscanf(messages.front().c_str(), "c|%i", &spiceSelect);
			//currentMessageStr="";
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
			data->snd.decide.Play(data);
			colorMenuTimer = -50;
			pickedColor = true;
			for (int i = 0; i < 5; i++)
				spice[i].setVisible(false);
			//re-init colors
			initNextList();
		}

	}
	//check if everyone picked colors
	if (colorMenuTimer == -2)
	{
		bool start = true;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (!currentgame->players[i]->active && !currentgame->settings->useCPUplayers)
				continue;
			if (!currentgame->players[i]->pickedColor || currentgame->players[i]->colorMenuTimer < -2)
			{
				start = false;
				break;
			}
		}
		if (start)
		{
			colorMenuTimer = 0;
			for (size_t i = 0; i < currentgame->players.size(); i++)
			{
				if (!currentgame->players[i]->active && !currentgame->settings->useCPUplayers)
					continue;
				currentgame->players[i]->currentphase = GETREADY;
			}
			currentgame->readyGoObj.prepareAnimation("readygo");
			currentgame->data->matchTimer = 0;
		}
	}
	//fade out with flicker
	if (colorMenuTimer < 0)
		spice[spiceSelect].setVisible(false);
	if (colorMenuTimer < -20)
	{
		spice[spiceSelect].setVisible(true);
		spice[spiceSelect].setTransparency(interpolate("linear", 1, 0, (colorMenuTimer + 50) / 30.0, 0, 0));
	}
	/*if (colorMenuTimer<20 && (-colorMenuTimer)%2==1)
		spice[spiceSelect].setVisible(true);
	if (colorMenuTimer<20 && (-colorMenuTimer)%2==0)
		spice[spiceSelect].setVisible(false);*/

}

void player::prepare()
{
	//Read colors and pop front after reading
	int color1 = *(m_nextList.begin());
	int color2 = *(m_nextList.begin() + 1);
	addNewColorPair(colors);
	popColor();

	movePuyoType mpt = DOUBLET;
	if (useDropPattern)
		mpt = getFromDropPattern(m_character, turns);
	movePuyos.prepare(mpt, this, color1, color2);

	if (cpuAI)
	{//determine best chain
		cpuAI->prepare(mpt, color1, color2);
		if (activeGarbage->GQ > 0)
			cpuAI->pinch = true;
		else
			cpuAI->pinch = false;
		cpuAI->findLargest();
	}
	//release controls for ONLINE player
	if (m_type == ONLINE)
		controls.release();

	//currentphase=MOVE;
	endPhase();
	poppedChain = false;
	createPuyo = false;
	forgiveGarbage = false;

}
void player::cpuMove()
{
	//get current rotation
	if (movePuyos.getType() != BIG)
	{
		if (movePuyos.getRotation() != cpuAI->bestRot)
			controls.A = cpuAI->timer % 12;
	}
	else
	{
		if (movePuyos.getColorBig() != cpuAI->bestRot)
			controls.A = cpuAI->timer % 12;
	}

	if (cpuAI->timer > 30)
		controls.Down++;
	cpuAI->timer++;

	//    debugstring=to_string(movePuyos.getPosX1())+"-"+to_string(cpuAI->bestPos)+"("+to_string(cpuAI->bestChain)+")\n";
	if (movePuyos.getPosX1() == cpuAI->bestPos)
	{
		controls.A = 0;
		controls.Left = 0;
		controls.Right = 0;
		return;
	}

	if (movePuyos.getPosX1() < cpuAI->bestPos && cpuAI->timer > 10)
		controls.Right++;
	if (movePuyos.getPosX1() > cpuAI->bestPos && cpuAI->timer > 10)
		controls.Left++;
	if (controls.Left > 0 && controls.Right > 0)
	{
		controls.Left = 0;
		controls.Right = 0;
	}

}

void player::destroyPuyos()
{//phase 31
	destroyPuyosTimer++;
	if (destroyPuyosTimer == chainPopSpeed)
	{
		//create "XX Chain" word
		fieldProp prop = activeField->getProperties();
		pchainWord->showAt(prop.gridWidth * rememberX * prop.scaleX,
			prop.gridHeight * (prop.gridY - 3 - (rememberMaxY + 3)) * prop.scaleY, chain);
		//play sound
		data->snd.chain[min(chain - 1, 6)].Play(data);
	}

	if (destroyPuyosTimer == chainPopSpeed)
	{
		//set EQ
		if (chain >= currentgame->currentruleset->requiredChain)
		{
			float power = 1;
			int div = 2;
			if (feverMode)
				power = currentgame->currentruleset->feverPower;
			if (currentgame->players.size() > 1)
				div = max(2, divider);

			EQ = int(currentScore / targetPoint) * power * 3 / (div + 1);
			//currentScore-=targetPoint*float(EQ/power);
			currentScore -= targetPoint * int(currentScore / targetPoint);

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

void player::playLightEffect()
{//light effect & secondsobj
	for (size_t i = 0; i < m_lightEffect.size(); i++)
	{
		m_lightEffect[i]->setTimer(garbageSpeed);
	}
	//check if any should be removed
	for (size_t i = 0; i < m_lightEffect.size(); i++)
	{
		if (m_lightEffect[i]->getTimer() > 300)
		{
			delete m_lightEffect[i];
			m_lightEffect.erase(std::remove(m_lightEffect.begin(), m_lightEffect.end(), m_lightEffect[i]), m_lightEffect.end());
		}
	}
	//play feverlight
	m_feverLight.setTimer(0.1);
	//update fevergauge
	feverGauge.update();

	//secondsobj
	for (size_t i = 0; i < m_secondsObj.size(); i++)
	{
		m_secondsObj[i]->move();
	}
	//check if any should be removed
	for (size_t i = 0; i < m_secondsObj.size(); i++)
	{
		if (m_secondsObj[i]->getTimer() > 50)
		{
			delete m_secondsObj[i];
			m_secondsObj.erase(std::remove(m_secondsObj.begin(), m_secondsObj.end(), m_secondsObj[i]), m_secondsObj.end());
		}
	}


}

void player::addFeverCount()
{
	if (feverGauge.getCount() == 7)
		return;

	int dir = m_nextPuyo.getOrientation();
	posVectorFloat startpv, middlepv, endpv;
	startpv = posVectorFloat(properties.offsetX + (192 / 2) * properties.scaleX * getGlobalScale(), properties.offsetY);
	endpv = posVectorFloat(m_nextPuyoOffsetX + (feverGauge.getPV().x) * m_nextPuyoScale, m_nextPuyoOffsetY + (feverGauge.getPV().y) * m_nextPuyoScale);
	middlepv.x = startpv.x + dir * PUYOX * 1.5;
	middlepv.y = startpv.y - PUYOY * 1.5;

	m_feverLight.init(startpv, middlepv, endpv);
	feverGauge.addCount();
}

int player::getGarbageSum()
{//total garbage including fever garbage
	return normalGarbage.CQ + normalGarbage.GQ + feverGarbage.CQ + feverGarbage.GQ;
}

int player::getAttackSum()
{//total attack amount
	int attack = EQ - max(getGarbageSum(), 0);  //----> do not count negative GQ
	//int attack=EQ-getGarbageSum();
	if (attack > 0)
		return attack;
	else
		return 0;
}

void player::addAttacker(garbageCounter* gc, player* pl)
{
	std::vector<player*>::iterator it;
	it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl);
	//found element
	if (it != gc->accumulator.end())
		return;
	gc->accumulator.push_back(pl);
}

void player::removeAttacker(garbageCounter* gc, player* pl)
{
	std::vector<player*>::iterator it;
	it = std::find(gc->accumulator.begin(), gc->accumulator.end(), pl);
	//found element
	if (it != gc->accumulator.end())
	{
		gc->accumulator.erase(it);
		lastAttacker = pl;
	}
}

void player::checkAnyAttacker(garbageCounter* gc)
{//check if any attackers left in accumulator
	//loop through attackers and check their state to be sure
	std::vector<player*> remove;
	for (size_t i = 0; i < gc->accumulator.size(); i++)
	{
		//player still alive?
		if (gc->accumulator[i]->losewin != NOWIN || !gc->accumulator[i]->active)
			remove.push_back(gc->accumulator[i]);
	}
	while (!remove.empty())
	{
		removeAttacker(gc, remove.back());
		remove.pop_back();
	}

	if (gc->accumulator.size() > 0)
		return;
	else
		//if none found, add CQ to GQ
		addToGQ(gc);

}

void player::checkFeverGarbage()
{//negative normal garbage during fever?!
	return;
	if (normalGarbage.GQ < 0 && feverGarbage.GQ>0 && feverGarbage.GQ == -normalGarbage.GQ)
	{
		normalGarbage.GQ = 0;
		feverGarbage.GQ = 0;
	}
	updateTray();
}

void player::addToGQ(garbageCounter* gc)
{
	if (gc->accumulator.size() == 0)
	{
		gc->GQ += gc->CQ;
		gc->CQ = 0;
	}
}

void player::resetNuisanceDropPattern()
{
	int fieldWidth = properties.gridX;
	m_nuisanceList.clear();
	for (int i = 0; i < fieldWidth; ++i) {
		m_nuisanceList.push_back(i);
	}

}

int player::nuisanceDropPattern()
{
	// refill if array is empty
	if (m_nuisanceList.size() == 0) {
		resetNuisanceDropPattern();
	}

	// pick an index
	double rand = m_randomizerNuisanceDrop->genrand_real1();
	int randomIndex = int(m_nuisanceList.size()) * rand;

	// swap that index with the end and pop
	int endValue = m_nuisanceList[m_nuisanceList.size() - 1];
	int value = m_nuisanceList[randomIndex];
	m_nuisanceList[randomIndex] = endValue;
	m_nuisanceList.pop_back();

	return value;
}

void player::startGarbage()
{
	garbageTimer = 0;

	//calculate DEF (CQ+GQ+sum(EQ))
	int totalEQ = 0;
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		//sum opponents attack
		if (currentgame->players[i] != this)
			totalEQ += currentgame->players[i]->getAttackSum();
	}
	int DEF = getGarbageSum() + totalEQ;

	//set targets on the first run of startgarbage phase
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		if (currentgame->players[i] != this && chain == 1)
		{
			//targetGarbage.push_back(currentgame->players[i]->activeGarbage);
			targetGarbage[currentgame->players[i]] = currentgame->players[i]->activeGarbage;
		}
	}

	//attack or defend?
	if (DEF <= 0 && EQ > 0)
	{
		attdef = ATTACK;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i] != this)
			{
				//add any negative GQ
				if (normalGarbage.GQ + feverGarbage.GQ < 0)
				{
					EQ += -1 * (normalGarbage.GQ + feverGarbage.GQ);
					normalGarbage.GQ = 0;
					feverGarbage.GQ = 0;
					updateTray();
				}

				//create light effect animation for each opponent
				posVectorFloat startpv, middlepv, endpv;
				startpv = activeField->getGlobalCoord(rememberX, rememberMaxY);
				endpv = currentgame->players[i]->activeField->getTopCoord(-1);
				//calculate a middle
				//int dir=sign(int(endpv.x-startpv.x));
				int dir = m_nextPuyo.getOrientation();
				middlepv.x = startpv.x - dir * PUYOX * 3;
				middlepv.y = startpv.y - PUYOY * 3;
				m_lightEffect.push_back(new lightEffect(data, startpv, middlepv, endpv));

				//add self to garbage accumulator
				currentgame->players[i]->addAttacker(targetGarbage[currentgame->players[i]], this);
			}
		}
	}
	else if (DEF > 0)
	{
		//calculate COUNTER = CQ+GQ+sum(EQ) - EQ
		int COUNTER = getGarbageSum() + totalEQ - EQ;
		//debugstring=to_string(getGarbageSum())+"+"+to_string(totalEQ)+"-"+to_string(EQ)+"="+to_string(COUNTER);
		if (COUNTER >= 0)
		{//normal defense
			attdef = DEFEND;
			//remove self from opponent garbage accumulator (if present)
			for (size_t i = 0; i < currentgame->players.size(); i++)
			{
				if (currentgame->players[i] != this)
				{
					currentgame->players[i]->removeAttacker(targetGarbage[currentgame->players[i]], this);
					currentgame->players[i]->checkAnyAttacker(targetGarbage[currentgame->players[i]]);
				}
			}
		}
		else
		{//countering defense
			//wait for defense to end before making next move
			attdef = COUNTERDEFEND;
			//counter attack voice
			if (playvoice > 4 && chain >= 3 && currentgame->currentruleset->voicePatternFever == true)
				playvoice = 11;
			if (playvoice <= 4 && chain >= 3 && currentgame->currentruleset->voicePatternFever == true)
				playvoice = 10;

		}
		//create light effect for defense
		posVectorFloat startpv, middlepv, endpv;
		startpv = activeField->getGlobalCoord(rememberX, rememberMaxY);
		endpv = activeField->getTopCoord(-1);
		//calculate a middle
		int dir = m_nextPuyo.getOrientation();
		middlepv.x = startpv.x - dir * PUYOX * 3;
		middlepv.y = startpv.y - PUYOY * 3;
		m_lightEffect.push_back(new lightEffect(data, startpv, middlepv, endpv));

	}
	else
	{
		attdef = NOATTACK;
	}
}

void player::garbagePhase()
{//phase 33
	//stop chaining
	if (currentgame->stopChaining)
		return;

	if (garbageTimer == 0)
	{
		hasMoved = false;

		//trigger voice
		if (destroyPuyosTimer == chainPopSpeed)
		{
			prepareVoice(chain, predictedChain);
		}
		startGarbage();
		//trigger animation
		if (destroyPuyosTimer == chainPopSpeed && playvoice >= 5)
		{
			if (playvoice < 10)
				characterAnimation.prepareAnimation(std::string("spell") + to_string(playvoice - 4));
			else if (playvoice == 11)
				characterAnimation.prepareAnimation(std::string("counter"));

		}
	}

	//change garbagetimer
	garbageTimer += garbageSpeed;

	if (garbageTimer > garbageEndTime)
	{
		endGarbage();
		garbageTimer = 0;
		endPhase();
	}
}

void player::counterGarbage()
{//almost the same as normal garbage phase, except no endphase
	garbageTimer += garbageSpeed;
	if (garbageTimer > garbageEndTime)
	{
		endGarbage();
		garbageTimer = 0;
	}
}

void player::endGarbage()
{
	//play sound
	if (EQ > 0 && m_lightEffect.size() > 0)
		data->snd.hit.Play(data);

	//Attack animation ends: check if any garbage sneaked past after all
	if (attdef == ATTACK || attdef == COUNTERATTACK)
	{//bad prediction of attack: there is garbage on your side after all and must be defended
		if (getGarbageSum() > 0 && (normalGarbage.GQ > 0 || feverGarbage.GQ > 0))
		{
			//defend
			activeGarbage->GQ -= EQ;
			//push defense through normal GQ if necessary?
			//be careful here! don't let normal.GQ go negative
			if (activeGarbage == &feverGarbage && normalGarbage.GQ > 0 && activeGarbage->GQ < 0)
			{
				normalGarbage.GQ += feverGarbage.GQ;
				feverGarbage.GQ = 0;
				if (normalGarbage.GQ < 0)
				{//don't let normal GQ go negative!
					feverGarbage.GQ += normalGarbage.GQ;
					normalGarbage.GQ = 0;
				}
				updateTray(&normalGarbage);
			}
			updateTray();
			//This part is different from real defense
			//garbage left: all is ok, defend ends.
			if (getGarbageSum() >= 0)
			{
				EQ = 0;
				attdef = NOATTACK;
			}
			//garbage is countered: put leftover back in EQ and continue attack, do not create counterattack animation!
			else
			{
				EQ = -1 * (getGarbageSum());
				normalGarbage.GQ = 0; normalGarbage.CQ = 0;
				feverGarbage.GQ = 0; feverGarbage.CQ = 0;
				updateTray();
			}
		}
	}

	//Normal Attack
	//add EQ to opponents CQ
	if (attdef == ATTACK || attdef == COUNTERATTACK)
	{
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i] != this)
			{
				targetGarbage[currentgame->players[i]]->CQ += EQ;
				//EQ=0;
				currentgame->players[i]->updateTray(targetGarbage[currentgame->players[i]]);
				playGarbageSound();
				//end of attack
				if (chain == predictedChain || (attdef == COUNTERATTACK && hasMoved))
				{
					//remove as accumulator
					currentgame->players[i]->removeAttacker(targetGarbage[currentgame->players[i]], this);
					//force player to check if zero (there should always be a last player)
					currentgame->players[i]->checkAnyAttacker(targetGarbage[currentgame->players[i]]);
				}
				currentgame->players[i]->checkFeverGarbage();
			}

		}
		//end of attack
		currentgame->currentruleset->onAttack(this);
		attdef = NOATTACK;
	}

	//Defense end
	if (attdef == DEFEND || attdef == COUNTERDEFEND)
	{
		activeGarbage->GQ -= EQ;
		//push defense through normal GQ if necessary?
		//be careful here! don't let normal.GQ go negative
		if (activeGarbage == &feverGarbage && normalGarbage.GQ > 0 && activeGarbage->GQ < 0)
		{
			normalGarbage.GQ += feverGarbage.GQ;
			feverGarbage.GQ = 0;
			if (normalGarbage.GQ < 0)
			{//don't let normal GQ go negative!
				feverGarbage.GQ += normalGarbage.GQ;
				normalGarbage.GQ = 0;
			}
			updateTray(&normalGarbage);
		}
		updateTray();

		//online players will not offset (?)
		if (m_type != ONLINE)
			currentgame->currentruleset->onOffset(this);

		//send message
		if (currentgame->connected && m_type == HUMAN)
			currentgame->network->sendToChannel(CHANNEL_GAME, "fo", currentgame->channelName);

		//THERE IS NO SUCH THING AS DEFENSE AND COUNTERDEFENSE
		//always check if defense can be countered at end!!
		attdef = COUNTERDEFEND;
	}

	//Counter defense
	if (attdef == COUNTERDEFEND)
	{
		//calculate DEF again
		int totalEQ = 0;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			//sum opponents EQ
			if (currentgame->players[i] != this)
				totalEQ += currentgame->players[i]->getAttackSum();
		}
		int DEF = getGarbageSum() + totalEQ;

		if (DEF >= 0)
		{//ineffective counter -> just end counter
			attdef = NOATTACK;
			//std::cout<<"End counter p"<<m_playernum<<std::endl;
		}
		else
		{//turn into attack
			attdef = COUNTERATTACK;
			for (size_t i = 0; i < currentgame->players.size(); i++)
			{
				if (currentgame->players[i] != this)
				{
					if (getGarbageSum() < 0)//no need to check again?
					{
						EQ = -1 * (getGarbageSum());
						normalGarbage.GQ = 0; normalGarbage.CQ = 0;
						feverGarbage.GQ = 0; feverGarbage.CQ = 0;
						updateTray();
					}

					//create new light effect animation for each opponent
					posVectorFloat startpv, middlepv, endpv;
					startpv = activeField->getTopCoord(-1);
					endpv = currentgame->players[i]->activeField->getTopCoord(-1);
					//calculate a middle
					int dir = m_nextPuyo.getOrientation();
					middlepv.x = startpv.x - dir * PUYOX * 3;
					middlepv.y = startpv.y - PUYOY * 3;
					m_lightEffect.push_back(new lightEffect(data, startpv, middlepv, endpv));

					//add self to garbage accumulator
					currentgame->players[i]->addAttacker(targetGarbage[currentgame->players[i]], this);
				}
			}
		}

	}

	//reset EQ
	if (attdef != COUNTERATTACK)
		EQ = 0;

}

void player::addNewColorPair(int n)
{//adds 2 new colors to nextlist and update nextpuyo
	int color1 = this->getRandom(n, m_randomizerNextList);
	int color2 = this->getRandom(n, m_randomizerNextList);

	//Perform TGM randomizer algorithm?
	if (currentgame->legacyRandomizer) {
		color1 = TGMR(color1, n);
		color2 = TGMR(color2, n);
	}

	m_nextList.push_back(color1);
	//Regenerate color 2 if type is quadruple and colors are the same
	if (useDropPattern)
	{
		while (color1 == color2 && getFromDropPattern(m_character, turns + 3) == QUADRUPLET)
		{
			color2 = this->getRandom(n, m_randomizerNextList);
		}
	}
	m_nextList.push_back(color2);

	//update nextpuyo
	if (useDropPattern)
		m_nextPuyo.update(m_nextList, m_character, turns);
	else
		m_nextPuyo.update(m_nextList, ARLE, turns);
}

void player::popColor()
{//clear front of deque
	if (m_nextList.size() > 6)
	{
		m_nextList.pop_front();
		m_nextList.pop_front();
	}
}

int player::TGMR(int color, int n)
{//check if color exists in last generated colors
	//do not apply TGMR if nextlist on the first 4 colors
	if (m_nextList.size() < 4)
		return color;

	std::deque<int>::iterator i;
	for (i = m_nextList.begin(); i != m_nextList.begin() + 4; i++)
	{
		if (*i == color)
		{
			color = this->getRandom(n, m_randomizerNextList);
			break;
		}
	}
	return color;
}

void player::checkAllClear()
{
	//check if you're the winner here
	checkWinner();
	if (activeField->count() == 0)
	{
		//play sound
		data->snd.allcleardrop.Play(data);
		//do something according to rules
		currentgame->currentruleset->onAllClear(this);
	}
	//end
	endPhase();
}

void player::checkLoser(bool endphase)
{
	//check if player places puyo on spawnpoint
	//doesnt necessarily mean losing, but the phase will change according to rules
	posVectorInt spawn = movePuyos.getSpawnPoint();
	bool lose = false;
	bool lose2 = false;
	lose = activeField->isPuyo(spawn.x, spawn.y);
	//is there a 2nd spawnpoint?
	if (currentgame->currentruleset->doubleSpawn)
		lose2 = activeField->isPuyo(spawn.x + 1, spawn.y);

	if (lose || lose2)
	{
		//player loses
		if (m_type != ONLINE)
		{
			setLose();
			//online game: send message to others
			if (currentgame->connected)
				currentgame->network->sendToChannel(CHANNEL_GAME, "l", currentgame->channelName.c_str());
		}
		else
		{
			//wait for message to arrive
			currentphase = WAITLOSE;
		}
	}
	else if (endphase)
		endPhase();
}

void player::checkWinner()
{
	//player wins if all others have lost
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
	//a winner is you
	if (losewin == NOWIN && Nlosers == currentgame->countActivePlayers() - 1 && Nlosers != 0 && active)
	{
		//save replay
		currentgame->saveReplay();
		wins++;

		currentgame->currentruleset->onWin(this);
		characterAnimation.prepareAnimation("win");

		//ranked match: prepare to disconnect
		prepareDisconnect();

	}
	//only player left after everyone else left the game
	if (currentgame->countActivePlayers() == 1 && currentgame->players.size() > 1 && active)
	{
		currentgame->currentruleset->onWin(this);
		characterAnimation.prepareAnimation("win");
	}

}

void player::setLose()
{
	currentgame->currentruleset->onLose(this);
	characterAnimation.prepareAnimation("lose");
	//play lose sound sound
	int totalplayers = currentgame->players.size();
	int activeplayers = 0;
	for (int i = 0; i < totalplayers; i++)
	{//check if all players are in win or lose state
		if (currentgame->players[i]->losewin == NOWIN)
		{
			activeplayers++;
		}
	}
	if (activeplayers > 1 && totalplayers > 0)
		data->snd.lose.Play(data);
	else if (totalplayers == 1) //endless
		data->snd.lose.Play(data);

}

void player::loseGame()
{
	//force others to check if winner
	//unless he's in the middle of chaining
	if (m_loseWinTimer == 0)
	{
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i] != this)
			{
				if (!(currentgame->players[i]->currentphase > 10 && currentgame->players[i]->currentphase < 40))
					currentgame->players[i]->checkWinner();
			}
		}
	}

	//drop puyos
//    if (activeField==&m_fieldNormal)
	activeField->loseDrop();
	//losetimer
	m_loseWinTimer++;

	if (m_loseWinTimer == 60)
	{
		characterVoices.lose.Play(data);
	}

	//online: wait for confirm message from everyone
	if (losewin == LOSEWAIT)
	{
		//count confirmed
		int count = 0;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i] != this && currentgame->players[i]->active && currentgame->players[i]->loseConfirm)
				count++;
		}
		//everybody confirmed your loss-> your state goes to lose
		if (count == currentgame->countActivePlayers() - 1)
		{
			losewin = LOSE;
			//again, check anyone else if winner
			for (size_t i = 0; i < currentgame->players.size(); i++)
			{
				if (currentgame->players[i] != this)
				{
					if (!(currentgame->players[i]->currentphase > 10 && currentgame->players[i]->currentphase < 40))
						currentgame->players[i]->checkWinner();
				}
			}

			//ranked match: report loss
			if (currentgame->settings->rankedMatch && m_type == HUMAN && currentgame->connected)
			{
				currentgame->network->sendToServer(9, "score");
				//maxwins reached
				prepareDisconnect();
			}

		}
	}
}

void player::winGame()
{
	m_loseWinTimer++;
	if (m_loseWinTimer == 120)
	{
		characterVoices.win.Play(data);
	}

}

void player::checkFever()
{
	//check if fevercount == 7
	if (feverGauge.getCount() == 7 && !feverMode)
	{
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		//clear field
		m_fieldFever.clearField();
		//all clear bonus: add feverchainamount and add time
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

	//wait until light has reached fevergauge
	if (feverGauge.getCount() == 7 && feverGauge.fullGauge())
	{
		feverMode = true;
		feverEnd = false;
		endPhase();
	}
	else if (feverGauge.getCount() != 7)
	{//no fever mode
		//all clear bonus: drop a 4 chain
		if (allClear == 1)
		{
			activeField->dropField(getFeverChain(getRandom(currentgame->currentruleset->NFeverChains, m_randomizerNextList), colors, 4, getRandom(colors, m_randomizerFeverColor)));
			//allClear=2; //do not reset allClear here, do it in ruleset.cpp, wait for field to drop
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

void player::startFever()
{//animation transition into fever
	//first 50 frames is to wait for light to hit fevergauge
	if (m_transitionTimer >= 50 && m_transitionTimer < 150)
		m_transitionTimer += 3;
	if (m_transitionTimer < 50)
		m_transitionTimer += 2;

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
		puyoBounceSpeed = 2.5;
		normalTray.align(activeField->getProperties().offsetX - (16) * m_globalScale, activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		normalTray.setDarken(true);
		feverSuccess = 0;
		//        feverGauge.setPositionSeconds(activeField->getTopCoord(0.5));
		endPhase();
	}

	//sound & animation
	if (m_transitionTimer == 50)
	{
		data->snd.fever.Play(data);
		characterVoices.fever.Play(data);
		characterAnimation.prepareAnimation("fever");
		if (data->windowFocus)
		{
			data->front->musicEvent(music_enter_fever);
			currentgame->currentVolumeFever = 0;
		}
	}

	//switch fields now
	if (m_transitionTimer == 101)
	{
		activeField = &m_fieldFever;
		activeGarbage = &feverGarbage;
	}

	//rotation animation
	if (m_transitionTimer > 50 && m_transitionTimer <= 100)
	{
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		//rotate out normal field
		m_transformScale = 1 - (m_transitionTimer - 50) / 50.0;
		activeField->setTransformScale(1 - (m_transitionTimer - 50) / 50.0);

	}
	else if (m_transitionTimer > 100 && m_transitionTimer <= 150)
	{
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		//rotate in fever field
		m_transformScale = (m_transitionTimer - 100) / 50.0;
		activeField->setTransformScale((m_transitionTimer - 100) / 50.0);
	}
}
void player::checkEndFeverOnline()
{
	//online player gets stuck in this state until message is received
	if (!messages.empty() && messages.front().compare("fe") == 0)
	{//fever ends
		messages.pop_front();
		feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		//check loser at the last moment
		if (currentgame->currentruleset->feverDeath)
			checkLoser(false); //do not end phase in this function
		endPhase();
	}
	else if (!messages.empty() && messages.front().compare("fc") == 0)
	{//fever continues;
		messages.pop_front();
		endPhase();
	}
}
void player::checkEndFever()
{
	//check if seconds == 0
	if (feverGauge.seconds == 0 && feverMode && m_type != ONLINE) //online players end fever when they give a message
	{
		//feverMode=false;
		feverEnd = true;
		m_transitionTimer = 0;
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);

		//send message BEFORE sending possible lose message
		if (m_type == HUMAN && currentgame->connected)
			currentgame->network->sendToChannel(CHANNEL_GAME, "fe", currentgame->channelName);

		//check loser at the last moment?
		if (currentgame->currentruleset->feverDeath)
			checkLoser(false); //do not end phase in this function
	}
	//not the end?
	if (!feverEnd && m_type == HUMAN && currentgame->connected)
		currentgame->network->sendToChannel(CHANNEL_GAME, "fc", currentgame->channelName);

	//check if player popped a chain
	if (poppedChain)
	{
		//update chains
		if (predictedChain >= currentFeverChainAmount)
		{//succes
			currentFeverChainAmount = predictedChain + 1;
			feverSuccess = 1;
			//if (feverGauge.seconds!=0)
			//    characterVoices.feversuccess.Play(global);
		}
		else if (predictedChain == currentFeverChainAmount - 1)
		{//no success

		}
		else
		{//failure
			int diff = currentFeverChainAmount - predictedChain;
			if (diff > 2)
				currentFeverChainAmount -= 2;
			else
				currentFeverChainAmount -= 1;
			feverSuccess = 2;
			//if (feverGauge.seconds!=0)
			//    characterVoices.feverfail.Play(global);
		}

		//time bonus
		if (predictedChain > 2 && feverGauge.seconds != 0 && feverGauge.seconds < 60 * feverGauge.maxSeconds)
		{
			feverGauge.addTime((predictedChain - 2) * 30);
			showSecondsObj((predictedChain - 2) * 30);
		}

		//all clear bonus
		if (allClear == 1)
		{
			allClear = 0;
			allclearTimer = 1;
			currentFeverChainAmount += 2;
			if (feverGauge.seconds < 60 * 30)
			{
				//do not add time if fever is actually ending
				if (feverGauge.seconds != 0)
				{
					feverGauge.addTime(5 * 60);
					showSecondsObj(5 * 60);
				}
			}
		}
		//minimum and maximum fever chain
		if (currentFeverChainAmount < 3)
			currentFeverChainAmount = 3;
		if (currentFeverChainAmount > 15)
			currentFeverChainAmount = 15;

		//clear field
		//if (feverGauge.seconds>0) //do not throw away at the last moment
		if (!feverEnd)
			activeField->throwAwayField();
	}
	endPhase();
}


void player::endFever()
{//transition out of fever
	if (m_transitionTimer < 100)
	{
		m_transitionTimer += 3;
	}
	else
	{
		m_transitionTimer = 100;
		m_transformScale = 1;
		chainPopSpeed = 25;
		garbageSpeed = 4.8;
		garbageEndTime = 100;
		puyoBounceEnd = 2;
		puyoBounceEnd = 50;
		//        feverGauge.resetPositionSeconds();
		endPhase();
	}

	//switch fields now
	if (m_transitionTimer == 51)
	{
		feverMode = false;
		//music: check if anyone is in fever
		bool stopMusic = true;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i]->feverMode == true && currentgame->players[i]->losewin == NOWIN && currentgame->players[i]->active)
			{
				stopMusic = false;
				break;
			}
		}
		if (stopMusic)
		{
			if (data->windowFocus)
				data->front->musicEvent(music_exit_fever);
			currentgame->currentVolumeNormal = 0;
		}
		activeField = &m_fieldNormal;
		activeGarbage = &normalGarbage;
		//add fevergarbage to normalgarbage, move accumulator
		//and switch any targets to fevergarbage to normalgarbage
		normalGarbage.CQ += feverGarbage.CQ; feverGarbage.CQ = 0;
		normalGarbage.GQ += feverGarbage.GQ; feverGarbage.GQ = 0;
		for (size_t i = 0; i < feverGarbage.accumulator.size(); i++)
		{//move accumulators to normalgarbage
			normalGarbage.accumulator.push_back(feverGarbage.accumulator[i]);
		}
		feverGarbage.accumulator.clear();
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{//set opponents targets to normalgarbage
			if (currentgame->players[i] != this)
			{
				currentgame->players[i]->targetGarbage[this] = &normalGarbage;
			}
		}
		updateTray(&normalGarbage);
		updateTray(&feverGarbage);

		//reset seconds and gauge
		feverGauge.seconds = 60 * 15;
		feverGauge.setCount(currentgame->currentruleset->initialFeverCount);

		//reset garbagetray
		normalTray.align(activeField->getProperties().offsetX, activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
		normalTray.setDarken(false);
	}

	//rotation animation
	if (m_transitionTimer > 0 && m_transitionTimer <= 50)
	{
		m_fieldSprite.setVisible(false);
		m_fieldFeverSprite.setVisible(true);
		//rotate out fever field
		m_transformScale = 1 - (m_transitionTimer) / 50.0;
		activeField->setTransformScale(1 - (m_transitionTimer) / 50.0);

	}
	else if (m_transitionTimer > 50 && m_transitionTimer <= 100)
	{
		m_fieldSprite.setVisible(true);
		m_fieldFeverSprite.setVisible(false);
		//rotate in fever field
		m_transformScale = (m_transitionTimer - 50) / 50.0;
		activeField->setTransformScale((m_transitionTimer - 50) / 50.0);
	}

}

void player::playFever()
{
	if (feverMode)
	{
		//play sounds
		if (feverGauge.seconds / 60 > 0 && feverGauge.seconds / 60 < 6 && feverGauge.seconds % 60 == 0 && losewin == NOWIN)
			data->snd.fevertimecount.Play(data);
		if (feverGauge.seconds == 1 && losewin == NOWIN)
			data->snd.fevertimeend.Play(data);

		//subtract seconds
		if (feverGauge.seconds > 0 && feverGauge.endless == false && losewin == NOWIN && !currentgame->stopChaining)
			feverGauge.seconds--;
		//display
		feverGauge.update();
	}
}

void player::dropFeverChain()
{
	int old = feverColor;
	while (old == feverColor)//background color
		feverColor = ppvs::getRandom(5);
	//drop field
	activeField->dropField(getFeverChain(getRandom(currentgame->currentruleset->NFeverChains, m_randomizerFeverChain), colors, currentFeverChainAmount, getRandom(colors, m_randomizerFeverColor)));
	calledRandomFeverChain++;

	//garbage should not drop after this
	forgiveGarbage = true;

	endPhase();
}

void player::showSecondsObj(int n)
{
	posVectorFloat pv = feverGauge.getPositionSeconds();
	m_secondsObj.push_back(new secondsObject(data));
	m_secondsObj.back()->setScale(m_globalScale);
	m_secondsObj.back()->showAt(m_nextPuyoOffsetX + (pv.x - 16) * m_nextPuyoScale * m_globalScale, m_nextPuyoOffsetY + (pv.y + 16) * m_nextPuyoScale * m_globalScale * m_secondsObj.size(), n);
}

void player::setMarginTimer()
{
	margintimer++;
}

void player::prepareVoice(int chain, int predictedChain)
{
	playvoice = getVoicePattern(chain, predictedChain, currentgame->currentruleset->voicePatternFever);
	//count diacute
	if (playvoice == 4)
		diacute++;
	stutterTimer = 0;
}

void player::playVoice()
{
	if (playvoice >= 0)
	{
		stutterTimer++;
		if (stutterTimer == 1)
		{
			characterVoices.chain[playvoice].Stop(data);
			characterVoices.chain[playvoice].Play(data);
		}
		if (diacute > 0 && stutterTimer == 1 && playvoice > 4)
		{
			stutterTimer = -10;
			diacute--;
		}
		if (diacute == 0)
			playvoice = -1;

	}
}

//{ Online stuff

void player::bindPlayer(std::string name, unsigned int id, bool setActive)
{
	onlineName = name;
	active = setActive;
	onlineID = id;
	//reset stuff
	messages.clear();
	wins = 0;
	loseConfirm = false;
}

void player::unbindPlayer()
{
	onlineName = "";
	onlineID = 0;
	active = false;
	prepareActive = false;
	rematch = false;
	rematchIcon.setVisible(false);
	messages.clear();
	//still playing
	if (losewin == NOWIN && currentphase != IDLE)
	{
		setLose();
		//add to replay
		if (currentgame->settings->recording == PVS_RECORDING)
		{
			messageEvent me = { data->matchTimer,"exit" };
			recordMessages.push_back(me);
		}

	}
	setStatusText(currentgame->translatableStrings.disconnected.c_str());

	//was player in choose color status?
	if (!pickedColor)
	{
		bool start = true;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (!currentgame->players[i]->active && !currentgame->settings->useCPUplayers)
				continue;
			if (!currentgame->players[i]->pickedColor || currentgame->players[i]->colorMenuTimer < -2)
			{
				start = false;
				break;
			}
		}
		if (start)
		{
			//players should be in pickcolor phase

			colorMenuTimer = 0;
			for (size_t i = 0; i < currentgame->players.size(); i++)
			{
				if (!currentgame->players[i]->active && !currentgame->settings->useCPUplayers)
					continue;

				if (currentgame->players[i]->currentphase == PICKCOLORS)
					currentgame->players[i]->currentphase = GETREADY;
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

void player::processMessage()
{
	//debug: display all messages
	/*
	debugstring="";
	for (stringList::iterator it=messages.begin();it!=messages.end();it++)
		debugstring+=(*it)+"\n";
	debugstring+=to_string(margintimer);
	*/
	//always clear currentMessage after using it, if it's not cleared, it indicates it has yet to be processed.
	/*if (currentMessageStr.compare("")==0 && !messages.empty())
	{
		//load newest
		currentMessageStr=messages.front();
		messages.pop_front();
	}*/
	//game continues, other functions pick up currentMessage

	//check if lose confirm was received
	//set to immediate check
	/*
	if (m_type==ONLINE && !messages.empty() && messages.front()[0]=='o')
	{
		//this player confirms your loss
		loseConfirm=true;
		messages.pop_front();
	}*/

	//skip move message if it's a big multiplayer match
	if (currentgame->players.size() > 10 && !messages.empty() && messages.front()[0] == 'm')
	{
		messages.pop_front();
	}

	//receive offset message
	if (m_type == ONLINE && !messages.empty() && messages.front().compare("fo") == 0)
	{
		currentgame->currentruleset->onOffset(this);
		messages.pop_front();
	}

	//replay: player exits
	if (currentgame->settings->recording == PVS_REPLAYING && m_type == ONLINE &&
		!messages.empty() && messages.front().compare("exit") == 0)
	{
		setLose();
	}


}

void player::addMessage(std::string mes)
{
	//add to replay
	if (currentgame->settings->recording == PVS_RECORDING)
	{
		messageEvent me = { data->matchTimer,"" };
		recordMessages.push_back(me);
		if (mes.length() < 64)
			strcpy(recordMessages.back().message, mes.c_str());
	}

	//process immediately?
	if (m_type == ONLINE && mes.compare("d") == 0)
	{
		waitForConfirm--;
		//do not add
		return;
	}

	//check if lose confirm was received
	if (m_type == ONLINE && mes.compare("o") == 0)
	{
		//this player confirms your loss
		loseConfirm = true;
		//do not add
		return;
	}

	//add message
	messages.push_back(mes);
}

void player::confirmGarbage()
{
	//moves < Min(4, Turns+1) -> moves should be less than 4, except the first turns

	//read everyone's confirm count, if there's anyone with confirm>4, you can't move on
	//(alternatively, we could count the total)
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		if (currentgame->players[i] != this && currentgame->players[i]->waitForConfirm > 3)
		{
			//also see garbagephase
			currentgame->stopChaining = true;
			return;
		}
	}

	currentgame->stopChaining = false;
	endPhase();
}

void player::waitGarbage()
{
	//ONLINE player gets stuck here until message about garbage arrives
	bool receive = false;

	//receive n: dropped nothing
	if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'n')

	{
		receive = true;
		//nothing else happens
		//currentMessageStr="";
		messages.pop_front();
	}

	if (m_type == ONLINE && !messages.empty() && messages.front()[0] == 'g')

	{
		//drop garbage
		int dropAmount = 0;//=to_int(currentMessage[1]);
		sscanf(messages.front().c_str(), "g|%i", &dropAmount);
		activeField->dropGarbage(false, dropAmount);
		receive = true;
		//currentMessageStr="";
		messages.pop_front();
	}

	if (receive == true)
	{
		//send message dO back
		if (currentgame->connected && m_type == ONLINE)
		{
			currentgame->network->sendToChannel(CHANNEL_GAME, "d", currentgame->channelName.c_str());
		}
		endPhase();
	}
}

void player::waitLose()
{//ignore all messages except the lose message
	if (!messages.empty())
	{
		if (!messages.empty() && messages.front()[0] == 'l')
		{//lose
			setLose();
			//send confirmation back to this particular player
			if (currentgame->connected)
				currentgame->network->sendToPeer(CHANNEL_GAME, "o", currentgame->channelName, onlineID);
		}
		//discard any message
		//currentMessageStr="";
		messages.pop_front();
	}
}

//}

//{ draw Code

void player::draw()
{//Draw everything from the player
	//drawFieldBack();//draw background
	//drawAllClear();//Draw all clear

	// Field coordinates
	// -----------------
	data->front->pushMatrix();
	data->front->translate(properties.offsetX, properties.offsetY, 0);
	data->front->scale(properties.scaleX * getGlobalScale(),
		properties.scaleY * getGlobalScale(), 1);

	data->front->clearDepth();
	data->front->setDepthFunction(lessOrEqual);
	data->front->setBlendMode(alphaBlending);
	data->front->setColor(0., 0., 0., 0.);
	data->front->drawRect(nullptr, 0, 0, 192, 336);
	// Fever flip coordinates
	// ----------------------
	data->front->pushMatrix();
	data->front->translate((192 / 2), 0, 0);
	data->front->scale(m_transformScale, 1, 1);
	data->front->translate(-(192 / 2), 0, 0);
	activeField->draw(); //draw to screen
	data->front->setDepthFunction(equal);
	movePuyos.draw(); //draw movePuyos on renderimage
	data->front->setDepthFunction(always);
	data->front->popMatrix();
	// ----------------------
	m_borderSprite.draw(data->front); //border
	pchainWord->draw(data->front); //draw chainword
	characterAnimation.draw();
	// -----------------
	data->front->popMatrix();

	// Next coordinates
	// ----------------
	data->front->pushMatrix();
	data->front->translate(m_nextPuyoOffsetX, m_nextPuyoOffsetY, 0);
	data->front->scale(m_nextPuyoScale, m_nextPuyoScale, 1);
	m_nextPuyo.draw(); //draw nextpuyo
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
	data->front->scale(properties.scaleX * getGlobalScale(),
		properties.scaleY * getGlobalScale(), 1);
	drawColorMenu();//draw choose color menu
	drawWin();//Draw win
	drawLose();//Draw lose

	//online; darken screen if player is not bound
	if (!currentgame->settings->useCPUplayers && (onlineName == "" || !active))
	{
		/*data->front->setBlendMode(alphaBlending);
		data->front->setColor(1., 1., 1., 0.375);
		data->front->drawRect(0, 0, 0, 192, 336);*/
		overlaySprite.draw(data->front);
	}

	//draw status text
	if (statusText)
	{
		data->front->setColor(255, 255, 255, 255);
		if ((m_type == ONLINE || m_type == HUMAN) && currentgame->currentGameStatus != GAMESTATUS_PLAYING
			&& currentgame->currentGameStatus != GAMESTATUS_SPECTATING && !currentgame->settings->useCPUplayers)
			statusText->draw(0, 0);
		else if (currentgame->forceStatusText)
			statusText->draw(0, 0);
	}
	//debug
//    char db[512];
//    sprintf(db,"attackers: %i\nCQ: %i\nGQ: %i\n%i",normalGarbage.accumulator.size(),normalGarbage.CQ,normalGarbage.GQ,debugCounter);
//    setStatusText(db);
//    statusText->draw(0, 0);
	// -----------------
	data->front->popMatrix();

	if (showCharacterTimer > 0 && currentgame->currentGameStatus != GAMESTATUS_PLAYING)
	{
		charHolderSprite.draw(data->front);
		currentCharacterSprite.draw(data->front);
		for (int i = 0; i < 16; i++)
			dropset[i].draw(data->front);
	}

	if (currentgame->currentGameStatus != GAMESTATUS_PLAYING && currentgame->currentGameStatus != GAMESTATUS_SPECTATING)
		rematchIcon.draw(data->front);

}

void player::drawEffect()
{//draw light effect, secondsobj
	for (size_t i = 0; i < m_lightEffect.size(); i++)
	{
		m_lightEffect[i]->draw(data->front);
	}

	//feverlight
	m_feverLight.draw(data->front);

	for (size_t i = 0; i < m_secondsObj.size(); i++)
	{
		m_secondsObj[i]->draw(data->front);
	}

}

void player::drawFieldBack(posVectorFloat /*position*/, float rotation)
{
	m_fieldSprite.setPosition(0, 0);
	m_fieldSprite.setCenter(0, 0);
	m_fieldSprite.setScale(1, 1);
	m_fieldSprite.setRotation(rotation);
	m_fieldSprite.draw(data->front);
}

// C99 round
double _round(double r) { return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5); }

void player::drawFieldFeverBack(posVectorFloat /*position*/, float rotation)
{
	if (!feverMode)
		return;
	m_fieldFeverSprite.setPosition(0, 0);
	m_fieldFeverSprite.setCenter(0, 0);
	m_fieldFeverSprite.setScale(1, 1);
	m_fieldFeverSprite.setRotation(rotation);
	//transition to new color
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
		//animate
		m_fieldFeverSprite.setImage(data->imgFeverBack[(data->globalTimer / 2) % 30]);
		m_fieldFeverSprite.setColor(feverColorR * 255, feverColorG * 255, feverColorB * 255);
		m_fieldFeverSprite.draw(data->front);
	}
}

void player::drawAllClear(posVectorFloat pos, float scaleX, float scaleY, float rotation)
{
	fieldProp p = activeField->getProperties();
	float dist = activeField->getFieldSize().y * 0.67;
	float x = pos.x - dist * sin(rotation * PI / 180);
	float y = pos.y - dist * cos(rotation * PI / 180);

	//tsu type
	if (allClear == 1)
	{//just draw it until it clears
		m_allclearSprite.setVisible(true);
		m_allclearSprite.setPosition(x, y);
		m_allclearSprite.setScale(1, 1);
		m_allclearSprite.setRotation(rotation);
		m_allclearSprite.draw(data->front);
	}
	//fever type: use timer
	if (allclearTimer >= 1)
	{
		allclearTimer++;
		m_allclearSprite.setPosition(x, y);
		m_allclearSprite.setScale(1, 1);
		m_allclearSprite.setRotation(rotation);
		if (allclearTimer >= 60)
		{//flicker
			//m_allclearSprite.setVisible((allclearTimer/2)%2);
			m_allclearSprite.setScale(scaleX + scaleX * (allclearTimer - 60) / 10.0, scaleY - scaleY * (allclearTimer - 60) / 10.0);
			m_allclearSprite.setTransparency(1 - (allclearTimer - 60) / 10.0);
		}
		else if (allclearTimer > 1 && allclearTimer < 60)
		{//show normally
			m_allclearSprite.setVisible(true);
		}
		//draw
		m_allclearSprite.draw(data->front);
		//end
		if (allclearTimer >= 70)
			allclearTimer = 0;
	}
}

void player::drawCross(frendertarget* r)
{
	//fevermode: do not draw crosses?
//    if (feverMode)
//        return;
	//fieldProp p=activeField->getProperties();
	//place at spawnpoint
	posVectorInt spv = movePuyos.getSpawnPoint();
	spv.y += 1;
	posVectorFloat rpv = activeField->getLocalCoord(spv.x, spv.y);

	m_crossSprite.setPosition(rpv.x, rpv.y);
	m_crossSprite.draw(r);

	//check if 2nd spawnpoint
	if (currentgame->currentruleset->doubleSpawn)
	{
		rpv = activeField->getLocalCoord(spv.x + 1, spv.y);
		m_crossSprite.setPosition(rpv.x, rpv.y);
		m_crossSprite.draw(r);
	}
}

void player::drawLose()
{
	fieldProp p = activeField->getProperties();
	posVectorFloat position = activeField->getBottomCoord(true);
	position.y -= activeField->getFieldSize().y * 0.75;

	if (losewin == LOSE && m_loseWinTimer > 60)
	{
		m_loseSprite.setPosition(position);
		m_loseSprite.setScale((1 + 0.1 * sin((m_loseWinTimer - 60) / 20.0)));
		m_loseSprite.draw(data->front);
	}
}

void player::drawWin()
{
	fieldProp p = activeField->getProperties();
	posVectorFloat position = activeField->getBottomCoord(true);
	position.y -= activeField->getFieldSize().y * 0.75;
	if (losewin == WIN)
	{
		m_winSprite.setPosition(position.x, position.y + 10 * sin(m_loseWinTimer / 20.0));
		m_winSprite.setScale(1);
		m_winSprite.draw(data->front);
	}
}

void player::drawColorMenu()
{
	for (int i = 0; i < 9; i++)
		colorMenuBorder[i].draw(data->front);
	for (int i = 0; i < 5; i++)
		spice[i].draw(data->front);
}
//}

void player::setStatusText(const char* utf8)
{
	if (utf8 == lastText) return;
	if (!statusFont) return;
	if (statusText) delete statusText;

	statusText = statusFont->render(utf8);
	lastText = utf8;
}

void player::getUpdate(std::string str)
{
	char fieldstring[500];
	char feverstring[500];

	//0[spectate]1[currentphase]2[fieldstringnormal]3[fevermode]4[fieldfever]5[fevercount]
	//6[rng seed]7[fever rng called]8[turns]9[colors]
	//10[margintimer]11[chain]12[currentFeverChainAmount]13[nGQ]14[fGQ]
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

	//initialize
	setRandomSeed(rngseed, &m_randomizerNextList);
	colors = clrs;
	initValues(rngseed + onlineID);
	//set other stuff
	currentphase = phase(ph);
	m_fieldNormal.setFieldFromString(fieldstring);
	if (fm == 0)
		feverMode = false;
	else
		feverMode = true;
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
	//call nextlist random
	for (int i = 0; i < turns; i++)
	{
		getRandom(0, m_randomizerNextList);
		getRandom(0, m_randomizerNextList);
	}
	margintimer = mrgntmr;
	chain = chn;
	predictedChain = prdctchn;
	allClear = allclr;

	//set target garbage, otherwise it may crash
	for (size_t i = 0; i < currentgame->players.size(); i++)
	{
		if (currentgame->players[i] != this)
		{
			targetGarbage[currentgame->players[i]] = currentgame->players[i]->activeGarbage;
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
		puyoBounceSpeed = 2.5;
		normalTray.align(activeField->getProperties().offsetX - (16) * m_globalScale, activeField->getProperties().offsetY - (32 + 16) * m_globalScale, m_globalScale);
		normalTray.setDarken(true);
	}
}

void player::prepareDisconnect()
{
	if (currentgame->settings->rankedMatch && m_type == HUMAN && currentgame->connected)
	{
		//maxwins reached
		bool disconnect = false;
		for (size_t i = 0; i < currentgame->players.size(); i++)
		{
			if (currentgame->players[i]->wins == currentgame->settings->maxWins)
			{
				//prepare to disconnect channel
				disconnect = true;
			}
		}
		if (disconnect)
		{
			currentgame->rankedState = 1;
			currentgame->network->requestChannelDescription(currentgame->channelName, std::string(""));
			//for (size_t i=0;i<currentgame->players.size();i++)
			//    currentgame->players[i]->unbindPlayer();
		}

	}
}
void player::setDropsetSprite(int x, int y, puyoCharacter pc)
{
	//get total width and center dropset
	float length = 0;
	float xx = 0;
	float scale = m_globalScale * 0.75;
	for (int j = 0; j < 16; j++)
	{
		movePuyoType mpt = getFromDropPattern(pc, j);
		if (mpt == DOUBLET)
			length += 10;
		else
			length += 18;
	}
	xx = -length / 2.0 - 5;
	//xx=-128;

	for (int j = 0; j < 16; j++)
	{
		movePuyoType mpt = getFromDropPattern(pc, j);
		dropset[j].setPosition(x + xx * scale, y);
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

}
