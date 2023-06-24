#include "RuleSet.h"
#include "../Game.h"
#include "../Player.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace ppvs {

// Lists
int linkBonusTsu[11] = { 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10 };
int colorBonusTsu[5] = { 0, 3, 6, 12, 24 };
int chainBonusTsu[22] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608 };

int linkBonusFever[11] = { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 8 };
int colorBonusFever[5] = { 0, 2, 4, 8, 16 };

// Character powers
int chainBonusNormal[24][24] = {
	{ 4, 11, 24, 33, 51, 106, 179, 274, 371, 472, 600, 732, 882, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 24, 32, 48, 96, 160, 240, 320, 400, 500, 600, 700, 800, 900, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 24, 33, 50, 101, 169, 254, 341, 428, 538, 648, 763, 876, 990, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 13, 25, 33, 48, 96, 158, 235, 310, 384, 475, 564, 644, 728, 810, 890, 968, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 25, 35, 52, 106, 179, 269, 362, 456, 575, 696, 826, 952, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 25, 32, 49, 96, 160, 241, 320, 400, 501, 600, 700, 800, 901, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 25, 33, 52, 106, 179, 275, 372, 472, 600, 732, 882, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 21, 28, 41, 82, 135, 202, 267, 332, 413, 492, 567, 644, 720, 795, 869, 936, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 23, 33, 51, 110, 188, 288, 392, 500, 638, 780, 945, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 24, 32, 48, 96, 160, 240, 320, 400, 500, 600, 700, 800, 900, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 13, 25, 33, 49, 96, 159, 235, 310, 385, 475, 565, 645, 729, 810, 890, 969, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 22, 29, 43, 86, 144, 216, 288, 360, 450, 540, 630, 720, 810, 900, 990, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 22, 30, 45, 91, 153, 230, 309, 388, 488, 588, 693, 796, 900, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 24, 33, 51, 106, 179, 274, 371, 472, 600, 732, 882, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 25, 34, 52, 106, 178, 269, 362, 456, 575, 696, 826, 952, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 22, 31, 45, 92, 153, 231, 309, 389, 489, 589, 693, 796, 901, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 22, 29, 43, 86, 145, 216, 289, 360, 451, 540, 630, 720, 810, 901, 991, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 24, 33, 51, 106, 179, 274, 371, 472, 600, 732, 882, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 23, 33, 51, 101, 167, 250, 331, 412, 513, 612, 766, 966, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 23, 33, 51, 110, 188, 288, 392, 500, 638, 780, 945, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 22, 29, 43, 86, 144, 216, 288, 360, 450, 540, 630, 720, 810, 900, 990, 999, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 21, 28, 41, 82, 135, 202, 267, 332, 413, 492, 567, 644, 720, 795, 869, 936, 999, 999, 999, 999, 999, 999 },
	{ 4, 12, 23, 31, 46, 92, 152, 229, 305, 380, 476, 570, 665, 760, 855, 898, 935, 969, 999, 999, 999, 999, 999, 999 },
	{ 4, 11, 22, 29, 43, 86, 144, 216, 288, 360, 450, 540, 630, 720, 810, 900, 990, 999, 999, 999, 999, 999, 999, 999 }
};
int chainBonusFever[24][24] = {
	{ 4, 9, 16, 20, 27, 43, 72, 108, 144, 216, 252, 259, 308, 360, 396, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 10, 18, 22, 30, 48, 80, 120, 160, 240, 280, 288, 342, 400, 440, 480, 520, 560, 600, 640, 680, 720, 760, 800 },
	{ 4, 9, 16, 20, 27, 43, 72, 108, 144, 216, 252, 259, 308, 360, 396, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 10, 18, 21, 29, 46, 76, 113, 150, 223, 259, 266, 313, 364, 398, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 9, 15, 19, 25, 39, 65, 96, 129, 192, 225, 231, 275, 320, 352, 385, 416, 449, 481, 512, 545, 576, 609, 640 },
	{ 4, 10, 19, 22, 31, 49, 80, 121, 160, 241, 280, 289, 342, 400, 440, 481, 520, 560, 600, 640, 680, 720, 760, 800 },
	{ 4, 9, 16, 20, 28, 43, 72, 109, 145, 216, 252, 259, 309, 360, 396, 432, 469, 505, 540, 576, 612, 649, 685, 720 },
	{ 4, 11, 19, 24, 32, 50, 84, 125, 166, 247, 287, 294, 347, 404, 442, 480, 520, 560, 600, 640, 680, 720, 760, 800 },
	{ 4, 9, 16, 20, 27, 43, 72, 108, 144, 216, 252, 259, 308, 360, 396, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 10, 18, 21, 29, 46, 76, 113, 150, 223, 259, 266, 313, 364, 398, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 10, 19, 22, 29, 46, 76, 113, 150, 223, 259, 266, 313, 365, 399, 432, 469, 505, 540, 576, 612, 649, 685, 720 },
	{ 4, 10, 19, 24, 34, 55, 93, 142, 191, 290, 343, 355, 428, 508, 565, 624, 676, 728, 780, 832, 884, 936, 988, 999 },
	{ 5, 12, 21, 25, 34, 53, 87, 130, 171, 254, 294, 301, 353, 408, 444, 480, 520, 560, 600, 640, 680, 720, 760, 800 },
	{ 3, 8, 15, 20, 28, 46, 77, 118, 159, 242, 287, 298, 360, 428, 477, 528, 572, 616, 660, 704, 748, 792, 836, 880 },
	{ 3, 8, 14, 18, 24, 38, 64, 96, 128, 192, 224, 230, 274, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640 },
	{ 4, 12, 22, 25, 35, 53, 88, 130, 172, 255, 295, 302, 353, 409, 445, 481, 520, 560, 600, 640, 680, 720, 760, 800 },
	{ 4, 9, 18, 22, 32, 50, 85, 130, 175, 266, 315, 326, 395, 469, 522, 576, 625, 672, 720, 769, 816, 865, 912, 961 },
	{ 3, 8, 15, 20, 28, 46, 77, 112, 151, 229, 272, 283, 342, 406, 453, 501, 543, 585, 627, 668, 710, 752, 794, 836 },
	{ 3, 8, 15, 18, 25, 41, 68, 103, 138, 209, 245, 253, 302, 356, 394, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 3, 8, 15, 18, 25, 41, 68, 103, 138, 209, 245, 253, 302, 356, 394, 432, 468, 504, 540, 576, 612, 648, 684, 720 },
	{ 4, 11, 20, 25, 34, 55, 92, 139, 186, 281, 329, 339, 405, 476, 526, 576, 624, 672, 720, 768, 816, 864, 912, 960 },
	{ 4, 11, 20, 25, 34, 55, 92, 139, 186, 281, 329, 339, 405, 476, 526, 576, 624, 672, 720, 768, 816, 864, 912, 960 },
	{ 4, 10, 19, 23, 32, 49, 82, 125, 165, 249, 292, 299, 358, 419, 462, 505, 546, 589, 630, 672, 715, 756, 799, 840 },
	{ 4, 9, 17, 22, 31, 50, 85, 130, 175, 266, 315, 326, 394, 468, 521, 576, 624, 672, 720, 768, 816, 864, 912, 960 }
};

