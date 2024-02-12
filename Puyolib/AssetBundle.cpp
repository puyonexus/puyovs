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
		new_list.push_back(folder.path().string());                                                                                    \
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
		{ "%usersounds%", kFolderUserSounds + "." }
	};
}

std::string TokenFnTranslator::token2fn(const std::string& token)
{
	std::string result = tokenToPseudoFn[token]; // Matches with table
	for (const auto& entry : specialTokens) {
		size_t pos = result.find(entry.first); // First occurence
		while (pos != std::string::npos) {
			result.replace(pos, strlen(entry.first), entry.second);
			pos = result.find(entry.first, pos + entry.second.length()); // Next (if any) occurence
		}
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
	return m_frontend->loadImage(m_translator->token2fn(token));
}

// Loads the sound, returns whether the load was successful (for files, whether the file exists and loads)
FeSound* FolderAssetBundle::loadSound(std::string token)
{

	return m_frontend->loadSound(m_translator->token2fn(token));
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
	pvs_ITERATE_DATA_FOLDER(kFolderUserPuyo);
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
int FolderAssetBundle::loadCharImage(FeImage* target, std::string token, PuyoCharacter character)
{
	return 0;
}
int FolderAssetBundle::loadCharSound(FeSound* target, std::string token, PuyoCharacter character)
{
	return 0;
}
int FolderAssetBundle::loadCharAnimations(Animation* target, std::string token, PuyoCharacter character)
{
	return 0;
}

} // ppvs