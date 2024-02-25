// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Animation.h"
#include "AssetBundle.h"
#include "DebugLog.h"
#include "Frontend.h"
#include "GameSettings.h"
#include <set>
#include <string>
namespace ppvs {

class AssetManager {
public:
	AssetManager();
	AssetManager(Frontend* fe, DebugLog* dbg);
	~AssetManager();

	void init(Frontend* fe, DebugLog* dbg);

	// Bundle management
	int loadBundle(AssetBundle* bundle, int priority = 0); // adds Bundle, assigns Frontend
	int deleteBundle(AssetBundle* bundle); // This function frees bundle

	// Drop-in replacements for ppvs::Game functions
	int loadGameSounds(Sounds* soundObject);
	int loadGameImages(GameData* gD);

	// Token-based loaders, useful for single loads
	FeImage* loadImage(const std::string& token);
	FeSound* loadSound(const std::string& token);

	FeImage* loadCharImage(const std::string& token, PuyoCharacter character);
	FeSound* loadCharSound(const std::string& token, PuyoCharacter character);


	FeImage* loadCharAnimationSprite(std::string filename, PuyoCharacter character);
	std::string getCharAnimationsFolder(ppvs::PuyoCharacter character);
	std::string getAnimationFolder(std::string token, const std::string& script_name = "animation.xml");

	int initAnimations(Animation* anim, std::string token);
	int initCharAnimations(Animation* anim, PuyoCharacter character);

	int reloadBundles();

	// Listeners for menus
	std::set<std::string> listPuyoSkins();
	std::set<std::string> listBackgrounds();
	std::set<std::string> listSfx();
	std::set<std::string> listCharacterSkins();

	DebugLog* m_debug = nullptr;



protected:
	std::list<AssetBundle*> m_bundleList;
	Frontend* m_front = nullptr;
	DebugLog* dbg = nullptr;

};

} // ppvs
