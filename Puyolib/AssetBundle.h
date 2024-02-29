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

enum class SoundEffectToken {
	chain1,
	chain2,
	chain3,
	chain4,
	chain5,
	chain6,
	chain7,
	allClearDrop,
	drop,
	fever,
	feverLight,
	feverTimeCount,
	feverTimeEnd,
	go,
	heavy,
	hit,
	lose,
	move,
	nuisanceHitL,
	nuisanceHitM,
	nuisanceHitS,
	nuisanceL,
	nuisanceS,
	ready,
	rotate,
	win,
	decide,
	cancel,
	cursor,
	charChain1,
	charChain2,
	charChain3,
	charChain4,
	charChain5,
	charChain6,
	charChain7,
	charSpell1,
	charSpell2,
	charSpell3,
	charSpell4,
	charSpell5,
	charCounter,
	charCounterSpell,
	charDamage1,
	charDamage2,
	charChoose,
	charFeverSuccess,
	charFeverFail,
	charFever,
	charLose,
	charWin
};

enum class ImageToken {
	puyo,
	imgMenu,
	imgLight,
	imgLightS,
	imgLightHit,
	imgFSparkle,
	imgFLight,
	imgFLightHit,
	imgTime,
	imgBackground,
	imgFieldFever,
	imgNextPuyoBackgroundR,
	imgNextPuyoBackgroundL,
	imgFeverBack,
	imgField1,
	imgField2,
	imgBorder1,
	imgBorder2,
	imgPlayerBorder,
	imgSpice,
	imgScore,
	imgAllClear,
	imgLose,
	imgWin,
	imgFeverGauge,
	imgSeconds,
	imgCharHolder,
	imgNameHolder,
	imgBlack,
	imgDropSet,
	imgChain,
	imgCheckMark,
	imgPlayerCharSelect,
	imgPlayerNumber,
	imgCharField,
	imgCharSelect,
	imgCharName,
	imgCharIcon
};

