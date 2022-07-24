#include <algorithm>
#include "RuleSet.h"
#include "../Player.h"
#include "../Game.h"

using namespace std;

namespace ppvs
{

// Lists
int linkBonus_TSU[11] = { 0,0,0,0,2,3,4,5,6,7,10 };
int colorBonus_TSU[5] = { 0,3,6,12,24 };
int chainBonus_TSU[22] = { 0,8,16,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512,544,576,608 };

int linkBonus_FEVER[11] = { 0,0,0,0,1,2,3,4,5,6,8 };
int colorBonus_FEVER[5] = { 0,2,4,8,16 };

// Character powers
int chainBonus_NORMAL[24][24] = {
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,12,24,32,48,96 ,160,240,320,400,500,600,700,800,900,999,999,999,999,999,999,999,999,999},
{4,12,24,33,50,101,169,254,341,428,538,648,763,876,990,999,999,999,999,999,999,999,999,999},
{4,13,25,33,48,96 ,158,235,310,384,475,564,644,728,810,890,968,999,999,999,999,999,999,999},
{4,12,25,35,52,106,179,269,362,456,575,696,826,952,999,999,999,999,999,999,999,999,999,999},
{4,12,25,32,49,96 ,160,241,320,400,501,600,700,800,901,999,999,999,999,999,999,999,999,999},
{4,12,25,33,52,106,179,275,372,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,11,21,28,41,82 ,135,202,267,332,413,492,567,644,720,795,869,936,999,999,999,999,999,999},
{4,11,23,33,51,110,188,288,392,500,638,780,945,999,999,999,999,999,999,999,999,999,999,999},
{4,12,24,32,48,96 ,160,240,320,400,500,600,700,800,900,999,999,999,999,999,999,999,999,999},
{4,13,25,33,49,96 ,159,235,310,385,475,565,645,729,810,890,969,999,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999},
{4,11,22,30,45,91 ,153,230,309,388,488,588,693,796,900,999,999,999,999,999,999,999,999,999},
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,12,25,34,52,106,178,269,362,456,575,696,826,952,999,999,999,999,999,999,999,999,999,999},
{4,12,22,31,45,92 ,153,231,309,389,489,589,693,796,901,999,999,999,999,999,999,999,999,999},
{4,12,22,29,43,86 ,145,216,289,360,451,540,630,720,810,901,991,999,999,999,999,999,999,999},
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,11,23,33,51,101,167,250,331,412,513,612,766,966,999,999,999,999,999,999,999,999,999,999},
{4,11,23,33,51,110,188,288,392,500,638,780,945,999,999,999,999,999,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999},
{4,11,21,28,41,82 ,135,202,267,332,413,492,567,644,720,795,869,936,999,999,999,999,999,999},
{4,12,23,31,46,92 ,152,229,305,380,476,570,665,760,855,898,935,969,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999} };
int chainBonus_FEVER[24][24] = {
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,22,30,48,80,120,160,240,280,288,342,400,440,480,520,560,600,640,680,720,760,800},
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,21,29,46,76,113,150,223,259,266,313,364,398,432,468,504,540,576,612,648,684,720},
{4,9 ,15,19,25,39,65,96 ,129,192,225,231,275,320,352,385,416,449,481,512,545,576,609,640},
{4,10,19,22,31,49,80,121,160,241,280,289,342,400,440,481,520,560,600,640,680,720,760,800},
{4,9 ,16,20,28,43,72,109,145,216,252,259,309,360,396,432,469,505,540,576,612,649,685,720},
{4,11,19,24,32,50,84,125,166,247,287,294,347,404,442,480,520,560,600,640,680,720,760,800},
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,21,29,46,76,113,150,223,259,266,313,364,398,432,468,504,540,576,612,648,684,720},
{4,10,19,22,29,46,76,113,150,223,259,266,313,365,399,432,469,505,540,576,612,649,685,720},
{4,10,19,24,34,55,93,142,191,290,343,355,428,508,565,624,676,728,780,832,884,936,988,999},
{5,12,21,25,34,53,87,130,171,254,294,301,353,408,444,480,520,560,600,640,680,720,760,800},
{3,8 ,15,20,28,46,77,118,159,242,287,298,360,428,477,528,572,616,660,704,748,792,836,880},
{3,8 ,14,18,24,38,64,96 ,128,192,224,230,274,320,352,384,416,448,480,512,544,576,608,640},
{4,12,22,25,35,53,88,130,172,255,295,302,353,409,445,481,520,560,600,640,680,720,760,800},
{4,9 ,18,22,32,50,85,130,175,266,315,326,395,469,522,576,625,672,720,769,816,865,912,961},
{3,8 ,15,20,28,46,77,112,151,229,272,283,342,406,453,501,543,585,627,668,710,752,794,836},
{3,8 ,15,18,25,41,68,103,138,209,245,253,302,356,394,432,468,504,540,576,612,648,684,720},
{3,8 ,15,18,25,41,68,103,138,209,245,253,302,356,394,432,468,504,540,576,612,648,684,720},
{4,11,20,25,34,55,92,139,186,281,329,339,405,476,526,576,624,672,720,768,816,864,912,960},
{4,11,20,25,34,55,92,139,186,281,329,339,405,476,526,576,624,672,720,768,816,864,912,960},
{4,10,19,23,32,49,82,125,165,249,292,299,358,419,462,505,546,589,630,672,715,756,799,840},
{4,9 ,17,22,31,50,85,130,175,266,315,326,394,468,521,576,624,672,720,768,816,864,912,960} };

