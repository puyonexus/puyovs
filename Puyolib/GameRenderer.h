// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once
#include "global.h"
#include "Game.h"
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

	// Public variables
	Animation m_readyGoObj {};
	Animation m_backgroundAnimation {};
	GameData* m_imageData = nullptr;
	TranslatableStrings m_translatableStrings {};
	std::string m_baseAssetDir;
	std::string m_winsString;


private:
	int m_targetVolumeNormal = 100;
	int m_currentVolumeNormal = 0;
	int m_targetVolumeFever = 100;
	int m_currentVolumeFever = 100;
	float m_globalVolume = 1.f;

};

} // ppvs