// Target point calculation
int getTargetFromMargin(const int initialTarget, const int marginTime, const int currentTime)
{
	if (currentTime < marginTime) {
		return initialTarget;
	}

	int step = (currentTime - marginTime) / (16 * 60);
	step = min(step, 13);
	int out = initialTarget;

	if (step % 2 == 0) {
		// Even
		out = initialTarget * 3 / 4;
		step = step / 2;
	} else {
		// Uneven
		step = (step + 1) / 2;
	}
	return max(static_cast<int>(out / pow(2., step)), 1);
}

//{Base rules
//-------------------------
RuleSet::RuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_addDropBonus = false;
	m_voicePatternFever = true;
	m_fastDrop = 48;
	m_marginTime = 0;
	m_initialFeverCount = 0;
	m_puyoToClear = 4;
	m_feverPower = 1;
	m_requiredChain = 0;
	m_nFeverChains = 4;
	m_allClearStart = true;
	m_feverDeath = true;
	m_bonusEq = false;
	m_quickDrop = false;
}

RuleSet::~RuleSet() = default;

void RuleSet::setRules(const Rules ruleString)
{
	m_rules = ruleString;
}

void RuleSet::setGame(Game* g)
{
	m_currentGame = g;
	m_data = g->m_gameRenderer->m_gameData;
	this->onSetGame();
}

