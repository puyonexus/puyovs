// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Animation.h"
#include "DebugLog.h"
#include "GameSettings.h"
#include "RuleSet/RuleSet.h"
#include <iostream>
#include <map>
#include <string>

namespace ppvs {

// A subset of ppvs::GameSettings for only file-related matters, is to be stored Client-wide
class GameAssetSettings {
public:
	GameAssetSettings() = default;
	explicit GameAssetSettings(GameSettings* gs);
	std::string baseAssetDir;
	std::string language;
	std::string background;
	std::string puyo;
	std::string sfx;
	std::map<PuyoCharacter, std::string> characterSetup;
};

// The Token-filename translator converts tokens and transforms them into legacy filenames
// This is not to be used unless legacy parity is required
class TokenFnTranslator {
public:
	explicit TokenFnTranslator(GameAssetSettings* aS);
	~TokenFnTranslator();

	void reload();

	// For regular (global) tokens
	std::string token2fn(const std::string& token, const std::string& custom = "");
	// For character-specific tokens
	std::string token2fn(const std::string& token, PuyoCharacter character);

	DebugLog* m_debug = nullptr;

private:
	GameAssetSettings* gameSettings;

	/*
	 * Special tokens
	 * The Legacy Folder Structure convertor uses special tokens starting and ending with the percentage symbol (%)
	 * to denote specific folder locations. Tokens get converted into specific locations corresponding to the original Puyolib code.
	 * Presented are also generic values for each special token for achieving maximal compatibility.
	 * Read TokenFnTranslator::reload() for an example usage.
	 *
	 * %base%
	 * - base of the asset folder structure, ideally a user-controlled folder containing the entire bundle
	 * - generic value: "./."
	 * %background%
	 * - location of the sub folder containing the chosen background theme
	 * - generic value: "User/Puyo/Forest"
	 * %puyo%
	 * - location of the sub folder containing the chosen puyo theme
	 * - generic value: "User/Backgrounds/Default"
	 * %sfx%
	 * - location of the sub folder containing the chosen sound effect theme
	 * - generic value: "User/Sounds/Default"
	 * %charfolder%
	 * - location of the sub folder containing character-specific assets
	 * - generic value: "User/Characters"
	 * %char%
	 * - name of the preferred theme for a given character
	 * - generic value: ""
	 * %custom%
	 * - custom string to be input by the user, used primarily for repeated numbered instances of objects
	 * - generic value: "0"
	 */
	std::map<const char*, std::string> specialTokens;

