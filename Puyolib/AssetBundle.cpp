// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetBundle.h"
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

namespace ppvs {
TokenFnTranslator::TokenFnTranslator(ppvs::GameAssetSettings* aS)
	: gameSettings(aS)
{
	reload();
}

TokenFnTranslator::~TokenFnTranslator()
{
}

void TokenFnTranslator::reload()
{
	specialTokens = {
		{ "%base%", gameSettings->baseAssetDir },
		{ "%lang", gameSettings->language },
		{ "%background%", kFolderUserBackgrounds + gameSettings->background },
		{ "%puyo%", kFolderUserPuyo + gameSettings->puyo },
		{ "%sfx", kFolderUserSounds + gameSettings->sfx }
	};
}

std::string TokenFnTranslator::token2fn(std::string token)
{
	std::string new_token = "";
	new_token = token;
	for (auto& i : specialTokens) {
		std::regex fst(i.first);
		std::regex_replace(new_token, fst, i.second);
	}
	return new_token;
}

FolderAssetBundle::FolderAssetBundle(Frontend* fe, ppvs::GameAssetSettings* folderLocations)
	: AssetBundle(fe)
{
	m_translator = new TokenFnTranslator(folderLocations);
}

// Loads the texture, returns whether the load was successful (for files, whether the file exists and loads)
int FolderAssetBundle::loadImage(FeImage* target, std::string token)
{
	target = m_frontend->loadImage(m_translator->token2fn(token));
	return target->error();
}

// Loads the sound, returns whether the load was successful (for files, whether the file exists and loads)
int FolderAssetBundle::loadSound(FeSound* target, const std::string& token)
{
	std::string name = m_translator->token2fn(token);
	target = m_frontend->loadSound(name);
	return target->error();
}

void FolderAssetBundle::reload()
{
	m_translator->reload();
}

#define pvs_ITERATE_DATA_FOLDER(FOLDER)                                                                                              \
	std::list<std::string> new_list;                                                                                                 \
	for (std::filesystem::directory_entry folder : std::filesystem::directory_iterator(m_translator->token2fn("%base%") + (FOLDER))) { \
		new_list.push_back(folder.path().string());                                                                                  \
	}                                                                                                                                \
	return new_list;

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

} // ppvs