Phase RuleSet::endPhase(Phase, Player*)
{
	return Phase::IDLE;
}

void RuleSet::onSetGame()
{
	// Use this to set legacyRandomizer
}

void RuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = 70;
}

void RuleSet::onAllClearPop(Player* thisplayer)
{
	thisplayer->m_allClear = 0;
}

void RuleSet::onAllClear(Player* thisplayer)
{
	thisplayer->m_allClear = 1;
}

void RuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;
}

void RuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
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
		return linkBonusTsu[10];
	return linkBonusTsu[n];
}

int RuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusTsu[4];
	return colorBonusTsu[n];
}

int RuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(m_chainBonus[15] + 32 * (n - 15), 999);
	return m_chainBonus[n];
}
//}

//{ENDLESS
//==========================================================
EndlessRuleSet::EndlessRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
}

EndlessRuleSet::~EndlessRuleSet()
{
}

Phase EndlessRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::DROPGARBAGE;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR:
		return Phase::DROPGARBAGE;
	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER:
		return Phase::PREPARE;

	default:
		return Phase::IDLE;
	}
}

void EndlessRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
}

void EndlessRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->m_currentPhase == Phase::SEARCHCHAIN) {
		thisplayer->m_currentScore += 30 * thisplayer->m_targetPoint;
		thisplayer->m_allClear = 0;
	}
}

void EndlessRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;

	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void EndlessRuleSet::onChain(Player*)
{
}

void EndlessRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}
int EndlessRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int EndlessRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int EndlessRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{TSU
//==========================================================
TsuRuleSet::TsuRuleSet()
{
	m_delayedFall = false;
	m_doubleSpawn = false;
	m_addDropBonus = true;
	m_voicePatternFever = false;
}

TsuRuleSet::~TsuRuleSet()
{
}

Phase TsuRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::DROPGARBAGE;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR:
		return Phase::DROPGARBAGE;
	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER:
		return Phase::PREPARE;

	default:
		return Phase::IDLE;
	}
}

void TsuRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(false);
	thisplayer->m_useDropPattern = false;
}

void TsuRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->m_currentPhase == Phase::SEARCHCHAIN) {
		thisplayer->m_currentScore += 30 * thisplayer->m_targetPoint;
		thisplayer->m_allClear = 0;
	}
}

void TsuRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound if player 1 wins or loses
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void TsuRuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;
	// Sounds for losing (not player 1)
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

int TsuRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusTsu[10];
	return linkBonusTsu[n];
}

int TsuRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusTsu[4];
	return colorBonusTsu[n];
}

int TsuRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonusTsu[15] + 32 * (n - 15), 999);
	return chainBonusTsu[n];
}
//}

//{FEVER
//==========================================================
FeverRuleSet::FeverRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
}

FeverRuleSet::~FeverRuleSet()
{
}

Phase FeverRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::DROPGARBAGE;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->m_feverMode)
			return Phase::CHECKENDFEVER;
		return Phase::DROPGARBAGE;
	}
	case Phase::CHECKENDFEVER: {
		if (thisplayer->m_feverGauge.m_seconds == 0)
			return Phase::ENDFEVER;
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
		{
			return Phase::DROPGARBAGE;
		}
		// Player popped a chain, drop a new feverchain
		thisplayer->m_poppedChain = false; // Chain must be reset here
		return Phase::DROPFEVER;
	}
	case Phase::ENDFEVER:
		return Phase::DROPGARBAGE;

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER: {
		if (!thisplayer->m_feverMode)
			return Phase::CHECKFEVER; // Not in fever: check if fever should start
		return Phase::PREPARE;
	}
	case Phase::CHECKFEVER: {
		if (thisplayer->m_feverMode)
			return Phase::PREPAREFEVER;
		if (thisplayer->m_allClear == 0)
			return Phase::PREPARE;
		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;
	}
	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;
	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return Phase::IDLE;
	}
}

void FeverRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void FeverRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
}

void FeverRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void FeverRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void FeverRuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;
}

void FeverRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void FeverRuleSet::onChain(Player*)
{
}

void FeverRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void FeverRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->m_feverMode && thisplayer->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int FeverRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int FeverRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int FeverRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{ENDLESS FEVER
//==========================================================
EndlessFeverRuleSet::EndlessFeverRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_allClearStart = false;
}

