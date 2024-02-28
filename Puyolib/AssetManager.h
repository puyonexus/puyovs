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

	// Creates an unactivated clone of the current manager
	AssetManager* clone();
	void activate(Frontend* fe, DebugLog* dbg);

	// Activation means that it's being used in a particular game
	void deactivate() {activated=false;};
	bool is_activated() {return activated;};

	// Bundle management
	int loadBundle(AssetBundle* bundle, int priority = 0); // adds Bundle, assigns Frontend
	int deleteBundle(AssetBundle* bundle); // This function frees bundle
	int unloadAll();

	// Attempts to load the image with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeImage* loadImage(const std::string& token,const std::string& custom = "");

	// Attempts to load the sound with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeSound* loadSound(const std::string& token,const std::string& custom = "");

	// Attempts to load the character-based image with the token (see FolderAssetBundle in AssetBundle.h)
	FeImage* loadCharImage(const std::string& token, PuyoCharacter character);
	// Attempts to load the character-based sound with the token (see FolderAssetBundle in AssetBundle.h)
	FeSound* loadCharSound(const std::string& token, PuyoCharacter character);

	// Attempts to locate a viable folder for animation files
	// Takes a token and filename of script
	std::string getAnimationFolder(std::string token, const std::string& script_name = "animation.xml");
	// Attempts to locate a viable folder for animation files
	// Takes a character, setup is assumed
	std::string getCharAnimationsFolder(ppvs::PuyoCharacter character);

	// Reloads the setup of all available bundles
	// Assumes that you still have access to the setup data objects of each bundle
	int reloadBundles();

	// Iterators for menus
	std::set<std::string> listPuyoSkins();
	std::set<std::string> listBackgrounds();
	std::set<std::string> listSfx();
	std::set<std::string> listCharacterSkins();

	DebugLog* m_debug = nullptr;

protected:
	std::list<AssetBundle*> m_bundleList;
	Frontend* m_front = nullptr;
	DebugLog* dbg = nullptr;
	bool activated = false;
	bool initialized = false;
};

} // ppvs