// Target point calculation
int getTargetFromMargin(int initialTarget, int marginTime, int currentTime)
{
	if (currentTime >= marginTime)
	{
		int step = (currentTime - marginTime) / (16 * 60);
		step = min(step, 13);
		int out = initialTarget;

		// Even
		if (step % 2 == 0)
		{
			out = initialTarget * 3 / 4;
			step = step / 2;
		}

		// Uneven
		else
		{
			step = (step + 1) / 2;
		}
		return max(int(out / pow(2., step)), 1);
	}
	return initialTarget;
}

//{Base rules
//-------------------------
RuleSet::RuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	addDropBonus = false;
	voicePatternFever = true;
	fastDrop = 48;
	marginTime = 0;
	initialFeverCount = 0;
	puyoToClear = 4;
	feverPower = 1;
	requiredChain = 0;
	NFeverChains = 4;
	allClearStart = true;
	feverDeath = true;
	bonusEQ = false;
	quickDrop = false;
}

RuleSet::~RuleSet()
{
}

void RuleSet::setRules(Rules rulestring)
{
	m_rules = rulestring;

	// Set constants
	switch (m_rules)
	{
	case ENDLESS:
		break;
	case TSU:
		break;
	case FEVER:
		break;
	}
}

void RuleSet::setGame(Game* g)
{
	currentgame = g;
	data = g->data;
	this->onSetGame();
}

Phase RuleSet::endPhase(Phase, Player*)
{
	return IDLE;
}

void RuleSet::onSetGame()
{
	// Use this to set legacyRandomizer
}

void RuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = 70;
}

void RuleSet::onAllClearPop(Player* thisplayer)
{
	thisplayer->allClear = 0;
}

void RuleSet::onAllClear(Player* thisplayer)
{
	thisplayer->allClear = 1;
}

void RuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;
}

void RuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;

}

void RuleSet::onChain(Player*)
{

}

void RuleSet::onOffset(Player*)
{

}

void RuleSet::onAttack(Player*)
{

}

int RuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_TSU[10];
	return linkBonus_TSU[n];
}

int RuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_TSU[4];
	return colorBonus_TSU[n];
}

int RuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonus[15] + 32 * (n - 15), 999);
	return chainBonus[n];
}
//}

//{ENDLESS
//==========================================================
EndlessRuleSet::EndlessRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
}

EndlessRuleSet::~EndlessRuleSet()
{

}


Phase EndlessRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return DROPGARBAGE;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		return DROPGARBAGE;
	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		return PREPARE;

	default:
		return IDLE;

	}
}

void EndlessRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
}

void EndlessRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->currentphase == SEARCHCHAIN)
	{
		thisplayer->currentScore += 30 * thisplayer->targetPoint;
		thisplayer->allClear = 0;
	}
}

void EndlessRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;

	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void EndlessRuleSet::onChain(Player*)
{
}

void EndlessRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}
int EndlessRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int EndlessRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int EndlessRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}

