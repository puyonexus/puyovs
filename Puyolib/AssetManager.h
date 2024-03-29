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

	// Initialized means that all bundles are loaded and is safe to refer to
	// Here mostly for thread safety of Client
	void init() {initialized=true;};
	void deInit() {initialized=false;};
	bool is_initialized() const {return initialized;};

	// Activation means that it's being used in a particular game
	void activate(Frontend* fe, DebugLog* dbg);
	void deactivate() {activated=false;};
	bool is_activated() const {return activated;};

	// Bundle management
	bool loadBundle(AssetBundle* bundle, int priority = 0); // adds Bundle, assigns Frontend
	bool deleteBundle(AssetBundle* bundle); // This function frees bundle
	bool unloadAll(); // Removes all bundles from the AssetManager

	// Attempts to load the image with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeImage* loadImage(ImageToken token,const std::string& custom = "");

	// Attempts to load the sound with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeSound* loadSound(SoundEffectToken token,const std::string& custom = "");

	// Attempts to load the character-based image with the token (see FolderAssetBundle in AssetBundle.h)
	FeImage* loadImage(ImageToken token, PuyoCharacter character);
	// Attempts to load the character-based sound with the token (see FolderAssetBundle in AssetBundle.h)
	FeSound* loadSound(SoundEffectToken token, PuyoCharacter character);

	// Attempts to locate a viable folder for animation files
	// Takes a token and filename of script
	std::string getAnimationFolder(AnimationToken token, const std::string& script_name = "animation.xml");
	// Attempts to locate a viable folder for animation files
	// Takes a character, setup is assumed
	std::string getAnimationFolder(ppvs::PuyoCharacter character);

	// Reloads the setup of all available bundles
	// Assumes that you still have access to the setup data objects of each bundle
	bool reloadBundles();

	// Iterators for menus
	std::set<std::string> listPuyoSkins();
	std::set<std::string> listBackgrounds();
	std::set<std::string> listSfx();
	std::set<std::string> listCharacterSkins();


protected:
	std::list<AssetBundle*> m_bundleList;
	Frontend* m_front{};
	DebugLog* m_debug{};
	bool activated = false;
	bool initialized = false;
};

} // ppvs
