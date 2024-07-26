// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetBundle.h"
#include "GameSettings.h"
#include <filesystem>
#include <regex>
#include <utility>

namespace fs = std::filesystem;

namespace ppvs {

GameAssetSettings::GameAssetSettings(ppvs::GameSettings* gs)
{
	baseAssetDir = gs->baseAssetDir;
	language = gs->language;
	background = gs->background;
	puyo = gs->puyo;
	sfx = gs->sfx;
	characterSetup = gs->characterSetup;
}

TokenFnTranslator::TokenFnTranslator(ppvs::GameAssetSettings* aS)
	: gameSettings(aS)
{
	reload();
}

TokenFnTranslator::~TokenFnTranslator() = default;

void TokenFnTranslator::reload()
{
	specialTokens = {
		{ "%base%", gameSettings->baseAssetDir + "." }, // HACK: fix importing from GameSetttings
		{ "%lang", gameSettings->language },
		{ "%background%", kFolderUserBackgrounds + "/" + gameSettings->background },
		{ "%puyo%", kFolderUserPuyo + "/" + gameSettings->puyo },
		{ "%sfx%", gameSettings->sfx },
		{ "%usersounds%", kFolderUserSounds },
		{ "%charsetup%", kFolderUserCharacter }
		// { "%custom%", custom_value } // Specifier for menus, animations
	};
}
<<<<<<< HEAD

void TokenFnTranslator::reload(GameAssetSettings* settings)
{
	settings->baseAssetDir = gameSettings->baseAssetDir; // Keep current folder location
	gameSettings = settings;
	reload();
}

std::string TokenFnTranslator::token2fn(const SoundEffectToken token, const std::string& custom)
{
	return token2fn(sndTokenToPseudoFn[token], custom);
}
std::string TokenFnTranslator::token2fn(const ImageToken token, const std::string& custom)
{
	return token2fn(imgTokenToPseudoFn[token], custom);
}
=======
std::string TokenFnTranslator::token2fn(const SoundEffectToken token, const std::string& custom)
{
	return token2fn(sndTokenToPseudoFn[token], custom);
}
std::string TokenFnTranslator::token2fn(const ImageToken token, const std::string& custom)
{
	return token2fn(imgTokenToPseudoFn[token], custom);
}
>>>>>>> bd86ca3 (Do not crash when a folder is missing in a FolderBundle)
std::string TokenFnTranslator::token2fn(const AnimationToken token, const std::string& custom)
{
	return token2fn(animTokenToPseudoFn[token], custom);
}

std::string TokenFnTranslator::token2fn(const std::string& token, const std::string& custom)
{
	std::string result = token;

	for (const auto& entry : specialTokens) {
		size_t pos = result.find(entry.first); // First occurence
		while (pos != std::string::npos) {
			result.replace(pos, strlen(entry.first), entry.second);
			pos = result.find(entry.first, pos + entry.second.length()); // Next (if any) occurence
		}
	}
	// HACK: custom token (menus, animations)
	size_t pos = result.find("%custom%"); // First occurence
	while (pos != std::string::npos) {
		result.replace(pos, strlen("%custom%"), custom);
		pos = result.find("%custom%", pos + custom.length()); // Next (if any) occurence
	}

	return result;
}
// FIXME: duplicated code
std::string TokenFnTranslator::token2fn(const SoundEffectToken token, const PuyoCharacter character)
{
	return token2fn(sndTokenToPseudoFn[token], character);
}
std::string TokenFnTranslator::token2fn(const ImageToken token, const PuyoCharacter character)
{
	return token2fn(imgTokenToPseudoFn[token], character);
}
std::string TokenFnTranslator::token2fn(const AnimationToken token, const PuyoCharacter character)
{
	return token2fn(animTokenToPseudoFn[token], character);
}

std::string TokenFnTranslator::token2fn(const std::string& token, PuyoCharacter character)
{

	std::string result = token2fn(token, "");
	// Character-specific
	size_t pos = result.find("%char%"); // First occurence
	while (pos != std::string::npos) {
		result.replace(pos, strlen("%char%"), gameSettings->characterSetup[character]);
		pos = result.find("%char%", pos + gameSettings->characterSetup[character].length()); // Next (if any) occurence
	}

	return result;
}

<<<<<<< HEAD
FolderAssetBundle::FolderAssetBundle(ppvs::GameAssetSettings* folderLocations, bool is_user_defined)
	: AssetBundle()
	, m_settings(folderLocations)
{
	m_translator = new TokenFnTranslator(folderLocations);
	user_defined = is_user_defined;
=======
std::filesystem::path TokenFnTranslator::getBaseFolder()
{
	return gameSettings->baseAssetDir;
}

FolderAssetBundle::FolderAssetBundle(Frontend* fe, ppvs::GameAssetSettings* folderLocations)
	: AssetBundle(fe)
	, m_settings(folderLocations)
{
	m_translator = new TokenFnTranslator(folderLocations);
	// Mark as valid if the folder actually exists on the file system
	if (std::filesystem::is_directory(folderLocations->baseAssetDir)) {
		valid = true;
	}
>>>>>>> bd86ca3 (Do not crash when a folder is missing in a FolderBundle)
}

AssetBundle* FolderAssetBundle::clone()
{
	AssetBundle* new_bundle = new FolderAssetBundle(m_settings);
	return new_bundle;
}

bool FolderAssetBundle::affectedByUser()
{
	return user_defined;
}

// Loads the texture, returns whether the load was successful (for files, whether the file exists and loads)
FeImage* FolderAssetBundle::loadImage(ImageToken token, std::string custom, Frontend* frontend)
{
	return frontend->loadImage(m_translator->token2fn(token, custom));
}

// Loads the sound, returns whether the load was successful (for files, whether the file exists and loads)
FeSound* FolderAssetBundle::loadSound(SoundEffectToken token, std::string custom, Frontend* frontend)
{
	return frontend->loadSound(m_translator->token2fn(token, custom));
}

FeImage* FolderAssetBundle::loadCharImage(ImageToken token, PuyoCharacter character, Frontend* frontend)
{
	return frontend->loadImage(m_translator->token2fn(token, character));
}

FeSound* FolderAssetBundle::loadCharSound(SoundEffectToken token, PuyoCharacter character, Frontend* frontend)
{
	return frontend->loadSound(m_translator->token2fn(token, character));
}

bool isPossiblyAnimation(std::string fname, std::string script_name)
{
	std::filesystem::path folder(fname);
	std::filesystem::path file(fname + script_name);
	return exists(folder) && is_directory(folder) && exists(file);
}

std::string FolderAssetBundle::getCharAnimationsFolder(ppvs::PuyoCharacter character)
{
	std::string dir = m_translator->token2fn(AnimationToken::characterPack, character);
	return isPossiblyAnimation(dir, "animation.xml") ? dir : "";
}

std::string FolderAssetBundle::getAnimationFolder(AnimationToken token, std::string script_name)
{
	std::string dir = m_translator->token2fn(token);
	return isPossiblyAnimation(dir, script_name) ? dir : "";
}

void FolderAssetBundle::reload(GameAssetSettings* settings)
{
	assert(user_defined);
	m_translator->reload(settings);
}

void FolderAssetBundle::reload()
{
	m_translator->reload();
}

std::list<std::string> FolderAssetBundle::listPuyoSkins()
{
	std::list<std::string> new_list;
	std::filesystem::path name = m_translator->getBaseFolder() / kFolderUserPuyo;
	if (is_directory(name)) {
		for (auto folder : std::filesystem::directory_iterator(name)) {
			new_list.push_back((folder.path().stem()).string());
		}
	}
	return new_list;
}

std::list<std::string> FolderAssetBundle::listBackgrounds()
{
	std::list<std::string> new_list;
	std::filesystem::path name = m_translator->getBaseFolder() / kFolderUserBackgrounds;
	if (is_directory(name)) {
		for (auto folder : std::filesystem::directory_iterator(name)) {
			if (folder.is_directory()) {
				new_list.push_back((folder.path().filename()).string());
			}
		}
	}
	return new_list;
}

std::list<std::string> FolderAssetBundle::listSfx()
{
	std::list<std::string> new_list;
	std::filesystem::path name = m_translator->getBaseFolder() / kFolderUserSounds;
	if (is_directory(name)) {
		for (auto folder : std::filesystem::directory_iterator(name)) {
			if (folder.is_directory()) {
				new_list.push_back((folder.path().filename()).string());
			}
		}
	}
	return new_list;
}

std::list<std::string> FolderAssetBundle::listCharacterSkins()
{
	std::list<std::string> new_list;
	std::filesystem::path name = m_translator->getBaseFolder() / kFolderUserCharacter;
	if (is_directory(name)) {
		for (auto folder : std::filesystem::directory_iterator(name)) {
			if (folder.is_directory()) {
				new_list.push_back((folder.path().filename()).string());
			}
		}
	}
	return new_list;
}

} // ppvs
