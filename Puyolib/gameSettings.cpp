#include "gameSettings.h"

namespace ppvs
{

rulesetInfo_t::rulesetInfo_t()
{
	setRules(TSU);
}

rulesetInfo_t::rulesetInfo_t(rules type)
{
	setRules(type);
}

void rulesetInfo_t::setRules(rules type)
{
	custom = true;
	quickDrop = false;
	marginTime = 192;
	targetPoint = 70;
	requiredChain = 0;
	initialFeverCount = 0;
	feverPower = 100;
	puyoToClear = 4;
	Nplayers = 2;
	colors = 4; // Default: cannot choose colors

	rulesetType = type;
	if (type == TSU)
	{
		targetPoint = 70;
	}
	else if (type == FEVER)
	{
		targetPoint = 120;
		puyoToClear = 4;
	}
	else if (type == TSU_ONLINE)
	{
		targetPoint = 70;
	}
	else if (type == FEVER_ONLINE)
	{
		targetPoint = 120;
		puyoToClear = 4;
	}
	else if (type == FEVER15_ONLINE)
	{
		targetPoint = 120;
		puyoToClear = 4;
	}
	else if (type == ENDLESSFEVERVS_ONLINE)
	{
		targetPoint = 120;
		puyoToClear = 4;
	}
}

gameSettings::gameSettings(const rulesetInfo_t& rulesetInfo) : rulesetInfo(rulesetInfo)
{
	// Set default values
	Nplayers = 2;
	Nhumans = 1;

	language = "EN";
	playSound = true;
	playMusic = true;
	background = "Forest";
	puyo = "aqua";
	sfx = "Default";
	defaultPuyoCharacter = ARLE;
	useCharacterField = true;
	startWithCharacterSelect = true;
	pickColors = true;
	useCPUplayers = false;
	spectating = false;
	recording = PVS_NOTRECORDING;
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

gameSettings::~gameSettings()
{
}

}
