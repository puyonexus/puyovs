// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "Game.h"
#include "CharacterSelect.h"

namespace ppvs {

class GameRenderer {
public:
	explicit GameRenderer(Game* game);
	~GameRenderer();

	void close();

	// Init functions
	void initRenderer(Frontend* f);
	void loadImages() const;
	void loadAudio() const;

	// Video related
	void renderGame();
	void setWindowFocus(bool focus) const;

	// Audio related
	void changeMusicVolume(int state);
	void continueMusic();

	// Parent objects
	Game* m_game = nullptr;

	// Public variables
	GameData* m_gameData = nullptr;
	Animation m_readyGoObj {};
	Animation m_backgroundAnimation {};
	GameData* m_imageData = nullptr;
	TranslatableStrings m_translatableStrings {};
	std::string m_baseAssetDir;
	std::string m_winsString;


	// Menus
	CharacterSelect* m_charSelectMenu = nullptr;
	Menu* m_mainMenu = nullptr;


private:
	int m_targetVolumeNormal = 100;
	int m_currentVolumeNormal = 0;
	int m_targetVolumeFever = 100;
	int m_currentVolumeFever = 100;
	float m_globalVolume = 1.f;

	// Status text
	FeFont* m_statusFont = nullptr;
	FeText* m_statusText = nullptr;
	std::string m_lastText;
	void setStatusText(const char* utf8);
	Sprite m_backgroundSprite {};

	// Timers
	int m_timerEndMatch = 0;
	Sprite m_timerSprite[2] {};

};

} // ppvs