//{TSU
//==========================================================
TsuRuleSet::TsuRuleSet()
{
	delayedFall = false;
	doubleSpawn = false;
	addDropBonus = true;
	voicePatternFever = false;
}

TsuRuleSet::~TsuRuleSet()
{
}


Phase TsuRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return DROPGARBAGE;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		return DROPGARBAGE;
	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		return PREPARE;

	default:
		return IDLE;

	}
}

void TsuRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(false);
	thisplayer->useDropPattern = false;
}

void TsuRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->currentphase == SEARCHCHAIN)
	{
		thisplayer->currentScore += 30 * thisplayer->targetPoint;
		thisplayer->allClear = 0;
	}
}

void TsuRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound if player 1 wins or loses
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void TsuRuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;
	// Sounds for losing (not player 1)
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

int TsuRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_TSU[10];
	return linkBonus_TSU[n];
}

int TsuRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_TSU[4];
	return colorBonus_TSU[n];
}

int TsuRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonus_TSU[15] + 32 * (n - 15), 999);
	return chainBonus_TSU[n];
}
//}

//{FEVER
//==========================================================
FeverRuleSet::FeverRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
}

FeverRuleSet::~FeverRuleSet()
{

}

Phase FeverRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return DROPGARBAGE;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->feverMode)
				return CHECKENDFEVER;
			return DROPGARBAGE;
		}
	case CHECKENDFEVER:
		{
			if (thisplayer->feverGauge.seconds == 0)
				return ENDFEVER;
			if (!thisplayer->poppedChain) // No chain popped: normal progression
			{
				return DROPGARBAGE;
			}
			// Player popped a chain, drop a new feverchain
			thisplayer->poppedChain = false; // Chain must be reset here
			return DROPFEVER;
		}
	case ENDFEVER:
		return DROPGARBAGE;

	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		{
			if (!thisplayer->feverMode)
				return CHECKFEVER; // Not in fever: check if fever should start
			return PREPARE;
		}
	case CHECKFEVER:
		{
			if (thisplayer->feverMode)
				return PREPAREFEVER;
			if (thisplayer->allClear == 0)
				return PREPARE;
			thisplayer->allClear = 0;
			return DROPPUYO;
		}
	case PREPAREFEVER:
		return DROPFEVER;
	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return IDLE;
	}
}

void FeverRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void FeverRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
}

void FeverRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void FeverRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void FeverRuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;
}

void FeverRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void FeverRuleSet::onChain(Player*)
{
}

void FeverRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void FeverRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->feverMode && thisplayer->feverGauge.seconds < 60 * 30)
	{
		thisplayer->feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int FeverRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int FeverRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int FeverRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}

//{ENDLESS FEVER
//==========================================================
EndlessFeverRuleSet::EndlessFeverRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	allClearStart = false;
}

EndlessFeverRuleSet::~EndlessFeverRuleSet()
{

}

Phase EndlessFeverRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return CHECKFEVER;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->feverMode)
				return CHECKENDFEVER;
			return DROPGARBAGE;
		}
	case CHECKENDFEVER:
		{
			if (thisplayer->feverGauge.seconds == 0)
			{
				// Lose
				thisplayer->losewin = LOSE;
				return LOSEDROP;
			}
			if (!thisplayer->poppedChain) // No chain popped: normal progression
			{
				return DROPGARBAGE;
			}
			// Player popped a chain, drop a new feverchain
			thisplayer->poppedChain = false; // Chain must be reset here
			return DROPFEVER;
		}
	case ENDFEVER:
		return DROPGARBAGE;

	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		{
			if (!thisplayer->feverMode)
				return CHECKFEVER; // Not in fever: check if fever should start
			return PREPARE;
		}
	case CHECKFEVER:
		{
			if (thisplayer->feverMode)
				return PREPAREFEVER;
			if (thisplayer->allClear == 0)
				return PREPARE;
			thisplayer->allClear = 0;
			return DROPPUYO;
		}
	case PREPAREFEVER:
		return DROPFEVER;
	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return IDLE;
	}
}

void EndlessFeverRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void EndlessFeverRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
	thisplayer->feverGauge.setCount(7);
	thisplayer->feverGauge.setSeconds(60 * 60);
	thisplayer->feverGauge.maxSeconds = 99;
	thisplayer->nextPuyoActive = false;
}

void EndlessFeverRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void EndlessFeverRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void EndlessFeverRuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;
}

void EndlessFeverRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void EndlessFeverRuleSet::onChain(Player*)
{
}

void EndlessFeverRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->feverMode && thisplayer->feverGauge.seconds < 60 * 30)
	{
		thisplayer->feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int EndlessFeverRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int EndlessFeverRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}

//{ENDLESS FEVER VS
//==========================================================
EndlessFeverVsRuleSet::EndlessFeverVsRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
}

EndlessFeverVsRuleSet::~EndlessFeverVsRuleSet()
{

}

Phase EndlessFeverVsRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return CHECKFEVER;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->feverMode)
				return CHECKENDFEVER;
			return DROPGARBAGE;
		}
	case CHECKENDFEVER:
		{
			if (thisplayer->feverGauge.seconds == 0)
				return ENDFEVER;
			if (!thisplayer->poppedChain) // No chain popped: normal progression
				return DROPGARBAGE;
			// Player popped a chain, drop a new feverchain
			thisplayer->poppedChain = false; // Chain must be reset here
			return DROPFEVER;
		}
	case ENDFEVER:
		return DROPGARBAGE;

	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		{
			if (!thisplayer->feverMode)
				return CHECKFEVER; // Not in fever: check if fever should start
			return PREPARE;
		}
	case CHECKFEVER:
		{
			if (thisplayer->feverMode)
				return PREPAREFEVER;
			if (thisplayer->allClear == 0)
				return PREPARE;
			thisplayer->allClear = 0;
			return DROPPUYO;
		}
	case PREPAREFEVER:
		return DROPFEVER;
	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return IDLE;
	}
}

void EndlessFeverVsRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void EndlessFeverVsRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
	thisplayer->feverGauge.setCount(7);
	thisplayer->feverGauge.setSeconds(99 * 60);
	thisplayer->feverGauge.endless = true;
	thisplayer->normalGarbage.GQ = 270;
	thisplayer->updateTray();
	thisplayer->nextPuyoActive = false;
}

void EndlessFeverVsRuleSet::onAllClearPop(Player*)
{
}

void EndlessFeverVsRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void EndlessFeverVsRuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;
}

void EndlessFeverVsRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void EndlessFeverVsRuleSet::onChain(Player*)
{
}

void EndlessFeverVsRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverVsRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->feverMode && thisplayer->feverGauge.seconds < 60 * 30)
	{
		thisplayer->feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverVsRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int EndlessFeverVsRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int EndlessFeverVsRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}

//{TSU_ONLINE
//==========================================================
TsuOnlineRuleSet::TsuOnlineRuleSet()
{
	delayedFall = false;
	doubleSpawn = false;
	addDropBonus = true;
	voicePatternFever = false;
}

TsuOnlineRuleSet::~TsuOnlineRuleSet()
{

}


Phase TsuOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}
	case WAITGARBAGE:
		return FALLGARBAGE;
	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN)
			{
				return WAITCONFIRMGARBAGE;
			}
			return CHECKLOSER;
		}
	case WAITCONFIRMGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		return PREPARE;

	default:
		return IDLE;

	}
}

void TsuOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(false);
	thisplayer->useDropPattern = false;
}

void TsuOnlineRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->currentphase == SEARCHCHAIN)
	{
		thisplayer->currentScore += 30 * thisplayer->targetPoint;
		thisplayer->allClear = 0;
	}
}

void TsuOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound if player 1 wins or loses
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void TsuOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->losewin = LOSEWAIT;
	else // No need to wait for anything
		thisplayer->losewin = LOSE;

	thisplayer->currentphase = LOSEDROP;
	// Sounds for losing (not player 1)
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

int TsuOnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_TSU[10];
	return linkBonus_TSU[n];
}

int TsuOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_TSU[4];
	return colorBonus_TSU[n];
}

int TsuOnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonus_TSU[15] + 32 * (n - 15), 999);
	return chainBonus_TSU[n];
}

void TsuOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = false;
}

//}

//{FEVER ONLINE
//==========================================================
FeverOnlineRuleSet::FeverOnlineRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	allClearStart = false;
	feverDeath = false;
	bonusEQ = true;
}

FeverOnlineRuleSet::~FeverOnlineRuleSet()
{

}

