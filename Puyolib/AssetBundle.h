// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Animation.h"
#include "RuleSet/RuleSet.h"
#include <iostream>
#include <map>
#include <string>

namespace ppvs {

class GameAssetSettings {
	GameAssetSettings() = default;

public:
	std::string baseAssetDir;
	std::string language;
	std::string background;
	std::string puyo;
	std::string sfx;
	std::map<PuyoCharacter, std::string> characterSetup;
};

class TokenFnTranslator {
public:
	TokenFnTranslator(GameAssetSettings* aS);
	~TokenFnTranslator();

	void reload();

	std::string token2fn(std::string token);

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
	std::map<const char*, std::string> tokenToPseudoFn = {
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
	AssetBundle(Frontend* fe)
		: m_frontend(fe) {};
	~AssetBundle() = default;

	virtual int loadImage(FeImage* target, std::string token);
	virtual int loadSound(FeImage* target, std::string token);
	virtual int loadAnimations(Animation* target, std::string token);

	virtual int loadCharImage(FeImage* target, std::string token, PuyoCharacter character);
	virtual int loadCharSound(FeSound* target, std::string token, PuyoCharacter character);
	virtual int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character);

	virtual std::list<std::string> listPuyoSkins();
	virtual std::list<std::string> listBackgrounds();
	virtual std::list<std::string> listSfx();
	virtual std::list<std::string> listCharacterSkins();

	virtual void reload();

	bool active = false;

protected:
	Frontend* m_frontend = nullptr;
};

class FolderAssetBundle : public AssetBundle {
public:
	FolderAssetBundle(Frontend* fe, GameAssetSettings* folderLocations);
	~FolderAssetBundle() = default;

	int loadImage(FeImage* target, std::string token) override;
	int loadSound(FeSound* target, const std::string& token);
	int loadAnimations(Animation* target, std::string token);

	int loadCharImage(FeImage* target, std::string token, PuyoCharacter character);
	int loadCharSound(FeSound* target, std::string token, PuyoCharacter character);
	int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character);

	std::list<std::string> listPuyoSkins();
	std::list<std::string> listBackgrounds();
	std::list<std::string> listSfx();
	std::list<std::string> listCharacterSkins();

	void reload();

private:
	bool fnameExists(std::string fname);
	TokenFnTranslator* m_translator;
};

} // ppvs
