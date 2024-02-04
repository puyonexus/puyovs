// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Animation.h"
#include "AssetBundle.h"
#include "Frontend.h"
#include "GameSettings.h"
#include <string>
namespace ppvs {

#define GO_THROUGH_BUNDLES(call) for(auto const& i : bundleList { \
    if(!i.call) {                                           \
        break;                                                \
		}                                                           \
	};

class AssetManager {
public:
	AssetManager(Frontend* fe);
	~AssetManager();

	// Bundle management
	int loadBundle(AssetBundle* bundle, int priority = 0); // adds Bundle, assigns Frontend
	int deleteBundle(AssetBundle* bundle); // This function frees bundle

	int loadGameSounds(Sounds* soundObject);
	int loadGameImages(GameData* gD);

	// Token-based loaders, useful for single loads
	FeImage* loadImage(std::string token);
	FeSound* loadSound(std::string token);

	FeImage* loadCharImage(std::string token, PuyoCharacter character);
	FeSound* loadCharSound(std::string token, PuyoCharacter character);

	int initAnimations(Animation* anim, std::string token);
	int initCharAnimations(Animation* anim, PuyoCharacter character);

	// Listeners for menus
	std::vector<std::string> listPuyoSkins();
	std::vector<std::string> listBackgrounds();
	std::vector<std::string> listSfx();
	std::vector<std::string> listCharacterSkins();

private:
	std::list<AssetBundle*> bundleList;
};

} // ppvs
