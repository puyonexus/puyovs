// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Animation.h"
#include "AssetBundle.h"
#include "DebugLog.h"
#include "Frontend.h"
#include "GameSettings.h"
#include <atomic>
#include <set>
#include <string>
namespace ppvs {
class AssetManagerPriv;
class AssetManager {
public:
	AssetManager();
	AssetManager(DebugLog* dbg);
	~AssetManager();

	// Creates a proxy object for a Game instance
	// This ensures that each ppvs::Game object receives a unique pointer that can be erased without any damage to others
	// TODO: use smart pointers for this
	AssetManagerPriv* generateGamePriv(Frontend* frontend, DebugLog* m_dbg);

	// Initialized means that all bundles are loaded and is safe to refer to
	// Here mostly for thread safety of Client
	void init() { initialized = true; };
	void deInit() { initialized = false; };
	bool isInitialized() const { return initialized; };

	// Bundle management
	bool loadBundle(AssetBundle* bundle, int priority = 0); // adds Bundle, assigns Frontend
	bool deleteBundle(AssetBundle* bundle); // This function frees bundle
	bool unloadAll(); // Removes all bundles from the AssetManager

	// Attempts to load the image with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeImage* loadImage(ImageToken token, const std::string& custom = "", Frontend* frontend = nullptr);

	// Attempts to load the sound with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeSound* loadSound(SoundEffectToken token, const std::string& custom = "", Frontend* frontend = nullptr);

	// Attempts to load the character-based image with the token (see FolderAssetBundle in AssetBundle.h)
	FeImage* loadImage(ImageToken token, PuyoCharacter character, Frontend* frontend = nullptr);
	// Attempts to load the character-based sound with the token (see FolderAssetBundle in AssetBundle.h)
	FeSound* loadSound(SoundEffectToken token, PuyoCharacter character, Frontend* frontend = nullptr);

	// Attempts to locate a viable folder for animation files
	// Takes a token and filename of script
	std::string getAnimationFolder(AnimationToken token, const std::string& script_name = "animation.xml");
	// Attempts to locate a viable folder for animation files
	// Takes a character, setup is assumed
	std::string getAnimationFolder(ppvs::PuyoCharacter character);

	// Reloads the setup of all available bundles
	// Assumes that you still have access to the setup data objects of each bundle
	bool reloadBundles();

	// Reloads the setup of all available bundles while overwriting user settings for all bundles
	// TODO: allow special bundles not to be affected
	bool reloadBundles(GameAssetSettings* settings);

	// Iterators for menus
	std::set<std::string> listPuyoSkins();
	std::set<std::string> listBackgrounds();
	std::set<std::string> listSfx();
	std::set<std::string> listCharacterSkins();

protected:
	std::list<AssetBundle*> m_bundleList;
	DebugLog* m_debug {};
	bool initialized = false;
};

class AssetManagerPriv {
public:
	AssetManagerPriv(AssetManager* parent, Frontend* frontend, DebugLog* dbg = nullptr) ;
	~AssetManagerPriv();

	// Activation means that it's being used in a particular game
	void activate(Frontend* fe, DebugLog* dbg);
	void deactivate() { activated = false; };
	bool isActivated() const { return activated; };

	// Attempts to load the image with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeImage* loadImage(ImageToken token, const std::string& custom = "");

	// Attempts to load the sound with the token (see FolderAssetBundle in AssetBundle.h)
	// parameter `custom` will replace %custom% tag in pseudo-filename
	FeSound* loadSound(SoundEffectToken token, const std::string& custom = "");

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

	// (Dis)allows any external object to load any assets.
	void disallow_loading()
	{
		while (load_lock.exchange(true)) { };
	};
	void allow_loading() { load_lock.store(false); };

private:
	AssetManager* own_parent;
	Frontend* m_frontend;
	DebugLog* m_debug;

	bool activated;

	std::atomic<bool> load_lock = { false };
};

} // ppvs
