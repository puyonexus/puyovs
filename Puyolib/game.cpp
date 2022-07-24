#include "game.h"
#include <ctime>
#include <fstream>
#include <cstdio>
#include <zlib.h>
#include "../PVS_ENet/PVS_Client.h"
#include "../PVS_ENet/PVS_Channel.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace ppvs
{

void encode(const char *key,char* in,int length)
{
	for (int i = 0; i < length; i++)
	{
		in[i] = in[i] ^ key[i % strlen(key)];
	}
}

game* activegame = nullptr;

game::game(gameSettings* gs)
{
	initGlobal();
	data = nullptr;
	currentruleset = nullptr;

	settings = gs;
	connected = false;
	menuSelect = 0;
	currentGameStatus = GAMESTATUS_WAITING;
	stopChaining = false;
	activegame = this;
	if (gs->rankedMatch == true) {
		choiceTimer = 5 * 60;
	}
	else {
		choiceTimer = 30 * 60;
	}
	colorTimer = 10 * 60;
	activeAtStart = 0;
	replayTimer = 3 * 60;
	rankedTimer = 15 * 60;
	rankedState = 0;
	currentReplayVersion = 0;
	replayState = REPLAYSTATE_NORMAL;
	replayBackwardsTimer = 0;
	backwardsOnce = false;
	legacyRandomizer = false;
	legacyNuisanceDrop = false;

	playNext = true;
	forceStatusText = false;

	targetVolumeNormal = 100;
	currentVolumeNormal = 0;
	targetVolumeFever = 100;
	currentVolumeFever = 100;
	globalVolume = 1.0f;

	black.setImage(nullptr);
	black.setScale(640 * 2, 480);
	black.setColor(0, 0, 0);
	black.setTransparency(0.5f);
	black.setPosition(-640 / 2, -480 / 4);

	statusFont = nullptr;
	statusText = nullptr;
}

game::~game()
{
	// Delete players
	while (!players.empty())
	{
		delete players.back();
		players.pop_back();
	}

	// Delete associated global properties
	delete settings;
	delete charSelectMenu;
	delete mainMenu;
	delete currentruleset;
	delete statusText;
	delete statusFont;
	delete data->front;
	delete data;
}

void game::close()
{
	// Close game
	runGame = false;
}

void game::loadGlobal()
{
	// Load usersettings
	data->gUserSettings.str_background = settings->background;
	data->gUserSettings.str_puyo = settings->puyo;
	data->gUserSettings.str_sfx = settings->sfx;

	// Set global images
	loadImages();

	// Load all sounds
	loadSounds();

	// Init shaders
	data->glowShader = nullptr;
	data->tunnelShader = nullptr;

	if (useShaders)
	{
		const static char* glowShaderSource =
			"uniform sampler2D tex;"
			"uniform float color;"
			"void main()"
			"{"
			"   gl_FragColor=texture2D(tex,gl_TexCoord[0].xy)+vec4(color,color,color,0);"
			"}";

		data->glowShader = data->front->loadShader(glowShaderSource);
		if (data->glowShader)
		{
			data->glowShader->setCurrentTexture("tex");
			data->glowShader->setParameter("color", 1.0f);
		}

		const static char* tunnelShaderSource =
			"uniform vec4 cl;"
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

		data->tunnelShader = data->front->loadShader(tunnelShaderSource);
		if (data->tunnelShader)
		{
			data->tunnelShader->setCurrentTexture("tex");
			data->tunnelShader->setParameter("time", 1.0f);
			data->tunnelShader->setParameter("cl", 0.0f, 0.0f, 1.0f, 1.0f);
		}
	}

	// Other
	data->globalTimer = 0;
	data->windowFocus = true;
	data->playSounds = settings->playSound;
	data->playMusic = settings->playMusic;

	statusFont = data->front->loadFont("Arial", 14);
	setStatusText("");
}

void game::loadSounds() const
{
	sounds& snd = data->snd;
	setBuffer(snd.chain[0], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain1.ogg"))));
	setBuffer(snd.chain[1], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain2.ogg"))));
	setBuffer(snd.chain[2], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain3.ogg"))));
	setBuffer(snd.chain[3], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain4.ogg"))));
	setBuffer(snd.chain[4], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain5.ogg"))));
	setBuffer(snd.chain[5], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain6.ogg"))));
	setBuffer(snd.chain[6], (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/chain7.ogg"))));
	setBuffer(snd.allcleardrop, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/allclear.ogg"))));
	setBuffer(snd.drop, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/drop.ogg"))));
	setBuffer(snd.fever, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/fever.ogg"))));
	setBuffer(snd.feverlight, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/feverlight.ogg"))));
	setBuffer(snd.fevertimecount, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/fevertimecount.ogg"))));
	setBuffer(snd.fevertimeend, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/fevertimeend.ogg"))));
	setBuffer(snd.go, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/go.ogg"))));
	setBuffer(snd.heavy, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/heavy.ogg"))));
	setBuffer(snd.hit, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/hit.ogg"))));
	setBuffer(snd.lose, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/lose.ogg"))));
	setBuffer(snd.move, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/move.ogg"))));
	setBuffer(snd.nuisance_hitL, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/nuisance_hitL.ogg"))));
	setBuffer(snd.nuisance_hitM, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/nuisance_hitM.ogg"))));
	setBuffer(snd.nuisance_hitS, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/nuisance_hitS.ogg"))));
	setBuffer(snd.nuisanceL, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/nuisanceL.ogg"))));
	setBuffer(snd.nuisanceS, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/nuisanceS.ogg"))));
	setBuffer(snd.ready, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/ready.ogg"))));
	setBuffer(snd.rotate, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/rotate.ogg"))));
	setBuffer(snd.win, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/win.ogg"))));
	setBuffer(snd.decide, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/decide.ogg"))));
	setBuffer(snd.cancel, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/cancel.ogg"))));
	setBuffer(snd.cursor, (data->front->loadSound(folder_user_sounds + data->gUserSettings.str_sfx + std::string("/cursor.ogg"))));
}

// Load images (user defined)
void game::loadImages() const
{
	// Load puyo
	data->imgPuyo = data->front->loadImage(folder_user_puyo + data->gUserSettings.str_puyo + std::string(".png"));

	// Check rotation center of quadruplet
	if (data->imgPuyo && !data->imgPuyo->error() && data->imgPuyo->height() > 10)
	{
		for (int i = 0; i < PUYOX; i++)
		{
			if (data->imgPuyo->pixel(11 * PUYOX - i, 14 * PUYOY).a > 50)
			{
				data->PUYOXCENTER = PUYOX - i;
				break;
			}
		}
	}
	data->imgPuyo->setFilter(linearFilter);

	// Lights
	data->imgLight = data->front->loadImage("Data/Light.png");
	data->imgLight_s = data->front->loadImage("Data/Light_s.png");
	data->imgLight_hit = data->front->loadImage("Data/Light_hit.png");
	data->imgfsparkle = data->front->loadImage("Data/CharSelect/fsparkle.png");
	data->imgfLight = data->front->loadImage("Data/fLight.png");
	data->imgfLight_hit = data->front->loadImage("Data/fLight_hit.png");

	data->imgTime = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/time.png"));

	// Menu
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			// Safe (because i/j have defined ranges)
			char buffer[128];
			sprintf(buffer, "Data/Menu/menu%i%i.png", i, j);
			data->imgMenu[i][j] = data->front->loadImage(buffer);
		}
	}

	// Backgrounds
	data->imgBackground = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/back.png"));
	data->imgFieldFever = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/ffield.png"));

	// Background of next puyo
	data->imgNextPuyoBackgroundR = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/nextR.png"));
	data->imgNextPuyoBackgroundL = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/nextL.png"));

	if (!useShaders)
	{
		for (int i = 0; i < 30; ++i)
		{
			data->imgFeverBack[i] = data->front->loadImage(std::string("Data/Fever/f" + to_string(i) + ".png").c_str());
		}
	}

	// Load default fields. Custom fields should be loaded per character
	data->imgField1 = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/field1.png"));
	data->imgField2 = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/field2.png"));
	data->imgBorder1 = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/border1.png"));
	data->imgBorder2 = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/border2.png"));
	data->imgPlayerBorder = data->front->loadImage("Data/border.png");
	data->imgSpice = data->front->loadImage("Data/spice.png");

	// Other
	data->imgScore = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/score.png"));
	data->imgAllClear = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/allclear.png"));
	data->imgLose = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/lose.png"));
	data->imgWin = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/win.png"));
	data->imgFeverGauge = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/fgauge.png"));
	data->imgSeconds = data->front->loadImage(folder_user_backgrounds + data->gUserSettings.str_background + std::string("/fcounter.png"));
	data->imgCharHolder = data->front->loadImage("Data/CharSelect/charHolder.png");
	data->imgNameHolder = data->front->loadImage("Data/CharSelect/nameHolder.png");
	data->imgBlack = data->front->loadImage("Data/CharSelect/black.png");
	data->imgDropset = data->front->loadImage("Data/CharSelect/dropset.png");
	data->imgChain = data->front->loadImage(std::string("User/Backgrounds/") + data->gUserSettings.str_background + std::string("/chain.png"));
	data->imgCheckmark = data->front->loadImage("Data/checkmark.png");
	data->imgPlayerCharSelect = data->front->loadImage("Data/CharSelect/charSelect.png");

	for (int i = 0; i < NUM_CHARACTERS; ++i)
	{
		data->imgCharField[i] = data->front->loadImage(folder_user_character + settings->characterSetup[i] + "/field.png");
		data->imgCharSelect[i] = data->front->loadImage(folder_user_character + settings->characterSetup[i] + "/select.png");
		data->imgCharName[i] = data->front->loadImage(folder_user_character + settings->characterSetup[i] + "/name.png");
		data->imgSelect[i] = data->front->loadImage(folder_user_character + settings->characterSetup[i] + "/select.png");
	}

	data->imgPlayerNumber = data->front->loadImage("Data/CharSelect/playernumber.png");
}

void game::initPlayers()
{
	// Create players according to settings
	int n = 1;
	for (int i = 0; i < settings->Nhumans; i++)
	{
		// Assign human players
		addPlayer(HUMAN, n, settings->Nplayers);
		// Give human controls
		n++;
	}

	if (settings->useCPUplayers)
	{
		for (int i = 0; i < settings->Nplayers - settings->Nhumans; i++)
		{
			// CPU players
			addPlayer(CPU, n, settings->Nplayers);
			n++;
		}
	}
	else
	{
		for (int i = 0; i < settings->Nplayers - settings->Nhumans; i++)
		{
			// Online players
			addPlayer(ONLINE, n, settings->Nplayers);
			n++;
		}
	}
}

// Count players that haven't lost
int game::getActivePlayers() const
{
	int pl = 0;
	for (const auto& player : players)
	{
		if (player->losewin == NOWIN && player->active)
		{
			pl++;
		}
	}
	return pl;
}

void game::initGame(frontend* f)
{
	data = new gameData;
	data->front = f;

	loadGlobal();

	// Set random seed
	randomSeed_NextList = getRandom(100000);

	// Rule settings
	setRules();

	// Set background
	m_spriteBackground.setImage(data->imgBackground);
	m_spriteBackground.setPosition(0.f, 0.f);
	initPlayers();
	runGame = true;
	if (!network) // Start with character select menu or not
	{
		menuSelect = static_cast<int>(settings->startWithCharacterSelect);
	}
	timerEndMatch = 0;

	// Initialize readygo animation
	readyGoObj.init(data, posVectorFloat(320, 240), 1, folder_user_backgrounds + data->gUserSettings.str_background + "/Animation/", "ready.xml", 3 * 60);
	backgroundAnimation.init(data, posVectorFloat(320, 240), 1, folder_user_backgrounds + data->gUserSettings.str_background + "/Animation/", "animation.xml", 30 * 60);
	backgroundAnimation.prepareAnimation("background");

	// Other stuff
	charSelectMenu = new characterSelect(this);
	charSelectMenu->prepare();
	mainMenu = new menu(this);

	timerSprite[0].setImage(data->imgPlayerNumber);
	timerSprite[1].setImage(data->imgPlayerNumber);
	timerSprite[0].setSubRect(0 / 10 * 24, 0, 0, 32);
	timerSprite[1].setSubRect(0 / 10 * 24, 0, 0, 32);
	timerSprite[0].setCenterBottom();
	timerSprite[1].setCenterBottom();
	timerSprite[0].setPosition(640 - 24, 32);
	timerSprite[1].setPosition(640 - 24 - 24, 32);
}

// Set rules from rulesetinfo
void game::setRules()
{
	delete currentruleset;

	// Create rule object
	switch (settings->rulesetInfo.rulesetType)
	{
	case ENDLESS:
		currentruleset = new ruleset_ENDLESS();
		break;
	case TSU:
		currentruleset = new ruleset_TSU();
		break;
	case FEVER:
		currentruleset = new ruleset_FEVER();
		break;
	case ENDLESSFEVER:
		currentruleset = new ruleset_ENDLESSFEVER();
		break;
	case ENDLESSFEVERVS:
		currentruleset = new ruleset_ENDLESSFEVERVS();
		break;
	case TSU_ONLINE:
		currentruleset = new ruleset_TSU_ONLINE();
		break;
	case FEVER_ONLINE:
		currentruleset = new ruleset_FEVER_ONLINE();
		break;
	case FEVER15_ONLINE:
		currentruleset = new ruleset_FEVER15_ONLINE();
		break;
	case ENDLESSFEVERVS_ONLINE:
		currentruleset = new ruleset_ENDLESSFEVERVS_ONLINE();
		break;
	default:
		currentruleset = new ruleset_ENDLESS();
		break;
	}
	currentruleset->setGame(this);
	currentruleset->marginTime = settings->rulesetInfo.marginTime * 60;
	if (settings->rulesetInfo.targetPoint > 0)
		currentruleset->targetPoint = settings->rulesetInfo.targetPoint;
	if (settings->rulesetInfo.feverPower > 0)
		currentruleset->feverPower = static_cast<float>(settings->rulesetInfo.feverPower) / 100.0f;
	if (settings->rulesetInfo.puyoToClear > 0)
		currentruleset->puyoToClear = settings->rulesetInfo.puyoToClear;
	if (settings->rulesetInfo.requiredChain >= 0)
		currentruleset->requiredChain = settings->rulesetInfo.requiredChain;
	if (settings->rulesetInfo.initialFeverCount >= 0 && settings->rulesetInfo.initialFeverCount <= 7)
		currentruleset->initialFeverCount = settings->rulesetInfo.initialFeverCount;
	if (settings->rulesetInfo.quickDrop)
		currentruleset->quickDrop = settings->rulesetInfo.quickDrop;
	if (settings->rulesetInfo.colors >= 3 && settings->rulesetInfo.colors <= 5)
	{
		for (const auto& player : players)
			player->colors = settings->rulesetInfo.colors;
		settings->pickColors = false;
	}
	else
	{
		for (const auto& player : players)
			player->colors = 4;
	}
}

bool game::isFever() const
{
	bool fever = false;
	for (const auto& player : players)
	{
		if (player->active && player->feverMode)
		{
			fever = true;
		}
	}
	return fever;
}

void game::playGame()
{
	if (settings->recording == PVS_REPLAYING && replayState == REPLAYSTATE_PAUSED)
	{
		return;
	}

	// Set controller states
	for (int i = 0; i < players.size(); i++)
	{
		if (settings->useCPUplayers && i >= settings->Nhumans)
			break;

		finput input = data->front->inputState(i);
		players[i]->controls.setstate(input, data->matchTimer);
	}

	// Active player when NOT playing
	for (const auto& player : players)
	{
		if (currentGameStatus != GAMESTATUS_PLAYING && player->prepareActive)
		{
			player->active = true;
			player->prepareActive = false;
		}
	}

	// Set choice timer
	if (currentGameStatus == GAMESTATUS_IDLE && !settings->useCPUplayers)
	{
		timerSprite[0].setSubRect(((choiceTimer / 60) % 10) * 24, 0, 24, 32);
		timerSprite[1].setSubRect((choiceTimer / 60 / 10) * 24, 0, 24, 32);

		timerSprite[1].setVisible(choiceTimer > 10 * 60);

		if (choiceTimer > 0)
		{
			choiceTimer--;
		}

		// No-one to rematch
		if (countActivePlayers() <= 1 && settings->rankedMatch == false)
		{
			choiceTimer = -1;
		}

		// 2 player game: no count down
		if (players.size() == 2 && settings->rankedMatch == false)
		{
			choiceTimer = -1;
		}

		// Check if new player came on
		if (choiceTimer < 0 && countActivePlayers()>1 && !settings->rankedMatch)
		{
			choiceTimer = 30 * 60;
		}

		// Ranked: start counting as soon as opponent enters
		if (choiceTimer < 0 && countBoundPlayers()>1 && settings->rankedMatch == true)
		{
			choiceTimer = 5 * 60;
		}

		// Time is up
		if (choiceTimer == 0)
		{
			players[0]->controls.release();
			players[0]->controls.A = 1;
			// Select rematch
			mainMenu->select = 0;
		}

	}
	// Color timer menu
	if (players[0]->getPlayerType() == HUMAN && players[0]->currentphase == PICKCOLORS && !settings->useCPUplayers)
	{
		timerSprite[0].setSubRect(((colorTimer / 60) % 10) * 24, 0, 24, 32);
		timerSprite[1].setSubRect((colorTimer / 60 / 10) * 24, 0, 24, 32);

		if (colorTimer > 10 * 60)
		{
			timerSprite[1].setVisible(true);
		}
		else
		{
			timerSprite[1].setVisible(false);
		}

		if (colorTimer > 0)
		{
			colorTimer--;
		}
	}
	// Replay timer
	if (settings->recording == PVS_REPLAYING && replayTimer > 0)
	{
		replayTimer--;
		if (replayTimer == 1)
		{
			// Load new replay
			nextReplay();
		}
	}

	// Menus (display them over the main game)
	// Character select
	if (menuSelect == 1)
	{
		charSelectMenu->play();
	}
	else if (menuSelect == 2)
	{
		mainMenu->play();
	}

	// Check if match can start
	if (currentGameStatus == GAMESTATUS_REMATCHING)
	{
		int rematching = 0;
		int active = 0;
		for (const auto& player : players)
		{
			if (player->rematch)
			{
				rematching++;
			}
			if (player->active)
			{
				active++;
			}
		}
		if (active == rematching && rematching > 1 && active > 0)
		{
			// Start
			// Reset rematch status
			for (const auto& player : players)
			{
				player->rematch = false;
				player->rematchIcon.setVisible(false);
			}
			if (!settings->spectating)
			{
				currentGameStatus = GAMESTATUS_PLAYING;
			}
			else
			{
				currentGameStatus = GAMESTATUS_SPECTATING;
				menuSelect = 0;
			}

			// Start game
			resetPlayers();
		}
		else if (rematching == 0 && active == 0)
		{
			currentGameStatus = GAMESTATUS_IDLE;
		}
	}

	// Ready go
	if (!players.empty())
	{
		readyGoObj.playAnimation();
	}
	backgroundAnimation.playAnimation();

	// Replay
	if (settings->recording == PVS_REPLAYING)
	{
		const int t = data->matchTimer;
		for (const auto& player : players)
		{
			// Check event by looping through vector
			// If the time is -1, it's considered as processed
			for (size_t i = 0; i < player->recordMessages.size(); i++)
			{
				if (player->recordMessages[i].time == t)
				{
					// Process all events with the correct time
					// Ignore color select
					if (player->recordMessages[i].message[0] == 's'
						|| player->recordMessages[i].message[0] == 'c')
					{
						continue;
					}

					// Add message
					player->addMessage(player->recordMessages[i].message);
					player->recordMessages[i].time = -1;
				}
				else if (player->recordMessages[i].time > t)
				{
					break;
				}
			}
		}
	}

	rankedMatch();

	// Main game
	for (const auto& player : players)
	{
		player->play();
	}

	// Check end of match
	checkEnd();

	// Set status text
	for (size_t i = 0; i < players.size(); i++)
	{
		std::string str;
		if (settings->recording == PVS_REPLAYING)
		{
			if (players[i]->onlineName.empty())
				continue;

			// Show playername and wins
			if (settings->showNames == 0 || (settings->showNames == 1 && i == 0))
			{
				str += players[i]->onlineName + "\n";
			}
			else
			{
				str += "...\n";
			}
			str += "Wins: " + to_string(players[i]->wins) + "\n";
			players[i]->setStatusText(str.c_str());
			continue;
		}

		// Player is bound
		if (!players[i]->onlineName.empty())
		{
			str += players[i]->onlineName + "\n";
			str += "Wins: " + to_string(players[i]->wins) + "\n";

			if (players[i]->getPlayerType() == ONLINE)
			{
				if (!players[i]->active)
					str += "Waiting to join...\n";
				else
				{
					if (players[i]->pickingCharacter && (currentGameStatus == GAMESTATUS_IDLE || currentGameStatus == GAMESTATUS_REMATCHING))
						str += "Currently picking\n a character.";
					else if (!players[i]->rematch && (currentGameStatus == GAMESTATUS_IDLE || currentGameStatus == GAMESTATUS_REMATCHING))
						str += "Waiting for rematch.\n";
				}
			}
			players[i]->setStatusText(str.c_str());
		}
	}

	changeMusicVolume();

	data->globalTimer++;
	data->matchTimer++;

	// Access menu during endless mode
	if (settings->rulesetInfo.Nplayers == 1 && menuSelect == 0)
	{
		if (!players.empty() && players[0]->losewin == NOWIN && players[0]->controls.Start > 0)
		{
			players[0]->setLose();
		}
	}

	if (backwardsOnce)
		return;

	// Replay normal
	if (settings->recording == PVS_REPLAYING && replayState == REPLAYSTATE_NORMAL)
		replayBackwardsTimer = data->matchTimer;

	// Replay backwards
	if (settings->recording == PVS_REPLAYING && replayState == REPLAYSTATE_REWIND)
	{
		if (replayBackwardsTimer > 0)
		{
			replayBackwardsTimer -= 60;
		}

		if (!settings->oldReplayPlayList.empty())
		{
			const bool soundSettings = settings->playSound;
			settings->playSound = false;
			data->playSounds = false;

			loadReplay(settings->oldReplayPlayList.back());
			// Get ready to play
			resetPlayers();
			data->matchTimer = 0;
			// Loop until time is reached
			while (data->matchTimer < replayBackwardsTimer)
			{
				replayState = REPLAYSTATE_NORMAL;
				backwardsOnce = true;
				playGame();
				replayState = REPLAYSTATE_REWIND;
			}
			backwardsOnce = false;
			settings->playSound = soundSettings;
			data->playSounds = soundSettings;
			replayState = REPLAYSTATE_NORMAL;
		}
		else
		{
			replayBackwardsTimer = data->matchTimer;
			replayState = REPLAYSTATE_NORMAL;
		}
	}
	// Replay fastforward
	if (settings->recording == PVS_REPLAYING && replayState == REPLAYSTATE_FASTFORWARD && data->matchTimer % 2 == 0)
	{
		playGame();
	}
	else if (settings->recording == PVS_REPLAYING && replayState == REPLAYSTATE_FASTFORWARDX4 && data->matchTimer % 4 == 0)
	{
		playGame();
		playGame();
		playGame();
	}
}

void game::renderGame()
{
	// Tunnel shader
	if (data->tunnelShader)
	{
		data->tunnelShader->setParameter("time", data->globalTimer / 60.0f);
	}

	// Clear screen
	data->front->clear();

	// Draw background
	m_spriteBackground.draw(data->front);
	backgroundAnimation.draw();

	// Draw timer
	if (currentGameStatus == GAMESTATUS_IDLE && !settings->useCPUplayers && (countActivePlayers() > 1 || settings->rankedMatch))
	{
		timerSprite[0].draw(data->front);
		timerSprite[1].draw(data->front);
	}
	if (players[0]->getPlayerType() == HUMAN && players[0]->currentphase == PICKCOLORS
		&& !settings->useCPUplayers && !players[0]->pickedColor)
	{
		timerSprite[0].draw(data->front);
		timerSprite[1].draw(data->front);
	}

	// Draw player related objects
	for (const auto& player : players)
	{
		// Draw fields
		player->draw();
	}
	for (const auto& player : players)
	{
		// Draw light effect
		player->drawEffect();
	}
	if (players.size() > 0)
	{
		// Needs at least 1 player to drawy ready-go
		readyGoObj.draw();
	}

	// Draw menuSelects
	if (menuSelect == 1)
	{
		charSelectMenu->draw();
	}
	if (menuSelect == 2)
	{
		mainMenu->draw();
	}

	// Darken screen
	if (rankedState >= 3)
	{
		black.draw(data->front);
		// Draw status text
		if (statusText)
		{
			data->front->setColor(255, 255, 255, 255);
			statusText->draw(8, 0);
		}
	}

	data->front->swapBuffers();
}

void game::setStatusText(const char* utf8)
{
	if (utf8 == lastText) return;
	if (!statusFont) return;
	delete statusText;

	statusText = statusFont->render(utf8);
	lastText = utf8;
}

void game::setWindowFocus(bool focus) const
{
	data->windowFocus = focus;
}

// Main loop of the game
void game::Loop()
{
	initGame(data->front);

	sprite debugSprite;
	debugSprite.setImage(data->imgNextPuyoBackgroundL);

	// Frame limiter variables
	double t = 0.0;
	const double dt = 1.0 / 60.0;
	double accumulator = 0.0;
	double deltaTime;
	uint64_t startTime = timeGetTime();
	uint64_t endTime;

	// FPS counter
	double second = 0;
	int fps = 0;
	int fpscounter = 0;

	// Main loop
	while (runGame)
	{
		// Frame limiter
		endTime = timeGetTime();
		deltaTime = (endTime - startTime) / 1000.0; // Convert to second
		startTime = endTime;
		accumulator += deltaTime;

		// FPS counter (screen refresh rate)
		second += deltaTime;
		if (second > 1)
		{
			fps = 0;
			fpscounter = 0;
			second = 0;
		}

		// Main gameplay loop
		while (accumulator >= dt)
		{
			// Inner game loop
			playGame();

			// End of loop
			// Update framelimiter variables
			t += dt;
			accumulator -= dt;
			// FPS
			fpscounter++;

			// Draw frame in the last loop
			if (accumulator < dt)
			{
				// Screen refresh rate
				fps++;

				// Innner loop render
				renderGame();

				// Display
				data->front->swapBuffers();
			}
		}
	}
}

void game::addPlayer(playerType type, int playernum, int totalPlayer)
{
	players.push_back(new player(type, playernum, totalPlayer, this));
}

void game::resetPlayers()
{
	// Set new nextlist seed
	if (settings->useCPUplayers)
	{
		randomSeed_NextList = getRandom(10000);
	}
	else
	{
		size_t lowest = players[0]->onlineID;
		int id = 0;

		// Find lowest ID
		for (size_t i = 0; i < players.size(); i++)
		{
			if (players[i]->active && players[i]->onlineID < lowest)
			{
				lowest = players[i]->onlineID;
				id = static_cast<int>(i);
			}
		}

		// Set random seed to proposed random seed
		if (settings->recording != PVS_REPLAYING)
		{
			randomSeed_NextList = players[id]->proposedRandomSeed;
		}
	}

	// Reset players
	winsString = "";
	for (const auto& player : players)
	{
		player->reset();
		winsString += to_string(player->wins) + "-";
	}
	if (winsString.length() > 0)
		winsString = winsString.substr(0, winsString.length() - 1);
	activeAtStart = countActivePlayers();

	menuSelect = 0;
	timerEndMatch = 0;
	loadMusic();
}

// Check if match has ended.
void game::checkEnd()
{
	// Wait a little bit before actually goint into the menu, use timerEndMatch for that

	if (menuSelect == 0)
	{
		// In match
		for (const auto& player : players)
		{
			// Check if all players are in win or lose state
			if (player->losewin == NOWIN && player->active)
			{
				// Still playing
				return;
			}
		}
		if (data->windowFocus)
		{
			data->front->musicEvent(music_can_stop);
		}
		targetVolumeNormal = 25;
		targetVolumeFever = 25;
		if (currentGameStatus == GAMESTATUS_PLAYING || settings->useCPUplayers)
		{
			timerEndMatch++;
			if (timerEndMatch == 120)
			{
				// Go to menu
				menuSelect = 2;
				if (settings->rankedMatch) {
					choiceTimer = 5 * 60;
				}
				else {
					choiceTimer = 30 * 60;
				}
				colorTimer = 10 * 60;
				rankedTimer = 15 * 60;

				// Online game: reset state if you were a player
				if (connected)
				{
					currentGameStatus = GAMESTATUS_IDLE;
				}

				// Release everyone's buttons (otherwise online players start choosing stuff)
				for (const auto& player : players)
				{
					player->controls.release();
				}

				// Change channel description
				if (checkLowestID())
					sendDescription();
			}
		}
		else if (settings->spectating)
		{
			// Just wait until next match starts
			menuSelect = -1;
			if (connected)
			{
				currentGameStatus = GAMESTATUS_REMATCHING;
			}

			// Play next replay
			if (!settings->replayPlayList.empty())
			{
				replayTimer = 3 * 60;
			}
		}
	}
}

int game::countActivePlayers() const
{
	int n = 0;
	for (const auto& player : players)
	{
		if (player->active)
		{
			n++;
		}
	}
	return n;
}
int game::countBoundPlayers() const
{
	int n = 0;
	for (const auto& player : players)
	{
		if (!player->onlineName.empty())
		{
			n++;
		}
	}
	return n;
}

bool game::checkLowestID() const
{
	// Check if active player has the lowest ID
	if (!connected)
		return false;

	// Only player
	bool returnval = false;
	const PVS_Channel* ch = network->channelManager.getChannel(channelName);
	if (ch != nullptr && network->channelManager.getStatus(channelName, network->getPVS_Peer()) == 1)
	{
		// Active
		if (currentGameStatus != GAMESTATUS_WAITING && currentGameStatus != GAMESTATUS_SPECTATING)
		{
			// Lowest id?
			for (size_t i = 1; i < players.size(); i++)
			{
				if (players[i]->active && players[i]->onlineID > players[0]->onlineID)
					returnval = false;
				break;
			}
		}
	}

	return returnval;
}

void game::sendDescription() const
{
	if (!connected)
	{
		return;
	}

	// Player?
	const PVS_Channel* ch = network->channelManager.getChannel(channelName);
	if (ch != nullptr && network->channelManager.getStatus(channelName, network->getPVS_Peer()) == 1)
	{
		if (!settings->rankedMatch)
		{
			// Send message about game
			std::string str = "type:friendly|"; // Temp

			std::string ruleString = "rules:Tsu";
			const rulesetInfo_t* rs = &(settings->rulesetInfo);
			if (rs->rulesetType == TSU_ONLINE)
				ruleString = "rules:Tsu";
			else if (rs->rulesetType == FEVER_ONLINE)
				ruleString = "rules:Fever";
			else if (rs->rulesetType == FEVER15_ONLINE)
				ruleString = "rules:Fever(15th)";
			else if (rs->rulesetType == ENDLESSFEVERVS_ONLINE)
				ruleString = "rules:EndlessFeverVS";
			str += ruleString;

			char other[1000];
			sprintf(other, "|marginTime:%i|targetPoint:%i|requiredChain:%i|initialFeverCount:%i|feverPower:%i|puyoToClear:%i|quickDrop:%i|Nplayers:%i|colors:%i"
				, rs->marginTime, rs->targetPoint, rs->requiredChain, rs->initialFeverCount, rs->feverPower, rs->puyoToClear, (int)rs->quickDrop, rs->Nplayers, rs->colors);
			str += other;
			str += "|current:" + to_string(countBoundPlayers());
			if (rs->custom)
				str += "|custom:1";
			else
				str += "|custom:0";
			str += "|channelname:" + channelName;
			// Add all peers
			std::string scr;
			for (size_t i = 0; i < players.size(); i++)
			{
				if (players[i]->onlineName != "")
					scr += players[i]->onlineName + " - " + to_string(players[i]->wins) + "\n";
			}
			str += "|currentscore:" + scr;
			network->requestChannelDescription(channelName, str);
		}
	}
}

std::string game::sendUpdate() const
{
	// 0[spectate]1[currentphase]2[fieldstringnormal]3[fevermode]4[fieldfever]5[fevercount]
	// 6[rng seed]7[fever rng called]8[turns]9[colors]
	// 10[margintimer]11[chain]12[currentFeverChainAmount]13[normal GQ]14[fever GQ]
	// 15[predictedchain]16[allclear]
	std::string str = "spectate|";
	player* pl = players[0];
	str += to_string(static_cast<int>(pl->currentphase)) + "|";

	// Get normal field
	str += pl->getNormalField()->getFieldString() + " |";
	str += to_string(pl->feverMode) + "|";
	str += pl->getFeverField()->getFieldString() + " |";
	str += to_string(pl->feverGauge.getCount()) + "|";
	str += to_string(randomSeed_NextList) + "|";
	str += to_string(pl->calledRandomFeverChain) + "|";
	str += to_string(pl->turns) + "|";
	str += to_string(pl->colors) + "|";
	str += to_string(pl->margintimer) + "|";
	str += to_string(pl->chain) + "|";
	str += to_string(pl->currentFeverChainAmount) + "|";
	str += to_string(pl->normalGarbage.GQ) + "|";
	str += to_string(pl->predictedChain) + "|";
	str += to_string(pl->allClear) + "|";

	return str;
}

void game::saveReplay() const
{
	if (settings->recording != PVS_RECORDING)
	{
		return;
	}

	if (activeAtStart == 0)
	{
		return;
	}

	time_t rawtime;
	tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	// Open file
	std::ofstream outfile;
	std::string replayfolder = "User/Replays/";

	// Create userfolder
	if (!settings->spectating)
	{
		if (players.size() > 2)
		{
			replayfolder += to_string(players.size()) + "p/";
		}
		else
		{
			replayfolder += to_string(players[1]->previousName) + "/";
		}
	}
	else
	{
		replayfolder += "Spectator/";
	}
	createFolder(replayfolder);

	// Create datefolder
	char dt[20];
	strftime(dt, 20, "%Y-%m-%d", timeinfo);
	replayfolder += std::string(dt) + "/";
	createFolder(replayfolder);

	// Rulesname
	std::string rulesname = "TSU_";
	if (settings->rulesetInfo.rulesetType == FEVER_ONLINE)
		rulesname = "FEVER_";
	else if (settings->rulesetInfo.rulesetType == FEVER15_ONLINE)
		rulesname = "FEVER15_";
	else if (settings->rulesetInfo.rulesetType == ENDLESSFEVERVS_ONLINE)
		rulesname = "ENDLESSFEVER_";
	std::string playersname = "_";
	if (players.size() > 2)
		playersname = std::string("(") + to_string(activeAtStart) + "p)_";
	else if (players.size() == 2)
	{
		playersname = to_string(players[0]->previousName) + "_vs_" + to_string(players[1]->previousName) + "_";
	}
	char ft[20];
	strftime(ft, 20, "(%H%M%S)_", timeinfo);
	std::string scorename;
	if (players.size() == 2)
	{
		scorename = winsString;
	}
	std::string filename = rulesname + ft + playersname + scorename;
	outfile.open((replayfolder + filename + ".rvs").c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

	// Construct header
	replay_header rh =
	{
		{'R','P','V','S'},
		PVS_REPLAYVERSION,
		"", // Date
		"", // Time
		data->matchTimer,
		static_cast<char>(activeAtStart),
		randomSeed_NextList
	};
	strftime(rh.date, 11, "%Y/%m/%d", timeinfo);
	strftime(rh.time, 9, "%H:%M:%S", timeinfo);

	outfile.write(reinterpret_cast<char*>(&rh), sizeof(replay_header));

	replay_ruleset_header rrh =
	{
		settings->rulesetInfo.rulesetType,
		settings->rulesetInfo.marginTime,
		settings->rulesetInfo.targetPoint,
		settings->rulesetInfo.requiredChain,
		settings->rulesetInfo.initialFeverCount,
		settings->rulesetInfo.feverPower,
		settings->rulesetInfo.puyoToClear,
		settings->rulesetInfo.Nplayers,
		settings->rulesetInfo.quickDrop,
		{0,0,0,0,0,0,0,0,0,0}
	};

	outfile.write(reinterpret_cast<char*>(&rrh), sizeof(replay_ruleset_header));

	// Go through all players
	for (auto& player : players)
	{
		// Player header
		int n = player->getPlayerNum();

		replay_player_header rph =
		{
			"", // Name
			static_cast<short>(player->wins),
			static_cast<int>(player->onlineID),
			static_cast<char>(n),
			player->getPlayerType(),
			player->getCharacter(),
			static_cast<char>(player->activeAtStart),
			static_cast<char>(player->colors),
			static_cast<int>(player->controls.recordEvents.size() * sizeof(controllerEvent)),
			static_cast<int>(player->recordMessages.size() * sizeof(messageEvent)),
			0,
			0
		};

		// Copy name
		std::string name = player->previousName.substr(0, 31);
		strcpy(rph.name, name.c_str());

		// Compress move vector
		int movLength = static_cast<int>(player->controls.recordEvents.size() * sizeof(controllerEvent));
		auto movComprLength = static_cast<unsigned long>(static_cast<float>(movLength) * 1.1f + 12);
		auto* movcompressed = new unsigned char[movComprLength];
		if (movLength != 0)
		{
			int z_result = compress(
				movcompressed,
				&movComprLength,
				reinterpret_cast<unsigned char*>(&player->controls.recordEvents[0]),
				movLength);

			// Check result
			if (z_result != Z_OK)
			{
				// Stop
				outfile.close();
				remove((replayfolder + filename + ".rvs").c_str());
				return;
			}
			// Update size
			rph.vectorsizecomp_movement = static_cast<int>(movComprLength);

			encode("pvs2424", reinterpret_cast<char*>(movcompressed), movComprLength);
		}
		else
			movComprLength = 0;

		// Compress messages
		int mesLength = static_cast<int>(player->recordMessages.size() * sizeof(messageEvent));
		auto mesComprLength = static_cast<unsigned long>((static_cast<float>(mesLength) * 1.1f) + 12);
		auto* mescompressed = new unsigned char[mesComprLength];
		if (mesLength != 0)
		{
			int z_result = compress(
				mescompressed,
				&mesComprLength,
				reinterpret_cast<unsigned char*>(&player->recordMessages[0]),
				mesLength);

			// Check result
			if (z_result != Z_OK)
			{
				// Stop
				outfile.close();
				remove((replayfolder + filename + ".rvs").c_str());
				return;
			}
			// Update size
			rph.vectorsizecomp_message = static_cast<int>(mesComprLength);
			encode("pvs2424", reinterpret_cast<char*>(mescompressed), mesComprLength);
		}
		else
			mesComprLength = 0;

		// Write compressed
		outfile.write(reinterpret_cast<char*>(&rph), sizeof(replay_player_header));
		if (movLength != 0)
			outfile.write(reinterpret_cast<char*>(movcompressed), movComprLength);
		if (mesLength != 0)
			outfile.write(reinterpret_cast<char*>(mescompressed), mesComprLength);

		// Remove compressed
		delete[] movcompressed;
		player->controls.recordEvents.clear();
		// Remove compressed
		delete[] mescompressed;
		player->recordMessages.clear();
	}
	outfile.close();
}

void game::loadReplay(std::string replayfile)
{
	settings->recording = PVS_REPLAYING;
	connected = false;
	network = nullptr;

	std::ifstream infile;
	infile.open(replayfile.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!infile.is_open())
	{
		return;
	}

	// Read header
	replay_header rh;
	infile.read(reinterpret_cast<char*>(&rh), sizeof(replay_header));

	// Check version
	if (PVS_REPLAYVERSION < rh.versionNumber)
	{
		return;
	}

	currentReplayVersion = rh.versionNumber;

	// Backwards compatibility for randomizer: version 3 introduces the pp2 randomizer
	if (currentReplayVersion < 3) {
		legacyRandomizer = true;
		legacyNuisanceDrop = true;
	}
	else {
		legacyRandomizer = false;
		legacyNuisanceDrop = false;
	}

	// Set
	randomSeed_NextList = rh.randomseed;

	// Read rules header
	replay_ruleset_header rrh;
	infile.seekg(sizeof(replay_header), std::ios::beg);
	infile.read(reinterpret_cast<char*>(&rrh), sizeof(replay_ruleset_header));

	settings->rulesetInfo.quickDrop = rrh.quickDrop;
	settings->rulesetInfo.rulesetType = rrh.rulesetType;
	settings->rulesetInfo.marginTime = rrh.marginTime;
	settings->rulesetInfo.targetPoint = rrh.targetPoint;
	settings->rulesetInfo.requiredChain = rrh.requiredChain;
	settings->rulesetInfo.initialFeverCount = rrh.initialFeverCount;
	settings->rulesetInfo.feverPower = rrh.feverPower;
	settings->rulesetInfo.puyoToClear = rrh.puyoToClear;
	settings->rulesetInfo.Nplayers = rrh.Nplayers;
	settings->Nplayers = rrh.Nplayers;

	// Set rules
	setRules();

	// Set up players
	if (static_cast<int>(players.size()) != rrh.Nplayers)
	{
		while (!players.empty())
		{
			delete players.back();
			players.pop_back();
		}
		initPlayers();
	}

	// By now the number of players should be correct
	if (rrh.Nplayers != static_cast<int>(players.size()))
		return;

	unsigned int sizePrevious = sizeof(replay_header) + sizeof(replay_ruleset_header);
	for (const auto& player : players)
	{
		// Read player header
		replay_player_header rph;
		infile.seekg(sizePrevious, std::ios::beg);
		infile.read((char*)&rph, sizeof(replay_player_header));

		// Update player
		player->bindPlayer(rph.name, rph.onlineID, true);
		player->wins = rph.currentwins;
		player->setPlayerType(rph.playertype);
		player->setCharacter(rph.character);
		player->active = rph.active;
		player->colors = rph.colors;
		player->controls.recordEvents.clear();
		player->recordMessages.clear();

		// Size of vectors
		unsigned long movSize = rph.vectorsize_movement;
		unsigned long mesSize = rph.vectorsize_message;
		const unsigned long movSizeComp = rph.vectorsizecomp_movement;
		const unsigned long mesSizeComp = rph.vectorsizecomp_message;

		// Prepare vec
		if (movSize > 0)
		{
			player->controls.recordEvents.resize(movSize / sizeof(controllerEvent));
		}
		if (mesSize > 0)
		{
			player->recordMessages.resize(mesSize / sizeof(messageEvent));
		}

		// Read compressed data
		const auto movcompressed = new unsigned char[movSizeComp];
		const auto mescompressed = new unsigned char[mesSizeComp];
		if (movSize > 0)
		{
			infile.seekg(sizePrevious + sizeof(replay_player_header), std::ios::beg);
			infile.read(reinterpret_cast<char*>(movcompressed), movSizeComp);
			encode("pvs2424", reinterpret_cast<char*>(movcompressed), movSizeComp);

		}
		if (mesSize > 0)
		{
			infile.seekg(sizePrevious + sizeof(replay_player_header) + movSizeComp, std::ios::beg);
			infile.read(reinterpret_cast<char*>(mescompressed), mesSizeComp);
			encode("pvs2424", reinterpret_cast<char*>(mescompressed), mesSizeComp);
		}

		// Decompress into vectors
		if (movSize > 0)
		{
			uncompress(
				reinterpret_cast<unsigned char*>(&player->controls.recordEvents[0]),
				&movSize,
				movcompressed, // Source buffer - the compressed data
				movSizeComp); // Length of compressed data in bytes
		}
		if (mesSize > 0)
		{
			uncompress(
				reinterpret_cast<unsigned char*>(&player->recordMessages[0]),
				&mesSize,
				mescompressed, // Source buffer - the compressed data
				mesSizeComp); // Length of compressed data in bytes
		}

		// Delete temporary buffer
		delete[] movcompressed;
		delete[] mescompressed;

		// Update size
		sizePrevious += sizeof(replay_player_header) + movSizeComp + mesSizeComp;
	}
	infile.close();
}

void game::nextReplay()
{
	if (settings->recording != PVS_REPLAYING)
	{
		return;
	}

	if (!playNext)
	{
		settings->oldReplayPlayList.push_back(settings->replayPlayList.front());
		settings->replayPlayList.pop_front();
		playNext = true;
	}

	if (!settings->replayPlayList.empty())
	{
		loadReplay(settings->replayPlayList.front());
		settings->oldReplayPlayList.push_back(settings->replayPlayList.front());
		settings->replayPlayList.pop_front();
		// Get ready to play
		resetPlayers();
	}
	else
	{
		return;
	}

	replayTimer = 0;
}

void game::previousReplay()
{
	if (settings->recording != PVS_REPLAYING)
	{
		return;
	}

	if (playNext)
	{
		settings->replayPlayList.push_front(settings->oldReplayPlayList.back());
		settings->oldReplayPlayList.pop_back();
		playNext = false;
	}

	if (!settings->oldReplayPlayList.empty())
	{
		settings->replayPlayList.push_front(settings->oldReplayPlayList.back());
		loadReplay(settings->replayPlayList.front());
		settings->oldReplayPlayList.pop_back();
		// Get ready to play
		resetPlayers();
	}
	else
	{
		return;
	}

	replayTimer = 0;
}

void game::rankedMatch()
{
	if (currentGameStatus != GAMESTATUS_IDLE && currentGameStatus != GAMESTATUS_WAITING || !settings->rankedMatch)
	{
		return;
	}

	if (rankedTimer > 0)
	{
		rankedTimer--;
	}

	if (rankedTimer == 0)
	{
		// The client is expected to have applied you for ranked match
		if (rankedState == 0 && channelName.empty())
		{
			network->sendToServer(CHANNEL_MATCH, settings->rulesetInfo.rulesetType == TSU_ONLINE ? "find|0" : "find|1");
			// Upon matching again, the timer is reduced
			rankedTimer = 5 * 60;
		}
		if (rankedState == 2) // Getting ready to close match
		{
			for (const auto& player : players)
			{
				player->unbindPlayer();
			}
			rankedState = 3;
			rankedTimer = 15 * 60;
		}
		else
			rankedTimer = 15 * 60;
	}
	// Wait for player input
	if (rankedState == 3)
	{
		setStatusText(translatableStrings.rankedWaiting.c_str());
		if (!players.empty() && players[0]->controls.Start == 1)
		{
			if (!newRankedMatchMessage.empty())
			{
				// Apply for new match
				network->sendToServer(CHANNEL_MATCH, newRankedMatchMessage.c_str());
				newRankedMatchMessage = "";
			}
			rankedState = 0;
			rankedTimer = 15 * 60;
		}
	}
}

void game::changeMusicVolume()
{
	if (!data->windowFocus)
	{
		return;
	}
	if (currentVolumeNormal > targetVolumeNormal)
	{
		currentVolumeNormal -= 1;
		if (currentVolumeNormal < 0)
			currentVolumeNormal = 0;
		data->front->musicVolume(currentVolumeNormal / 100.0f * globalVolume, false);
	}
	else if (currentVolumeNormal < targetVolumeNormal && data->globalTimer % 3 == 0)
	{
		currentVolumeNormal += 1;
		if (currentVolumeNormal > 100)
			currentVolumeNormal = 100;
		data->front->musicVolume(currentVolumeNormal / 100.0f * globalVolume, false);
	}
	if (currentVolumeFever > targetVolumeFever)
	{
		currentVolumeFever -= 1;
		if (currentVolumeFever < 0)
			currentVolumeFever = 0;
		data->front->musicVolume(currentVolumeFever / 100.0f * globalVolume, true);
	}
	else if (currentVolumeFever < targetVolumeFever)
	{
		currentVolumeFever += 1;
		if (currentVolumeFever > 100)
			currentVolumeFever = 100;
		data->front->musicVolume(currentVolumeFever / 100.0f * globalVolume, true);
	}
}

void game::loadMusic()
{
	if (settings->recording == PVS_REPLAYING && backwardsOnce == true)
	{
		return;
	}

	data->front->musicEvent(music_continue);
	targetVolumeNormal = 100;
	targetVolumeFever = 100;

	// Force to set volume
	currentVolumeNormal -= 1;
	currentVolumeFever -= 1;
}

}
