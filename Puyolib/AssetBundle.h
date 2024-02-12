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

class GameAssetSettings {
public:
	GameAssetSettings() {};
	GameAssetSettings(GameSettings* gs);
	std::string baseAssetDir;
	std::string language;
	std::string background;
	std::string puyo;
	std::string sfx;
	std::map<PuyoCharacter, std::string> characterSetup;
};

class TokenFnTranslator {
public:
	explicit TokenFnTranslator(GameAssetSettings* aS);
	~TokenFnTranslator();

	void reload();

	std::string token2fn(const std::string& token);

	DebugLog* m_debug = nullptr;

private:
	const std::string baseSpecifier = "%base%";
	const std::string language = "%lang";
	const std::string background = "%background%";
	const std::string puyo = "%puyo%";
	const std::string sfx = "%sfx%";
	const std::string characterSetup = "%charfolder%";

	const std::string userSounds = "%usersounds%";

	GameAssetSettings* gameSettings;
	std::map<const char*, std::string> specialTokens;
	std::map<std::string, std::string> tokenToPseudoFn = {
		{ "chain[0]", "%base%/%usersounds%/%sfx%/chain1.ogg" },
		{ "chain[1]", "%base%/%usersounds%/%sfx%/chain2.ogg" },
		{ "chain[2]", "%base%/%usersounds%/%sfx%/chain3.ogg" },
		{ "chain[3]", "%base%/%usersounds%/%sfx%/chain4.ogg" },
		{ "chain[4]", "%base%/%usersounds%/%sfx%/chain5.ogg" },
		{ "chain[5]", "%base%/%usersounds%/%sfx%/chain6.ogg" },
		{ "chain[6]", "%base%/%usersounds%/%sfx%/chain7.ogg" },
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
	};
};

class AssetBundle {
public:
	AssetBundle() = default;
	explicit AssetBundle(Frontend* fe)
		: m_frontend(fe) {};
	~AssetBundle() = default;

	virtual int init(Frontend* fe) { return 1; };

	virtual FeImage* loadImage(const std::string token) = 0;
	virtual FeSound* loadSound(const std::string token) = 0;
	virtual int loadAnimations(Animation* target, const std::string token) = 0;

	virtual int loadCharImage(FeImage* target, std::string token, PuyoCharacter character) = 0;
	virtual int loadCharSound(FeSound* target, std::string token, PuyoCharacter character) = 0;
	virtual int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character) = 0;

	virtual std::list<std::string> listPuyoSkins() = 0;
	virtual std::list<std::string> listBackgrounds() = 0;
	virtual std::list<std::string> listSfx() = 0;
	virtual std::list<std::string> listCharacterSkins() = 0;

	virtual void reload() = 0;
	virtual void reload(Frontend* fe) = 0;

	bool active = false;
	DebugLog* m_debug = nullptr;

protected:
	Frontend* m_frontend = nullptr;
};

class FolderAssetBundle : public AssetBundle {
public:
	FolderAssetBundle(Frontend* fe, GameAssetSettings* folderLocations);
	~FolderAssetBundle() = default;

	int init(Frontend* fe);

	FeImage* loadImage(std::string token) override;
	FeSound* loadSound(std::string token) override;

	// TODO: All of them
	int loadAnimations(Animation* target, const std::string token) override;

	int loadCharImage(FeImage* target, std::string token, PuyoCharacter character);
	int loadCharSound(FeSound* target, std::string token, PuyoCharacter character);
	int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character);

	std::list<std::string> listPuyoSkins();
	std::list<std::string> listBackgrounds();
	std::list<std::string> listSfx();
	std::list<std::string> listCharacterSkins();

	void reload() override;
	void reload(Frontend* fe) override;

private:
	TokenFnTranslator* m_translator;
};

} // ppvs
