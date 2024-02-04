// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include <string>
#include <map>
#include <iostream>
#include "Animation.h"
#include "RuleSet/RuleSet.h"


namespace ppvs {

class GameAssetSettings {
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
	TokenFnTranslator(GameAssetSettings *aS);
	~TokenFnTranslator();

	constexpr std::string token2fn(std::string token);
private:
	const std::string baseSpecifier = "%base%";
	const std::string language = "%lang";
	const std::string background = "%background%";
	const std::string puyo = "%puyo%";
	const std::string sfx = "%sfx%";
	const std::string characterSetup = "%charfolder%";

	std::map<std::string,std::string> tokenToPseudoFn = {
		{"",""},
		{"",""},
	};
};





class AssetBundle {
public:
	AssetBundle();
	~AssetBundle();

	virtual int loadImage(FeImage* target, std::string token);
	virtual int loadSound(FeImage* target, std::string token);
	virtual int loadAnimations(Animation* target, std::string token);

	virtual int loadCharImage(FeImage* target, std::string token, PuyoCharacter character);
	virtual int loadCharSound(FeSound* target, std::string token, PuyoCharacter character);
	virtual int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character);
};

class FolderAssetBundle : public AssetBundle {
public:
	FolderAssetBundle(GameAssetSettings folderLocations);
	~FolderAssetBundle();

	int loadImage(FeImage* target, std::string token);
	int loadSound(FeImage* target, std::string token);
	int loadAnimations(Animation* target, std::string token);

	int loadCharImage(FeImage* target, std::string token, PuyoCharacter character);
	int loadCharSound(FeSound* target, std::string token, PuyoCharacter character);
	int loadCharAnimations(Animation* target, std::string token, PuyoCharacter character);

};

} // ppvs