Phase FeverOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->feverMode)
			{
				if (thisplayer->getPlayerType() == ONLINE)
					return CHECKENDFEVERONLINE;
				return CHECKENDFEVER;
			}
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}
	case WAITGARBAGE:
		return FALLGARBAGE;
	case CHECKENDFEVERONLINE:
		return CHECKENDFEVER;
	case CHECKENDFEVER:
		{
			if (thisplayer->feverEnd)
			{
				return ENDFEVER;
			}
			if (!thisplayer->poppedChain) // No chain popped: normal progression
			{
				if (thisplayer->getPlayerType() == ONLINE)
					return WAITGARBAGE;
				return DROPGARBAGE;
			}
			// Player popped a chain, drop a new feverchain
			thisplayer->poppedChain = false; // Chain must be reset here
			return DROPFEVER;
		}
	case ENDFEVER:
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}

	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN)
			{
				return WAITCONFIRMGARBAGE;
			}
			return CHECKLOSER;
		}
	case WAITCONFIRMGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		{
			if (!thisplayer->feverMode)
				return CHECKFEVER; // Not in fever: check if fever should start
			return PREPARE;
		}
	case CHECKFEVER:
		{
			if (thisplayer->feverMode)
				return PREPAREFEVER;
			if (thisplayer->allClear == 0)
				return PREPARE;
			thisplayer->allClear = 0;
			return DROPPUYO;
		}
	case PREPAREFEVER:
		return DROPFEVER;
	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode
	case WAITLOSE:
		return WAITLOSE;

	default:
		return IDLE;
	}
}

void FeverOnlineRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void FeverOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
}

void FeverOnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void FeverOnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void FeverOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->losewin = LOSEWAIT;
	else // No need to wait for anything
		thisplayer->losewin = LOSE;

	thisplayer->currentphase = LOSEDROP;
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

void FeverOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void FeverOnlineRuleSet::onChain(Player*)
{
}

void FeverOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
	// Add a second to last attacker
	if (thisplayer->lastAttacker == nullptr)
		return;

	if (!thisplayer->lastAttacker->feverMode && thisplayer->lastAttacker->feverGauge.seconds < 60 * 30)
	{
		thisplayer->lastAttacker->feverGauge.addTime(60);
		thisplayer->lastAttacker->showSecondsObj(60);
	}
}

void FeverOnlineRuleSet::onAttack(Player*)
{

}

int FeverOnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int FeverOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int FeverOnlineRuleSet::getChainBonus(Player* pl)
{
	const int character = pl->getCharacter();
	int n = pl->chain;

	n--;

	if (n < 0)
		return 0;

	if (n == 0)
		return 0; // First value is 0 in PP20th

	if (n > 23)
		return 999;

	if (pl->feverMode)
		return (chainBonus_FEVER[character][n] * 699) / 999;

	return (chainBonus_NORMAL[character][n] * 699) / 999;
}
//}

//{FEVER15 ONLINE
//==========================================================
Fever15OnlineRuleSet::Fever15OnlineRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	allClearStart = false;
	NFeverChains = 12;
}

Fever15OnlineRuleSet::~Fever15OnlineRuleSet()
{

}