enum class AnimationToken {
	bg,
	characterPack
};

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
	std::string token2fn(SoundEffectToken token, const std::string& custom = "");
	std::string token2fn(ImageToken token, const std::string& custom = "");
	std::string token2fn(AnimationToken token, const std::string& custom = "");
	// For character-specific tokens
	std::string token2fn(const std::string& token,  PuyoCharacter character);
	std::string token2fn(SoundEffectToken token, PuyoCharacter character);
	std::string token2fn(ImageToken token, PuyoCharacter character);
	std::string token2fn(AnimationToken token, PuyoCharacter character);


	DebugLog* m_debug {};

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

	std::map<SoundEffectToken, std::string> sndTokenToPseudoFn = {
		{ SoundEffectToken::chain1, "%base%/%usersounds%/%sfx%/chain1.ogg" },
		{ SoundEffectToken::chain2, "%base%/%usersounds%/%sfx%/chain2.ogg" },
		{ SoundEffectToken::chain3, "%base%/%usersounds%/%sfx%/chain3.ogg" },
		{ SoundEffectToken::chain4, "%base%/%usersounds%/%sfx%/chain4.ogg" },
		{ SoundEffectToken::chain5, "%base%/%usersounds%/%sfx%/chain5.ogg" },
		{ SoundEffectToken::chain6, "%base%/%usersounds%/%sfx%/chain6.ogg" },
		{ SoundEffectToken::chain7, "%base%/%usersounds%/%sfx%/chain7.ogg" },
		{ SoundEffectToken::allClearDrop, "%base%/%usersounds%/%sfx%/allclear.ogg" },
		{ SoundEffectToken::drop, "%base%/%usersounds%/%sfx%/drop.ogg" },
		{ SoundEffectToken::fever, "%base%/%usersounds%/%sfx%/fever.ogg" },
		{ SoundEffectToken::feverLight, "%base%/%usersounds%/%sfx%/feverlight.ogg" },
		{ SoundEffectToken::feverTimeCount, "%base%/%usersounds%/%sfx%/fevertimecount.ogg" },
		{ SoundEffectToken::feverTimeEnd, "%base%/%usersounds%/%sfx%/fevertimeend.ogg" },
		{ SoundEffectToken::go, "%base%/%usersounds%/%sfx%/go.ogg" },
		{ SoundEffectToken::heavy, "%base%/%usersounds%/%sfx%/heavy.ogg" },
		{ SoundEffectToken::hit, "%base%/%usersounds%/%sfx%/hit.ogg" },
		{ SoundEffectToken::lose, "%base%/%usersounds%/%sfx%/lose.ogg" },
		{ SoundEffectToken::move, "%base%/%usersounds%/%sfx%/move.ogg" },
		{ SoundEffectToken::nuisanceHitL, "%base%/%usersounds%/%sfx%/nuisance_hitL.ogg" },
		{ SoundEffectToken::nuisanceHitM, "%base%/%usersounds%/%sfx%/nuisance_hitM.ogg" },
		{ SoundEffectToken::nuisanceHitS, "%base%/%usersounds%/%sfx%/nuisance_hitS.ogg" },
		{ SoundEffectToken::nuisanceL, "%base%/%usersounds%/%sfx%/nuisanceL.ogg" },
		{ SoundEffectToken::nuisanceS, "%base%/%usersounds%/%sfx%/nuisanceS.ogg" },
		{ SoundEffectToken::ready, "%base%/%usersounds%/%sfx%/ready.ogg" },
		{ SoundEffectToken::rotate, "%base%/%usersounds%/%sfx%/rotate.ogg" },
		{ SoundEffectToken::win, "%base%/%usersounds%/%sfx%/win.ogg" },
		{ SoundEffectToken::decide, "%base%/%usersounds%/%sfx%/decide.ogg" },
		{ SoundEffectToken::cancel, "%base%/%usersounds%/%sfx%/cancel.ogg" },
		{ SoundEffectToken::cursor, "%base%/%usersounds%/%sfx%/cursor.ogg" },
		{ SoundEffectToken::charChain1, "%base%/%charsetup%/%char%/Voice/chain1.wav" },
		{ SoundEffectToken::charChain2, "%base%/%charsetup%/%char%/Voice/chain2.wav" },
		{ SoundEffectToken::charChain3, "%base%/%charsetup%/%char%/Voice/chain3.wav" },
		{ SoundEffectToken::charChain4, "%base%/%charsetup%/%char%/Voice/chain4.wav" },
		{ SoundEffectToken::charChain5, "%base%/%charsetup%/%char%/Voice/chain5.wav" },
		{ SoundEffectToken::charChain6, "%base%/%charsetup%/%char%/Voice/chain6.wav" },
		{ SoundEffectToken::charChain7, "%base%/%charsetup%/%char%/Voice/chain7.wav" },
		{ SoundEffectToken::charSpell1, "%base%/%charsetup%/%char%/Voice/spell1.wav" },
		{ SoundEffectToken::charSpell2, "%base%/%charsetup%/%char%/Voice/spell2.wav" },
		{ SoundEffectToken::charSpell3, "%base%/%charsetup%/%char%/Voice/spell3.wav" },
		{ SoundEffectToken::charSpell4, "%base%/%charsetup%/%char%/Voice/spell4.wav" },
		{ SoundEffectToken::charSpell5, "%base%/%charsetup%/%char%/Voice/spell5.wav" },
		{ SoundEffectToken::charCounter, "%base%/%charsetup%/%char%/Voice/counter.wav" },
		{ SoundEffectToken::charCounterSpell, "%base%/%charsetup%/%char%/Voice/counterspell.wav" },
		{ SoundEffectToken::charDamage1, "%base%/%charsetup%/%char%/Voice/damage1.wav" },
		{ SoundEffectToken::charDamage2, "%base%/%charsetup%/%char%/Voice/damage2.wav" },
		{ SoundEffectToken::charChoose, "%base%/%charsetup%/%char%/Voice/choose.wav" },
		{ SoundEffectToken::charFeverSuccess, "%base%/%charsetup%/%char%/Voice/feversuccess.wav" },
		{ SoundEffectToken::charFeverFail, "%base%/%charsetup%/%char%/Voice/feverfail.wav" },
		{ SoundEffectToken::charFever, "%base%/%charsetup%/%char%/Voice/fever.wav" },
		{ SoundEffectToken::charLose, "%base%/%charsetup%/%char%/Voice/lose.wav" },
		{ SoundEffectToken::charWin, "%base%/%charsetup%/%char%/Voice/win.wav" },
	};

	std::map<ImageToken, std::string> imgTokenToPseudoFn = {
		{ ImageToken::puyo, "%base%/%puyo%.png" },
		{ ImageToken::imgMenu, "%base%/Data/Menu/menu%custom%.png" },
		{ ImageToken::imgLight, "%base%/Data/Light.png" },
		{ ImageToken::imgLightS, "%base%/Data/Light_s.png" },
		{ ImageToken::imgLightHit, "%base%/Data/Light_hit.png" },
		{ ImageToken::imgFSparkle, "%base%/Data/fsparkle.png" },
		{ ImageToken::imgFLight, "%base%/Data/fLight.png" },
		{ ImageToken::imgFLightHit, "%base%/Data/fLight_hit.png" },
		{ ImageToken::imgTime, "%base%/%background%/time.png" },
		{ ImageToken::imgBackground, "%base%/%background%/back.png" },
		{ ImageToken::imgFieldFever, "%base%/%background%/ffield.png" },
		{ ImageToken::imgNextPuyoBackgroundR, "%base%/%background%/nextR.png" },
		{ ImageToken::imgNextPuyoBackgroundL, "%base%/%background%/nextL.png" },
		{ ImageToken::imgFeverBack, "%base%/Data/Fever/f%custom%.png" },
		{ ImageToken::imgField1, "%base%/%background%/field1.png" },
		{ ImageToken::imgField2, "%base%/%background%/field2.png" },
		{ ImageToken::imgBorder1, "%base%/%background%/border1.png" },
		{ ImageToken::imgBorder2, "%base%/%background%/border2.png" },
		{ ImageToken::imgPlayerBorder, "%base%/Data/border.png" },
		{ ImageToken::imgSpice, "%base%/Data/spice.png" },
		{ ImageToken::imgScore, "%base%/%background%/score.png" },
		{ ImageToken::imgAllClear, "%base%/%background%/allclear.png" },
		{ ImageToken::imgLose, "%base%/%background%/lose.png" },
		{ ImageToken::imgWin, "%base%/%background%/win.png" },
		{ ImageToken::imgFeverGauge, "%base%/%background%/fgauge.png" },
		{ ImageToken::imgSeconds, "%base%/%background%/fcounter.png" },
		{ ImageToken::imgCharHolder, "%base%/Data/CharSelect/charHolder.png" },
		{ ImageToken::imgNameHolder, "%base%/Data/CharSelect/nameHolder.png" },
		{ ImageToken::imgBlack, "%base%/Data/CharSelect/black.png" },
		{ ImageToken::imgDropSet, "%base%/Data/CharSelect/dropset.png" },
		{ ImageToken::imgChain, "%base%/%background%/chain.png" },
		{ ImageToken::imgCheckMark, "%base%/Data/checkmark.png" },
		{ ImageToken::imgPlayerCharSelect, "%base%/Data/CharSelect/charSelect.png" },
		{ ImageToken::imgPlayerNumber, "%base%/Data/CharSelect/playernumber.png" },
		{ ImageToken::imgCharField, "%base%/%charsetup%/%char%/field.png" },
		{ ImageToken::imgCharSelect, "%base%/%charsetup%/%char%/select.png" },
		{ ImageToken::imgCharName, "%base%/%charsetup%/%char%/name.png" },
		{ ImageToken::imgCharIcon, "%base%/%charsetup%/%char%/icon.png" },
	};
	std::map<AnimationToken, std::string> animTokenToPseudoFn = {
		{ AnimationToken::bg, "%base%/%background%/Animation/" },
		{ AnimationToken::characterPack, "%base%/%charsetup%/%char%/Animation/" }
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
	virtual bool init(Frontend* fe) = 0;

	virtual FeImage* loadImage(ImageToken token, std::string custom) = 0;
	virtual FeSound* loadSound(SoundEffectToken token, std::string custom) = 0;

	virtual FeImage* loadCharImage(ImageToken token, PuyoCharacter character) = 0;
	virtual FeSound* loadCharSound(SoundEffectToken token, PuyoCharacter character) = 0;
	virtual std::string getCharAnimationsFolder(PuyoCharacter character) = 0;
	virtual std::string getAnimationFolder(AnimationToken token, std::string script_name) = 0;

	virtual std::list<std::string> listPuyoSkins() = 0;
	virtual std::list<std::string> listBackgrounds() = 0;
	virtual std::list<std::string> listSfx() = 0;
	virtual std::list<std::string> listCharacterSkins() = 0;

	virtual void reload() = 0;
	// Reloads this bundle and gives it a Frontend, usually binding it to a particular game
	virtual void reload(Frontend* fe) = 0;

	bool active = false;
	DebugLog* m_debug {};

protected:
	Frontend* m_frontend {};
};

class FolderAssetBundle : public AssetBundle {
public:
	FolderAssetBundle(Frontend* fe, GameAssetSettings* folderLocations);
	~FolderAssetBundle() override = default;

	AssetBundle* clone() override;
	bool init(Frontend* fe) override;

	FeImage* loadImage(ImageToken token, std::string custom) override;
	FeSound* loadSound(SoundEffectToken token, std::string custom) override;

	FeImage* loadCharImage(ImageToken token, PuyoCharacter character) override;
	FeSound* loadCharSound(SoundEffectToken token, PuyoCharacter character) override;

	// Returns "" if invalid, otherwise a possible candidate for animation
	std::string getCharAnimationsFolder(PuyoCharacter character) override;
	std::string getAnimationFolder(AnimationToken token, std::string script_name) override;

	std::list<std::string> listPuyoSkins() override;
	std::list<std::string> listBackgrounds() override;
	std::list<std::string> listSfx() override;
	std::list<std::string> listCharacterSkins() override;

	void reload() override;
	void reload(Frontend* fe) override;

private:
	TokenFnTranslator* m_translator {};
	GameAssetSettings* m_settings {};
};

} // ppvs
