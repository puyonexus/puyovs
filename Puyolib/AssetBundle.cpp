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

#define pvs_ITERATE_DATA_FOLDER(FOLDER)                                                                                                \
	std::list<std::string> new_list;                                                                                                   \
	for (std::filesystem::directory_entry folder : std::filesystem::directory_iterator(m_translator->token2fn("%base%") + (FOLDER))) { \
		if (folder.is_directory()) {                                                                                                   \
			new_list.push_back(folder.path().stem());                                                                                  \
		}                                                                                                                              \
	}                                                                                                                                  \
	return new_list;

void TokenFnTranslator::reload()
{
	specialTokens = {
		{ "%base%", gameSettings->baseAssetDir + "." }, // HACK: fix importing from GameSetttings
		{ "%lang", gameSettings->language },
		{ "%background%", kFolderUserBackgrounds + gameSettings->background },
		{ "%puyo%", kFolderUserPuyo + gameSettings->puyo },
		{ "%sfx%", gameSettings->sfx },
		{ "%usersounds%", kFolderUserSounds + "." },
		{ "%charsetup%", kFolderUserCharacter + "." }
	};
}

std::string TokenFnTranslator::token2fn(const std::string& token)
{
	std::string result = sndTokenToPseudoFn[token]; // Matches with table
	if (result == std::string("") || result == token) {
		result = imgTokenToPseudoFn[token];
	}
	if (result == std::string("") || result == token) {
		result = animTokenToPseudoFn[token];
	}

		for (const auto& entry : specialTokens) {
		size_t pos = result.find(entry.first); // First occurence
		while (pos != std::string::npos) {
			result.replace(pos, strlen(entry.first), entry.second);
			pos = result.find(entry.first, pos + entry.second.length()); // Next (if any) occurence
		}
	}
	return result;
}
// FIXME: duplicated code
std::string TokenFnTranslator::token2fn(const std::string& token, PuyoCharacter character)
{

	std::string result = token2fn(token);
	// Character-specific
	size_t pos = result.find("%char%"); // First occurence
	while (pos != std::string::npos) {
		result.replace(pos, strlen("%char%"), gameSettings->characterSetup[character]);
		pos = result.find("%char%", pos + gameSettings->characterSetup[character].length()); // Next (if any) occurence
	}

	return result;
}

FolderAssetBundle::FolderAssetBundle(Frontend* fe, ppvs::GameAssetSettings* folderLocations)
	: AssetBundle(fe)
{
	m_translator = new TokenFnTranslator(folderLocations);
}

int FolderAssetBundle::init(ppvs::Frontend* fe)
{

	if (!m_frontend) {
		m_frontend = fe;
		return 0;
	}
	return 1; // invalid call
}

// Loads the texture, returns whether the load was successful (for files, whether the file exists and loads)
FeImage* FolderAssetBundle::loadImage(std::string token)
{
	m_translator->m_debug = m_debug;
	return m_frontend->loadImage(m_translator->token2fn(token));
}

// Loads the sound, returns whether the load was successful (for files, whether the file exists and loads)
FeSound* FolderAssetBundle::loadSound(std::string token)
{

	return m_frontend->loadSound(m_translator->token2fn(token));
}

FeImage* FolderAssetBundle::loadCharImage(std::string token, PuyoCharacter character)
{
	return m_frontend->loadImage(m_translator->token2fn(token, character));
}

FeSound* FolderAssetBundle::loadCharSound(std::string token, PuyoCharacter character)
{
	return m_frontend->loadSound(m_translator->token2fn(token, character));
}

bool isPossiblyAnimation(std::string fname,std::string script_name)
{
	std::filesystem::path folder(fname);
	std::filesystem::path file(fname + script_name);
	return exists(folder) && is_directory(folder) && exists(file);
}

FeImage* FolderAssetBundle::loadCharAnimationSprite(std::string filename, PuyoCharacter character)
{
	return m_frontend->loadImage(getCharAnimationsFolder(character) + filename);
}

std::string FolderAssetBundle::getCharAnimationsFolder(ppvs::PuyoCharacter character)
{
	std::string dir = m_translator->token2fn("animationCharFolder", character);
	return isPossiblyAnimation(dir,"animation.xml") ? dir : "";
}

std::string FolderAssetBundle::getAnimationFolder(std::string token, std::string script_name)
{
	std::string dir = m_translator->token2fn(token);
	return isPossiblyAnimation(dir,script_name) ? dir : "";
}

void FolderAssetBundle::reload(Frontend* fe)
{
	if (fe) {
		m_frontend = fe;
	}
	m_translator->reload();
}

void FolderAssetBundle::reload()
{
	m_translator->reload();
}

std::list<std::string> FolderAssetBundle::listPuyoSkins()
{
	std::list<std::string> new_list;
	for (std::filesystem::directory_entry folder : std::filesystem::directory_iterator(m_translator->token2fn("%base%") + (kFolderUserPuyo))) {
		new_list.push_back(folder.path().stem());
	}
	return new_list;
}

std::list<std::string> FolderAssetBundle::listBackgrounds()
{
	pvs_ITERATE_DATA_FOLDER(kFolderUserBackgrounds);
}

std::list<std::string> FolderAssetBundle::listSfx()
{
	pvs_ITERATE_DATA_FOLDER(kFolderUserSounds);
}

std::list<std::string> FolderAssetBundle::listCharacterSkins()
{
	pvs_ITERATE_DATA_FOLDER(kFolderUserCharacter);
}

// TODO
int FolderAssetBundle::loadAnimations(Animation* target, const std::string token)
{
	return 0;
}

} // ppvs