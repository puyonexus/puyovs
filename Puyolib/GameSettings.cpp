#include "GameSettings.h"

namespace ppvs {

RuleSetInfo::RuleSetInfo()
{
	setRules(Rules::TSU);
}

RuleSetInfo::RuleSetInfo(const Rules type)
{
	setRules(type);
}

void RuleSetInfo::setRules(const Rules type)
{
	custom = true;
	quickDrop = false;
	marginTime = 192;
	requiredChain = 0;
	initialFeverCount = 0;
	feverPower = 100;
	puyoToClear = 4;
	numPlayers = 2;
	colors = 4; // Default: cannot choose colors
	ruleSetType = type;
	switch (type) {
	case Rules::TSU:
	case Rules::TSU_ONLINE:
	case Rules::ENDLESS:
	case Rules::ENDLESSFEVER:
	case Rules::ENDLESSFEVERVS:
	case Rules::EXCAVATION:
		targetPoint = 70;
		break;

	case Rules::FEVER:
	case Rules::FEVER_ONLINE:
	case Rules::FEVER15_ONLINE:
	case Rules::ENDLESSFEVERVS_ONLINE:
		targetPoint = 120;
	}
}

GameSettings::GameSettings(const RuleSetInfo& ruleSetInfo)
	: ruleSetInfo(ruleSetInfo)
{
	// Set default values
	numPlayers = 2;
	numHumans = 1;

	language = "EN";
	playSound = true;
	playMusic = true;
	background = "Forest";
	puyo = "Default";
	sfx = "Default";
	defaultPuyoCharacter = ARLE;
	useCharacterField = true;
	startWithCharacterSelect = true;
	swapABConfirm = false;
	pickColors = true;
	useCpuPlayers = false;
	spectating = false;
	recording = RecordState::NOT_RECORDING;
	showNames = 0;
	rankedMatch = false;
	maxWins = 2;

	// Default character folders
	characterSetup[ACCORD] = "Accord";
	characterSetup[AMITIE] = "Amitie";
	characterSetup[ARLE] = "Arle";
	characterSetup[DONGURIGAERU] = "Dongurigaeru";
	characterSetup[DRACO] = "Draco";
	characterSetup[CARBUNCLE] = "Carbuncle";
	characterSetup[ECOLO] = "Ecolo";
	characterSetup[FELI] = "Feli";
	characterSetup[KLUG] = "Klug";
	characterSetup[LEMRES] = "Lemres";
	characterSetup[MAGURO] = "Maguro";
	characterSetup[OCEAN_PRINCE] = "Ocean Prince";
	characterSetup[OSHARE_BONES] = "Oshare Bones";
	characterSetup[RAFFINE] = "Raffine";
	characterSetup[RIDER] = "Rider";
	characterSetup[RISUKUMA] = "Risukuma";
	characterSetup[RINGO] = "Ringo";
	characterSetup[RULUE] = "Rulue";
	characterSetup[SATAN] = "Satan";
	characterSetup[SCHEZO] = "Schezo";
	characterSetup[SIG] = "Sig";
	characterSetup[SUKETOUDARA] = "Suketoudara";
	characterSetup[WITCH] = "Witch";
	characterSetup[YU_REI] = "Yu";

	// Default controls
	controls["1up"] = "up";
	controls["1down"] = "down";
	controls["1left"] = "left";
	controls["1right"] = "right";
	controls["1a"] = "x";
	controls["1b"] = "z";
	controls["1start"] = "enter";

	controls["2up"] = "i";
	controls["2down"] = "k";
	controls["2left"] = "j";
	controls["2right"] = "l";
	controls["2a"] = "m";
	controls["2b"] = "n";
	controls["2start"] = "space";
}

GameSettings::~GameSettings() = default;

}