EndlessFeverRuleSet::~EndlessFeverRuleSet()
{
}

Phase EndlessFeverRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::CHECKFEVER;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->m_feverMode)
			return Phase::CHECKENDFEVER;
		return Phase::DROPGARBAGE;
	}
	case Phase::CHECKENDFEVER: {
		if (thisplayer->m_feverGauge.m_seconds == 0) {
			// Lose
			thisplayer->m_loseWin = LoseWinState::LOSE;
			return Phase::LOSEDROP;
		}
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
		{
			return Phase::DROPGARBAGE;
		}
		// Player popped a chain, drop a new feverchain
		thisplayer->m_poppedChain = false; // Chain must be reset here
		return Phase::DROPFEVER;
	}
	case Phase::ENDFEVER:
		return Phase::DROPGARBAGE;

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER: {
		if (!thisplayer->m_feverMode)
			return Phase::CHECKFEVER; // Not in fever: check if fever should start
		return Phase::PREPARE;
	}
	case Phase::CHECKFEVER: {
		if (thisplayer->m_feverMode)
			return Phase::PREPAREFEVER;
		if (thisplayer->m_allClear == 0)
			return Phase::PREPARE;
		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;
	}
	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;
	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return Phase::IDLE;
	}
}

void EndlessFeverRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void EndlessFeverRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
	thisplayer->m_feverGauge.setCount(7);
	thisplayer->m_feverGauge.setSeconds(60 * 60);
	thisplayer->m_feverGauge.m_maxSeconds = 99;
	thisplayer->m_nextPuyoActive = false;
}

void EndlessFeverRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void EndlessFeverRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void EndlessFeverRuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;
}

void EndlessFeverRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void EndlessFeverRuleSet::onChain(Player*)
{
}

void EndlessFeverRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->m_feverMode && thisplayer->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int EndlessFeverRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int EndlessFeverRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{ENDLESS FEVER VS
//==========================================================
EndlessFeverVsRuleSet::EndlessFeverVsRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
}

EndlessFeverVsRuleSet::~EndlessFeverVsRuleSet()
{
}

Phase EndlessFeverVsRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::CHECKFEVER;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->m_feverMode)
			return Phase::CHECKENDFEVER;
		return Phase::DROPGARBAGE;
	}
	case Phase::CHECKENDFEVER: {
		if (thisplayer->m_feverGauge.m_seconds == 0)
			return Phase::ENDFEVER;
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
			return Phase::DROPGARBAGE;
		// Player popped a chain, drop a new feverchain
		thisplayer->m_poppedChain = false; // Chain must be reset here
		return Phase::DROPFEVER;
	}
	case Phase::ENDFEVER:
		return Phase::DROPGARBAGE;

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER: {
		if (!thisplayer->m_feverMode)
			return Phase::CHECKFEVER; // Not in fever: check if fever should start
		return Phase::PREPARE;
	}
	case Phase::CHECKFEVER: {
		if (thisplayer->m_feverMode)
			return Phase::PREPAREFEVER;
		if (thisplayer->m_allClear == 0)
			return Phase::PREPARE;
		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;
	}
	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;
	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	default:
		return Phase::IDLE;
	}
}

void EndlessFeverVsRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void EndlessFeverVsRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
	thisplayer->m_feverGauge.setCount(7);
	thisplayer->m_feverGauge.setSeconds(99 * 60);
	thisplayer->m_feverGauge.m_endless = true;
	thisplayer->m_normalGarbage.gq = 270;
	thisplayer->updateTray();
	thisplayer->m_nextPuyoActive = false;
}

void EndlessFeverVsRuleSet::onAllClearPop(Player*)
{
}

void EndlessFeverVsRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void EndlessFeverVsRuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;
}

void EndlessFeverVsRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void EndlessFeverVsRuleSet::onChain(Player*)
{
}

void EndlessFeverVsRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverVsRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->m_feverMode && thisplayer->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverVsRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int EndlessFeverVsRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int EndlessFeverVsRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{TSU_ONLINE
//==========================================================
TsuOnlineRuleSet::TsuOnlineRuleSet()
{
	m_delayedFall = false;
	m_doubleSpawn = false;
	m_addDropBonus = true;
	m_voicePatternFever = false;
}

TsuOnlineRuleSet::~TsuOnlineRuleSet()
{
}

Phase TsuOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY: {
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}
	case Phase::WAITGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN) {
				return Phase::WAITCONFIRMGARBAGE;
			}
			return Phase::CHECKLOSER;
		}
	case Phase::WAITCONFIRMGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER:
		return Phase::PREPARE;

	default:
		return Phase::IDLE;
	}
}

void TsuOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(false);
	thisplayer->m_useDropPattern = false;
}

void TsuOnlineRuleSet::onAllClearPop(Player* thisplayer)
{
	// Add 30 nuisance puyos
	// Triggers during phase 30
	if (thisplayer->m_currentPhase == Phase::SEARCHCHAIN) {
		thisplayer->m_currentScore += 30 * thisplayer->m_targetPoint;
		thisplayer->m_allClear = 0;
	}
}

void TsuOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound if player 1 wins or loses
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void TsuOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->m_loseWin = LoseWinState::LOSEWAIT;
	else // No need to wait for anything
		thisplayer->m_loseWin = LoseWinState::LOSE;

	thisplayer->m_currentPhase = Phase::LOSEDROP;
	// Sounds for losing (not player 1)
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

int TsuOnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusTsu[10];
	return linkBonusTsu[n];
}

int TsuOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusTsu[4];
	return colorBonusTsu[n];
}

int TsuOnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonusTsu[15] + 32 * (n - 15), 999);
	return chainBonusTsu[n];
}

void TsuOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = false;
}

//}

//{FEVER ONLINE
//==========================================================
FeverOnlineRuleSet::FeverOnlineRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_allClearStart = false;
	m_feverDeath = false;
	m_bonusEq = true;
}

FeverOnlineRuleSet::~FeverOnlineRuleSet()
{
}

Phase FeverOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY: {
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->m_feverMode) {
			if (thisplayer->getPlayerType() == ONLINE)
				return Phase::CHECKENDFEVERONLINE;
			return Phase::CHECKENDFEVER;
		}
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}
	case Phase::WAITGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::CHECKENDFEVERONLINE:
		return Phase::CHECKENDFEVER;
	case Phase::CHECKENDFEVER: {
		if (thisplayer->m_feverEnd) {
			return Phase::ENDFEVER;
		}
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return Phase::WAITGARBAGE;
			return Phase::DROPGARBAGE;
		}
		// Player popped a chain, drop a new feverchain
		thisplayer->m_poppedChain = false; // Chain must be reset here
		return Phase::DROPFEVER;
	}
	case Phase::ENDFEVER: {
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN) {
				return Phase::WAITCONFIRMGARBAGE;
			}
			return Phase::CHECKLOSER;
		}
	case Phase::WAITCONFIRMGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER: {
		if (!thisplayer->m_feverMode)
			return Phase::CHECKFEVER; // Not in fever: check if fever should start
		return Phase::PREPARE;
	}
	case Phase::CHECKFEVER: {
		if (thisplayer->m_feverMode)
			return Phase::PREPAREFEVER;
		if (thisplayer->m_allClear == 0)
			return Phase::PREPARE;
		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;
	}
	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;
	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode
	case Phase::WAITLOSE:
		return Phase::WAITLOSE;

	default:
		return Phase::IDLE;
	}
}

void FeverOnlineRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void FeverOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
}

void FeverOnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void FeverOnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void FeverOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->m_loseWin = LoseWinState::LOSEWAIT;
	else // No need to wait for anything
		thisplayer->m_loseWin = LoseWinState::LOSE;

	thisplayer->m_currentPhase = Phase::LOSEDROP;
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

void FeverOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void FeverOnlineRuleSet::onChain(Player*)
{
}

void FeverOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
	// Add a second to last attacker
	if (thisplayer->m_lastAttacker == nullptr)
		return;

	if (!thisplayer->m_lastAttacker->m_feverMode && thisplayer->m_lastAttacker->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_lastAttacker->m_feverGauge.addTime(60);
		thisplayer->m_lastAttacker->showSecondsObj(60);
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
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int FeverOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int FeverOnlineRuleSet::getChainBonus(Player* pl)
{
	const int character = pl->getCharacter();
	int n = pl->m_chain;

	n--;

	if (n < 0)
		return 0;

	if (n == 0)
		return 0; // First value is 0 in PP20th

	if (n > 23)
		return 999;

	if (pl->m_feverMode)
		return (chainBonusFever[character][n] * 699) / 999;

	return (chainBonusNormal[character][n] * 699) / 999;
}
//}

//{FEVER15 ONLINE
//==========================================================
Fever15OnlineRuleSet::Fever15OnlineRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_allClearStart = false;
	m_nFeverChains = 12;
}

Fever15OnlineRuleSet::~Fever15OnlineRuleSet()
{
}

Phase Fever15OnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		if (thisplayer->getPlayerType() == ONLINE) {
			return Phase::WAITGARBAGE;
		}
		return Phase::DROPGARBAGE;

	case Phase::PREPARE:
		return Phase::MOVE;

	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;

	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;

	case Phase::DROPPUYO:
		return Phase::FALLPUYO;

	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;

	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		if (!thisplayer->m_foundChain)
			return Phase::CHECKALLCLEAR;

		thisplayer->m_destroyPuyosTimer = 1;
		return Phase::DESTROYPUYO;

	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;

	case Phase::GARBAGE:
		return Phase::DROPPUYO;

	case Phase::CHECKALLCLEAR:
		if (thisplayer->m_feverMode) {
			if (thisplayer->getPlayerType() == ONLINE) {
				return Phase::CHECKENDFEVERONLINE;
			}

			return Phase::CHECKENDFEVER;
		}

		if (thisplayer->getPlayerType() == ONLINE) {
			return Phase::WAITGARBAGE;
		}

		return Phase::DROPGARBAGE;

	case Phase::WAITGARBAGE:
		return Phase::FALLGARBAGE;

	case Phase::CHECKENDFEVERONLINE:
		return Phase::CHECKENDFEVER;

	case Phase::CHECKENDFEVER:
		if (thisplayer->m_feverEnd) {
			return Phase::ENDFEVER;
		}
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
		{
			if (thisplayer->getPlayerType() == ONLINE) {
				return Phase::WAITGARBAGE;
			}

			return Phase::DROPGARBAGE;
		}
		thisplayer->m_poppedChain = false; // C3hain must be reset here
		return Phase::DROPFEVER;

	case Phase::ENDFEVER:
		if (thisplayer->getPlayerType() == ONLINE) {
			return Phase::WAITGARBAGE;
		}

		return Phase::DROPGARBAGE;

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;

	case Phase::FALLGARBAGE:
		// Expect other players to confirm your move
		if (thisplayer->getPlayerType() == HUMAN) {
			return Phase::WAITCONFIRMGARBAGE;
		}

		return Phase::CHECKLOSER;

	case Phase::WAITCONFIRMGARBAGE:
		return Phase::CHECKLOSER;

	case Phase::CHECKLOSER:
		// Not in fever: check if fever should start
		if (!thisplayer->m_feverMode) {
			return Phase::CHECKFEVER;
		}
		return Phase::PREPARE;

	case Phase::CHECKFEVER:
		if (thisplayer->m_feverMode) {
			return Phase::PREPAREFEVER;
		}

		if (thisplayer->m_allClear == 0) {
			return Phase::PREPARE;
		}

		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;

	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;

	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode

	case Phase::WAITLOSE:
		return Phase::WAITLOSE;

	default:
		return Phase::IDLE;
	}
}

void Fever15OnlineRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void Fever15OnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
}

void Fever15OnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void Fever15OnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void Fever15OnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->m_loseWin = LoseWinState::LOSEWAIT;
	else // No need to wait for anything
		thisplayer->m_loseWin = LoseWinState::LOSE;

	thisplayer->m_currentPhase = Phase::LOSEDROP;
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

void Fever15OnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void Fever15OnlineRuleSet::onChain(Player*)
{
}

void Fever15OnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void Fever15OnlineRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->m_feverMode && thisplayer->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int Fever15OnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int Fever15OnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int Fever15OnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{ ENDLESSFEVERVS ONLINE
//==========================================================
EndlessFeverVsOnlineRuleSet::EndlessFeverVsOnlineRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_allClearStart = false;
	m_nFeverChains = 12;
}

EndlessFeverVsOnlineRuleSet::~EndlessFeverVsOnlineRuleSet()
{
}

Phase EndlessFeverVsOnlineRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::CHECKFEVER;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR: {
		if (thisplayer->m_feverMode) {
			if (thisplayer->getPlayerType() == ONLINE)
				return Phase::CHECKENDFEVERONLINE;
			return Phase::CHECKENDFEVER;
		}
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}
	case Phase::WAITGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::CHECKENDFEVERONLINE:
		return Phase::CHECKENDFEVER;
	case Phase::CHECKENDFEVER: {
		if (thisplayer->m_feverEnd) {
			return Phase::ENDFEVER;
		}
		if (!thisplayer->m_poppedChain) // No chain popped: normal progression
		{
			if (thisplayer->getPlayerType() == ONLINE)
				return Phase::WAITGARBAGE;
			return Phase::DROPGARBAGE;
		}
		// Player popped a chain, drop a new feverchain
		thisplayer->m_poppedChain = false; // Chain must be reset here
		return Phase::DROPFEVER;
	}
	case Phase::ENDFEVER: {
		if (thisplayer->getPlayerType() == ONLINE)
			return Phase::WAITGARBAGE;
		return Phase::DROPGARBAGE;
	}

	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		// Expect other players to confirm your move
		{
			// Expect other players to confirm your move
			if (thisplayer->getPlayerType() == HUMAN) {
				return Phase::WAITCONFIRMGARBAGE;
			}
			return Phase::CHECKLOSER;
		}
	case Phase::WAITCONFIRMGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER: {
		if (!thisplayer->m_feverMode) {
			return Phase::CHECKFEVER; // Not in fever: check if fever should start
		}
		return Phase::PREPARE;
	}
	case Phase::CHECKFEVER: {
		if (thisplayer->m_feverMode) {
			return Phase::PREPAREFEVER;
		}
		if (thisplayer->m_allClear == 0) {
			return Phase::PREPARE;
		}
		thisplayer->m_allClear = 0;
		return Phase::DROPPUYO;
	}
	case Phase::PREPAREFEVER:
		return Phase::DROPFEVER;
	case Phase::DROPFEVER:
		return Phase::DROPPUYO;

	case Phase::LOSEDROP:
		return Phase::LOSEDROP; // In fever it's possible the game tries to call endphase in fever mode
	case Phase::WAITLOSE:
		return Phase::WAITLOSE;

	default:
		return Phase::IDLE;
	}
}

void EndlessFeverVsOnlineRuleSet::onSetGame()
{
	m_currentGame->m_legacyRng = true;
	m_currentGame->m_legacyNuisanceDrop = true;
}

void EndlessFeverVsOnlineRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(true);
	m_puyoToClear = 4; // Must be 4!
	thisplayer->m_feverGauge.setCount(7);
	thisplayer->m_feverGauge.setSeconds(99 * 60);
	thisplayer->m_feverGauge.m_endless = true;
	thisplayer->m_normalGarbage.gq = 270;
	thisplayer->updateTray();
	thisplayer->m_nextPuyoActive = false;
}

void EndlessFeverVsOnlineRuleSet::onAllClearPop(Player*)
{
	// Does nothing
}

void EndlessFeverVsOnlineRuleSet::onAllClear(Player* thisplayer)
{
	// Drop a fever pattern on the field or increase feverchain amount & fevertime
	// Implementation of allclear is inside player object (checkstartfever and checkendfever)
	thisplayer->m_allClear = 1;
}

void EndlessFeverVsOnlineRuleSet::onLose(Player* thisplayer)
{
	// Human online player: wait for confirmation
	if (thisplayer->getPlayerType() == HUMAN)
		thisplayer->m_loseWin = LoseWinState::LOSEWAIT;
	else // No need to wait for anything
		thisplayer->m_loseWin = LoseWinState::LOSE;

	thisplayer->m_currentPhase = Phase::LOSEDROP;
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

void EndlessFeverVsOnlineRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;
	// Sound
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void EndlessFeverVsOnlineRuleSet::onChain(Player*)
{
}

void EndlessFeverVsOnlineRuleSet::onOffset(Player* thisplayer)
{
	thisplayer->m_forgiveGarbage = true;
	thisplayer->addFeverCount();
}

void EndlessFeverVsOnlineRuleSet::onAttack(Player* thisplayer)
{
	if (!thisplayer->m_feverMode && thisplayer->m_feverGauge.m_seconds < 60 * 30) {
		thisplayer->m_feverGauge.addTime(60);
		thisplayer->showSecondsObj(60);
	}
}

int EndlessFeverVsOnlineRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusFever[10];
	return linkBonusFever[n];
}

int EndlessFeverVsOnlineRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusFever[4];
	return colorBonusFever[n];
}

int EndlessFeverVsOnlineRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	int character = pl->getCharacter();
	n--;
	if (n < 0)
		return 0;

	if (n > 23)
		return 999;
	if (pl->m_feverMode)
		return chainBonusFever[character][n];
	return chainBonusNormal[character][n];
}
//}

//{EXCAVATION
//==========================================================
ExcavationRuleSet::ExcavationRuleSet()
{
	m_delayedFall = true;
	m_doubleSpawn = true;
	m_addDropBonus = true;
	m_voicePatternFever = true;
}

ExcavationRuleSet::~ExcavationRuleSet()
{
}

Phase ExcavationRuleSet::endPhase(Phase currentPhase, Player* thisplayer)
{
	switch (currentPhase) {
	case Phase::GETREADY:
		return Phase::DROPPUYO;
	case Phase::PREPARE:
		return Phase::MOVE;
	case Phase::MOVE:
		thisplayer->m_createPuyo = true;
		return Phase::CREATEPUYO;
	case Phase::CREATEPUYO:
		return Phase::DROPPUYO;
	case Phase::DROPPUYO:
		return Phase::FALLPUYO;
	case Phase::FALLPUYO:
		return Phase::SEARCHCHAIN;
	case Phase::SEARCHCHAIN:
		// Continue to phase 40 if no chain found, else to 32
		{
			// Continue to phase 40 if no chain found, else to 32
			if (!thisplayer->m_foundChain)
				return Phase::CHECKALLCLEAR;
			// Found chain
			thisplayer->m_destroyPuyosTimer = 1;
		}
		return Phase::DESTROYPUYO;
	case Phase::DESTROYPUYO:
		return Phase::GARBAGE;
	case Phase::GARBAGE:
		return Phase::DROPPUYO;
	case Phase::CHECKALLCLEAR:
		return Phase::DROPGARBAGE;
	case Phase::DROPGARBAGE:
		return Phase::FALLGARBAGE;
	case Phase::FALLGARBAGE:
		return Phase::CHECKLOSER;
	case Phase::CHECKLOSER:
		return Phase::PREPARE;

	default:
		return Phase::IDLE;
	}
}

void ExcavationRuleSet::onInit(Player* thisplayer)
{
	thisplayer->m_targetPoint = m_targetPoint;
	thisplayer->m_feverGauge.setVisible(false);
	thisplayer->m_useDropPattern = false;
}

void ExcavationRuleSet::onAllClearPop(Player* thisplayer)
{
}

void ExcavationRuleSet::onWin(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::WIN;
	thisplayer->m_currentPhase = Phase::WIN_IDLE;

	// Sound if player 1 wins or loses
	if (thisplayer == m_currentGame->m_players[0])
		m_data->snd.win.play(m_data);
	else
		m_data->snd.lose.play(m_data);
}

void ExcavationRuleSet::onLose(Player* thisplayer)
{
	thisplayer->m_loseWin = LoseWinState::LOSE;
	thisplayer->m_currentPhase = Phase::LOSEDROP;

	// Sounds for losing (not player 1)
	if (thisplayer != m_currentGame->m_players[0] && m_currentGame->getActivePlayers() != 1)
		m_data->snd.lose.play(m_data);
}

int ExcavationRuleSet::getLinkBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 11)
		return linkBonusTsu[10];
	return linkBonusTsu[n];
}

int ExcavationRuleSet::getColorBonus(int n)
{
	n--;
	if (n < 0)
		return 0;

	if (n >= 5)
		return colorBonusTsu[4];
	return colorBonusTsu[n];
}

int ExcavationRuleSet::getChainBonus(Player* pl)
{
	int n = pl->m_chain;
	n--;
	if (n < 0)
		return 0;
	if (n >= 16)
		return min(chainBonusTsu[15] + 32 * (n - 15), 999);
	return chainBonusTsu[n];
}
//}

}