	std::map<std::string, std::string> sndTokenToPseudoFn = {
		{ "chain1", "%base%/%usersounds%/%sfx%/chain1.ogg" },
		{ "chain2", "%base%/%usersounds%/%sfx%/chain2.ogg" },
		{ "chain3", "%base%/%usersounds%/%sfx%/chain3.ogg" },
		{ "chain4", "%base%/%usersounds%/%sfx%/chain4.ogg" },
		{ "chain5", "%base%/%usersounds%/%sfx%/chain5.ogg" },
		{ "chain6", "%base%/%usersounds%/%sfx%/chain6.ogg" },
		{ "chain7", "%base%/%usersounds%/%sfx%/chain7.ogg" },
		{ "allClearDrop", "%base%/%usersounds%/%sfx%/allclear.ogg" },
		{ "drop", "%base%/%usersounds%/%sfx%/drop.ogg" },
		{ "fever", "%base%/%usersounds%/%sfx%/fever.ogg" },
		{ "feverLight", "%base%/%usersounds%/%sfx%/feverlight.ogg" },
		{ "feverTimeCount", "%base%/%usersounds%/%sfx%/fevertimecount.ogg" },
		{ "feverTimeEnd", "%base%/%usersounds%/%sfx%/fevertimeend.ogg" },
		{ "go", "%base%/%usersounds%/%sfx%/go.ogg" },
		{ "heavy", "%base%/%usersounds%/%sfx%/heavy.ogg" },
		{ "hit", "%base%/%usersounds%/%sfx%/hit.ogg" },
		{ "lose", "%base%/%usersounds%/%sfx%/lose.ogg" },
		{ "move", "%base%/%usersounds%/%sfx%/move.ogg" },
		{ "nuisanceHitL", "%base%/%usersounds%/%sfx%/nuisance_hitL.ogg" },
		{ "nuisanceHitM", "%base%/%usersounds%/%sfx%/nuisance_hitM.ogg" },
		{ "nuisanceHitS", "%base%/%usersounds%/%sfx%/nuisance_hitS.ogg" },
		{ "nuisanceL", "%base%/%usersounds%/%sfx%/nuisanceL.ogg" },
		{ "nuisanceS", "%base%/%usersounds%/%sfx%/nuisanceS.ogg" },
		{ "ready", "%base%/%usersounds%/%sfx%/ready.ogg" },
		{ "rotate", "%base%/%usersounds%/%sfx%/rotate.ogg" },
		{ "win", "%base%/%usersounds%/%sfx%/win.ogg" },
		{ "decide", "%base%/%usersounds%/%sfx%/decide.ogg" },
		{ "cancel", "%base%/%usersounds%/%sfx%/cancel.ogg" },
		{ "cursor", "%base%/%usersounds%/%sfx%/cursor.ogg" },
		{ "charChain1", "%base%/%charsetup%/%char%/Voice/chain1.wav" },
		{ "charChain2", "%base%/%charsetup%/%char%/Voice/chain2.wav" },
		{ "charChain3", "%base%/%charsetup%/%char%/Voice/chain3.wav" },
		{ "charChain4", "%base%/%charsetup%/%char%/Voice/chain4.wav" },
		{ "charChain5", "%base%/%charsetup%/%char%/Voice/chain5.wav" },
		{ "charChain6", "%base%/%charsetup%/%char%/Voice/chain6.wav" },
		{ "charChain7", "%base%/%charsetup%/%char%/Voice/chain7.wav" },
		{ "charSpell1", "%base%/%charsetup%/%char%/Voice/spell1.wav" },
		{ "charSpell2", "%base%/%charsetup%/%char%/Voice/spell2.wav" },
		{ "charSpell3", "%base%/%charsetup%/%char%/Voice/spell3.wav" },
		{ "charSpell4", "%base%/%charsetup%/%char%/Voice/spell4.wav" },
		{ "charSpell5", "%base%/%charsetup%/%char%/Voice/spell5.wav" },
		{ "charCounter", "%base%/%charsetup%/%char%/Voice/counter.wav" },
		{ "charCounterSpell", "%base%/%charsetup%/%char%/Voice/counterspell.wav" },
		{ "charDamage1", "%base%/%charsetup%/%char%/Voice/damage1.wav" },
		{ "charDamage2", "%base%/%charsetup%/%char%/Voice/damage2.wav" },
		{ "charChoose", "%base%/%charsetup%/%char%/Voice/choose.wav" },
		{ "charFeverSuccess", "%base%/%charsetup%/%char%/Voice/feversuccess.wav" },
		{ "charFeverFail", "%base%/%charsetup%/%char%/Voice/feverfail.wav" },
		{ "charFever", "%base%/%charsetup%/%char%/Voice/fever.wav" },
		{ "charLose", "%base%/%charsetup%/%char%/Voice/lose.wav" },
		{ "charWin", "%base%/%charsetup%/%char%/Voice/win.wav" },
	};