Phase Fever15OnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		if (thisplayer->getPlayerType() == ONLINE)
		{
			return WAITGARBAGE;
		}
		return DROPGARBAGE;

	case PREPARE:
		return MOVE;

	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;

	case CREATEPUYO:
		return DROPPUYO;

	case DROPPUYO:
		return FALLPUYO;

	case FALLPUYO:
		return SEARCHCHAIN;

	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		if (!thisplayer->foundChain)
			return CHECKALLCLEAR;

		thisplayer->destroyPuyosTimer = 1;
		return DESTROYPUYO;

	case DESTROYPUYO:
		return GARBAGE;

	case GARBAGE:
		return DROPPUYO;

	case CHECKALLCLEAR:
		if (thisplayer->feverMode)
		{
			if (thisplayer->getPlayerType() == ONLINE)
			{
				return CHECKENDFEVERONLINE;
			}

			return CHECKENDFEVER;
		}

		if (thisplayer->getPlayerType() == ONLINE)
		{
			return WAITGARBAGE;
		}

		return DROPGARBAGE;

	case WAITGARBAGE:
		return FALLGARBAGE;

	case CHECKENDFEVERONLINE:
		return CHECKENDFEVER;

	case CHECKENDFEVER:
		if (thisplayer->feverEnd)
		{
			return ENDFEVER;
		}
		if (!thisplayer->poppedChain) // No chain popped: normal progression
		{
			if (thisplayer->getPlayerType() == ONLINE)
			{
				return WAITGARBAGE;
			}

			return DROPGARBAGE;
		}
		thisplayer->poppedChain = false; // C3hain must be reset here
		return DROPFEVER;

	case ENDFEVER:
		if (thisplayer->getPlayerType() == ONLINE)
		{
			return WAITGARBAGE;
		}

		return DROPGARBAGE;

	case DROPGARBAGE:
		return FALLGARBAGE;

	case FALLGARBAGE:
		// Expect other players to confirm your move
		if (thisplayer->getPlayerType() == HUMAN)
		{
			return WAITCONFIRMGARBAGE;
		}

		return CHECKLOSER;

	case WAITCONFIRMGARBAGE:
		return CHECKLOSER;

	case CHECKLOSER:
		// Not in fever: check if fever should start
		if (!thisplayer->feverMode)
		{
			return CHECKFEVER;
		}
		return PREPARE;

	case CHECKFEVER:
		if (thisplayer->feverMode)
		{
			return PREPAREFEVER;
		}

		if (thisplayer->allClear == 0)
		{
			return PREPARE;
		}

		thisplayer->allClear = 0;
		return DROPPUYO;

	case PREPAREFEVER:
		return DROPFEVER;

	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	case WAITLOSE:
		return WAITLOSE;

	default:
		return IDLE;
	}
}

void Fever15OnlineRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void Fever15OnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
}

void Fever15OnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void Fever15OnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void Fever15OnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->losewin = LOSEWAIT;
	else // No need to wait for anything
		thisplayer->losewin = LOSE;

	thisplayer->currentphase = LOSEDROP;
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

void Fever15OnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void Fever15OnlineRuleSet::onChain(Player*)
{
}

void Fever15OnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void Fever15OnlineRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->feverMode && thisplayer->feverGauge.seconds < 60 * 30)
	{
		thisplayer->feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int Fever15OnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int Fever15OnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int Fever15OnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}


//{ ENDLESSFEVERVS ONLINE
//==========================================================
EndlessFeverVsOnlineRuleSet::EndlessFeverVsOnlineRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	allClearStart = false;
	NFeverChains = 12;
}

EndlessFeverVsOnlineRuleSet::~EndlessFeverVsOnlineRuleSet()
{

}

Phase EndlessFeverVsOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return CHECKFEVER;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		{
			if (thisplayer->feverMode)
			{
				if (thisplayer->getPlayerType() == ONLINE)
					return CHECKENDFEVERONLINE;
				return CHECKENDFEVER;
			}
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}
	case WAITGARBAGE:
		return FALLGARBAGE;
	case CHECKENDFEVERONLINE:
		return CHECKENDFEVER;
	case CHECKENDFEVER:
		{
			if (thisplayer->feverEnd)
			{
				return ENDFEVER;
			}
			if (!thisplayer->poppedChain) // No chain popped: normal progression
			{
				if (thisplayer->getPlayerType() == ONLINE)
					return WAITGARBAGE;
				return DROPGARBAGE;
			}
			// Player popped a chain, drop a new feverchain
			thisplayer->poppedChain = false; // Chain must be reset here
			return DROPFEVER;
		}
	case ENDFEVER:
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return WAITGARBAGE;
			return DROPGARBAGE;
		}

	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN)
			{
				return WAITCONFIRMGARBAGE;
			}
			return CHECKLOSER;
		}
	case WAITCONFIRMGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		{
			if (!thisplayer->feverMode)
			{
				return CHECKFEVER; // Not in fever: check if fever should start
			}
			return PREPARE;
		}
	case CHECKFEVER:
		{
			if (thisplayer->feverMode)
			{
				return PREPAREFEVER;
			}
			if (thisplayer->allClear == 0)
			{
				return PREPARE;
			}
			thisplayer->allClear = 0;
			return DROPPUYO;
		}
	case PREPAREFEVER:
		return DROPFEVER;
	case DROPFEVER:
		return DROPPUYO;

	case LOSEDROP:
		return LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode
	case WAITLOSE:
		return WAITLOSE;

	default:
		return IDLE;
	}
}

