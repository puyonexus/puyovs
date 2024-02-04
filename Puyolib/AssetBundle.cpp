// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#include "AssetBundle.h"
#include <regex>

namespace ppvs {
TokenFnTranslator::TokenFnTranslator(ppvs::GameAssetSettings* aS)
	: gameSettings(aS)
{
	reload();
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

int FolderAssetBundle::loadImage(FeImage* target, std::string token)
{
	target = m_frontend->loadImage(m_translator->token2fn(token));
	return target->error();
}

int FolderAssetBundle::loadSound(FeSound* target, std::string token)
{

}

void FolderAssetBundle::reload()
{
	m_translator->reload();
}

} // ppvs