	std::map<std::string, std::string> imgTokenToPseudoFn = {
		{ "puyo", "%base%/%puyo%.png" },
		{ "imgMenu", "%base%/Data/Menu/menu%custom%.png" },
		{ "imgLight", "%base%/Data/Light.png" },
		{ "imgLightS", "%base%/Data/Light_s.png" },
		{ "imgLightHit", "%base%/Data/Light_hit.png" },
		{ "imgFSparkle", "%base%/Data/fsparkle.png" },
		{ "imgFLight", "%base%/Data/fLight.png" },
		{ "imgFLightHit", "%base%/Data/fLight_hit.png" },
		{ "imgTime", "%base%/%background%/time.png" },
		{ "imgBackground", "%base%/%background%/back.png" },
		{ "imgFieldFever", "%base%/%background%/ffield.png" },
		{ "imgNextPuyoBackgroundR", "%base%/%background%/nextR.png" },
		{ "imgNextPuyoBackgroundL", "%base%/%background%/nextL.png" },
		{ "imgFeverBack", "%base%/Data/Fever/f%custom%.png" },
		{ "imgField1", "%base%/%background%/field1.png" },
		{ "imgField2", "%base%/%background%/field2.png" },
		{ "imgBorder1", "%base%/%background%/border1.png" },
		{ "imgBorder2", "%base%/%background%/border2.png" },
		{ "imgPlayerBorder", "%base%/Data/border.png" },
		{ "imgSpice", "%base%/Data/spice.png" },
		{ "imgScore", "%base%/%background%/score.png" },
		{ "imgAllClear", "%base%/%background%/allclear.png" },
		{ "imgLose", "%base%/%background%/lose.png" },
		{ "imgWin", "%base%/%background%/win.png" },
		{ "imgFeverGauge", "%base%/%background%/fgauge.png" },
		{ "imgSeconds", "%base%/%background%/fcounter.png" },
		{ "imgCharHolder", "%base%/Data/CharSelect/charHolder.png" },
		{ "imgNameHolder", "%base%/Data/CharSelect/nameHolder.png" },
		{ "imgBlack", "%base%/Data/CharSelect/black.png" },
		{ "imgDropSet", "%base%/Data/CharSelect/dropset.png" },
		{ "imgChain", "%base%/%background%/chain.png" },
		{ "imgCheckMark", "%base%/Data/checkmark.png" },
		{ "imgPlayerCharSelect", "%base%/Data/CharSelect/charSelect.png" },
		{ "imgPlayerNumber", "%base%/Data/CharSelect/playernumber.png" },
		{ "imgCharField", "%base%/%charsetup%/%char%/field.png" },
		{ "imgCharSelect", "%base%/%charsetup%/%char%/select.png" },
		{ "imgCharName", "%base%/%charsetup%/%char%/name.png" },
		{ "imgCharSelect", "%base%/%charsetup%/%char%/select.png" },
		{ "imgCharIcon", "%base%/%charsetup%/%char%/icon.png" },
	};
	std::map<std::string, std::string> animTokenToPseudoFn = {
		{ "bg", "%base%/%background%/Animation/" },
		{ "animationCharFolder", "%base%/%charsetup%/%char%/Animation/" }
	};
};

class AssetBundle {
public:
	AssetBundle() = default;
	explicit AssetBundle(Frontend* fe)
		: m_frontend(fe) {};
	virtual ~AssetBundle() = default;
	virtual AssetBundle* clone() = 0;

	// Gives this bundle a Frontend, usually binding it to a particular game
	virtual int init(Frontend* fe) { m_frontend = fe;return 0;};

	virtual FeImage* loadImage(std::string token, std::string custom) = 0;
	virtual FeSound* loadSound(std::string token, std::string custom) = 0;

	virtual FeImage* loadCharImage(std::string token, PuyoCharacter character) = 0;
	virtual FeSound* loadCharSound(std::string token, PuyoCharacter character) = 0;
	virtual std::string getCharAnimationsFolder(PuyoCharacter character) = 0;
	virtual std::string getAnimationFolder(std::string token, std::string script_name) = 0;

	virtual std::list<std::string> listPuyoSkins() = 0;
	virtual std::list<std::string> listBackgrounds() = 0;
	virtual std::list<std::string> listSfx() = 0;
	virtual std::list<std::string> listCharacterSkins() = 0;

	virtual void reload() = 0;
	// Reloads this bundle and gives it a Frontend, usually binding it to a particular game
	virtual void reload(Frontend* fe) = 0;

	bool active = false;
	DebugLog* m_debug{};

protected:
	Frontend* m_frontend{};
};

class FolderAssetBundle : public AssetBundle {
public:
	FolderAssetBundle(Frontend* fe, GameAssetSettings* folderLocations);
	~FolderAssetBundle() override = default;

	AssetBundle* clone() override;
	int init(Frontend* fe) override;

	FeImage* loadImage(std::string token, std::string custom) override;
	FeSound* loadSound(std::string token, std::string custom) override;

	FeImage* loadCharImage(std::string token, PuyoCharacter character) override;
	FeSound* loadCharSound(std::string token, PuyoCharacter character) override;

	// Returns "" if invalid, otherwise a possible candidate for animation
	std::string getCharAnimationsFolder(PuyoCharacter character) override;
	// Returns "" if invalid, otherwise a possible candidate for animation
	std::string getAnimationFolder(std::string token, std::string script_name) override;

	std::list<std::string> listPuyoSkins() override;
	std::list<std::string> listBackgrounds() override;
	std::list<std::string> listSfx() override;
	std::list<std::string> listCharacterSkins() override;

	void reload() override;
	void reload(Frontend* fe) override;

private:
	TokenFnTranslator* m_translator;
	GameAssetSettings* m_settings;
};

} // ppvs