void EndlessFeverVsOnlineRuleSet::onSetGame()
{
	currentgame->legacyRandomizer = true;
	currentgame->legacyNuisanceDrop = true;
}

void EndlessFeverVsOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(true);
	puyoToClear = 4; // Must be 4!
	thisplayer->feverGauge.setCount(7);
	thisplayer->feverGauge.setSeconds(99 * 60);
	thisplayer->feverGauge.endless = true;
	thisplayer->normalGarbage.GQ = 270;
	thisplayer->updateTray();
	thisplayer->nextPuyoActive = false;
}

void EndlessFeverVsOnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void EndlessFeverVsOnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->allClear = 1;
}

void EndlessFeverVsOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->losewin = LOSEWAIT;
	else // No need to wait for anything
		thisplayer->losewin = LOSE;

	thisplayer->currentphase = LOSEDROP;
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

void EndlessFeverVsOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;
	// Sound
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void EndlessFeverVsOnlineRuleSet::onChain(Player*)
{
}

void EndlessFeverVsOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverVsOnlineRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->feverMode && thisplayer->feverGauge.seconds < 60 * 30)
	{
		thisplayer->feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverVsOnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_FEVER[10];
	return linkBonus_FEVER[n];
}

int EndlessFeverVsOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_FEVER[4];
	return colorBonus_FEVER[n];
}

int EndlessFeverVsOnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->feverMode)
		return chainBonus_FEVER[character][n];
	return chainBonus_NORMAL[character][n];
}
//}


//{EXCAVATION
//==========================================================
ExcavationRuleSet::ExcavationRuleSet()
{
	delayedFall = true;
	doubleSpawn = true;
	addDropBonus = true;
	voicePatternFever = true;
}

ExcavationRuleSet::~ExcavationRuleSet()
{
}

Phase ExcavationRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase)
	{
	case GETREADY:
		return DROPPUYO;
	case PREPARE:
		return MOVE;
	case MOVE:
		thisplayer->createPuyo = true;
		return CREATEPUYO;
	case CREATEPUYO:
		return DROPPUYO;
	case DROPPUYO:
		return FALLPUYO;
	case FALLPUYO:
		return SEARCHCHAIN;
	case SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->foundChain)
				return CHECKALLCLEAR;
				// Found chain
			thisplayer->destroyPuyosTimer = 1;
		}
		return DESTROYPUYO;
	case DESTROYPUYO:
		return GARBAGE;
	case GARBAGE:
		return DROPPUYO;
	case CHECKALLCLEAR:
		return DROPGARBAGE;
	case DROPGARBAGE:
		return FALLGARBAGE;
	case FALLGARBAGE:
		return CHECKLOSER;
	case CHECKLOSER:
		return PREPARE;

	default:
		return IDLE;

	}
}

void ExcavationRuleSet::onInit(Player* thisplayer)
{
	thisplayer->targetPoint = targetPoint;
	thisplayer->feverGauge.setVisible(false);
	thisplayer->useDropPattern = false;
}

void ExcavationRuleSet::onAllClearPop(Player* thisplayer)
{
}

void ExcavationRuleSet::onWin(Player* thisplayer)
{
	thisplayer->losewin = WIN;
	thisplayer->currentphase = WIN_IDLE;

	// Sound if player 1 wins or loses
	if (thisplayer == currentgame->players[0])
		data->snd.win.play(data);
	else
		data->snd.lose.play(data);
}

void ExcavationRuleSet::onLose(Player* thisplayer)
{
	thisplayer->losewin = LOSE;
	thisplayer->currentphase = LOSEDROP;

	// Sounds for losing (not player 1)
	if (thisplayer != currentgame->players[0] && currentgame->getActivePlayers() != 1)
		data->snd.lose.play(data);
}

int ExcavationRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonus_TSU[10];
	return linkBonus_TSU[n];
}

int ExcavationRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonus_TSU[4];
	return colorBonus_TSU[n];
}

int ExcavationRuleSet::getChainBonus(Player* pl)
{
	int n = pl->chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonus_TSU[15] + 32 * (n - 15), 999);
	return chainBonus_TSU[n];
}
//}

}
