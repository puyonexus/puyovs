// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#include "GameRenderer.h"
#include "global.h"

namespace ppvs {
GameRenderer::GameRenderer(ppvs::Game* game)
	: m_game(game)
{
}

GameRenderer::~GameRenderer()
{
	delete m_statusText;
	delete m_statusFont;
	delete m_gameData->front;
	delete m_gameData;
}

void GameRenderer::initRenderer(Frontend* f)
{
	m_gameData = new GameData;
	m_gameData->front = f;

	// loadGlobal()
	m_baseAssetDir = m_game->m_settings->baseAssetDir;
	m_gameData->gUserSettings.backgroundDirPath = m_game->m_settings->background;
	m_gameData->gUserSettings.puyoDirPath = m_game->m_settings->puyo;
	m_gameData->gUserSettings.sfxDirPath = m_game->m_settings->sfx;

	loadImages();

	loadAudio();

	if (useShaders) {
		// https://stackoverflow.com/a/33172917
		static auto glowShaderSource =
#include "glowShader.vs"
			;

		m_gameData->glowShader = m_gameData->front->loadShader(glowShaderSource);
		if (m_gameData->glowShader) {
			m_gameData->glowShader->setCurrentTexture("tex");
			m_gameData->glowShader->setParameter("color", 1.0);
		}

		static auto tunnelShaderSource =
#include "tunnelShader.vs"
			;
		m_gameData->tunnelShader = m_gameData->front->loadShader(tunnelShaderSource);
		if (m_gameData->tunnelShader) {
			m_gameData->tunnelShader->setCurrentTexture("tex");
			m_gameData->tunnelShader->setParameter("time", 1.0);
			m_gameData->tunnelShader->setParameter("cl", 0.0, 0.0, 1.0, 1.0);
		}
	}

	m_gameData->globalTimer = 0;
	m_gameData->windowFocus = true;
	m_gameData->playSounds = m_game->m_settings->playSound;
	m_gameData->playMusic = m_game->m_settings->playMusic;

	m_statusFont = m_gameData->front->loadFont("Arial", 14);
	setStatusText("");

	// Game::initGame

	m_backgroundSprite.setImage(m_gameData->imgBackground);
	m_backgroundSprite.setPosition(0.f,0.f);

	m_readyGoObj.init(m_gameData, PosVectorFloat(320, 240), 1, m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + "/Animation/", "ready.xml", 3 * 60);
	m_backgroundAnimation.init(m_gameData, PosVectorFloat(320, 240), 1, m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + "/Animation/", "animation.xml", 30 * 60);
	m_backgroundAnimation.prepareAnimation("background");

	m_charSelectMenu = new CharacterSelect(m_game);
	m_charSelectMenu->prepare();
	m_mainMenu = new Menu(m_game);

	m_timerSprite[0].setImage(m_gameData->imgPlayerNumber);
	m_timerSprite[1].setImage(m_gameData->imgPlayerNumber);
	m_timerSprite[0].setSubRect(0 / 10 * 24, 0, 0, 32);
	m_timerSprite[1].setSubRect(0 / 10 * 24, 0, 0, 32);
	m_timerSprite[0].setCenterBottom();
	m_timerSprite[1].setCenterBottom();
	m_timerSprite[0].setPosition(640 - 24, 32);
	m_timerSprite[1].setPosition(640 - 24 - 24, 32);

}

void GameRenderer::setStatusText(const char* utf8)
{
	if (utf8 != m_lastText && m_statusFont) {
		delete m_statusText;

		m_statusText = m_statusFont->render(utf8);
		m_lastText = utf8;
	}
}

void GameRenderer::loadImages() const
{
	// Load Puyo sheet
	m_gameData->imgPuyo = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserPuyo + m_gameData->gUserSettings.puyoDirPath + std::string(".png"));

	// Check rotation center of quadruplet
	if (m_gameData->imgPuyo && !m_gameData->imgPuyo->error() && m_gameData->imgPuyo->height() > 10) {
		for (int i = 0; i < kPuyoX; i++) {
			if (m_gameData->imgPuyo->pixel(11 * kPuyoX - i, 14 * kPuyoY).a > 50) {
				m_gameData->quadrupletCenter = kPuyoX - i;
				break;
			}
		}
	}
	m_gameData->imgPuyo->setFilter(FilterType::LinearFilter);

	// Light images
	m_gameData->imgLight = m_gameData->front->loadImage(m_baseAssetDir + "Data/Light.png");
	m_gameData->imgLightS = m_gameData->front->loadImage(m_baseAssetDir + "Data/Light_s.png");
	m_gameData->imgLightHit = m_gameData->front->loadImage(m_baseAssetDir + "Data/Light_hit.png");
	m_gameData->imgFSparkle = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/fsparkle.png");
	m_gameData->imgFLight = m_gameData->front->loadImage(m_baseAssetDir + "Data/fLight.png");
	m_gameData->imgFLightHit = m_gameData->front->loadImage(m_baseAssetDir + "Data/fLight_hit.png");

	m_gameData->imgTime = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/time.png"));

	// Menu
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			// Safe (because i/j have defined ranges) BUT WHY?

			m_gameData->imgMenu[i][j] = m_gameData->front->loadImage(m_baseAssetDir + std::string("Data/Menu/menu"+toString(i)+toString(j)+".png"));
			/*
			char buffer[128];
			sprintf(buffer, "Data/Menu/menu%i%i.png", i, j);
			m_gameData->imgMenu[i][j] = m_gameData->front->loadImage(m_baseAssetDir + buffer);*/
		}
	}

	// Backgrounds
	m_gameData->imgBackground = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/back.png"));
	m_gameData->imgFieldFever = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/ffield.png"));

	// Background of next puyo
	m_gameData->imgNextPuyoBackgroundR = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/nextR.png"));
	m_gameData->imgNextPuyoBackgroundL = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/nextL.png"));

	if (!useShaders) {
		for (int i = 0; i < 30; ++i) {
			m_gameData->imgFeverBack[i] = m_gameData->front->loadImage(m_baseAssetDir + std::string("Data/Fever/f" + toString(i) + ".png"));
		}
	}

	// Load default fields. Custom fields should be loaded per character
	m_gameData->imgField1 = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/field1.png"));
	m_gameData->imgField2 = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/field2.png"));
	m_gameData->imgBorder1 = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/border1.png"));
	m_gameData->imgBorder2 = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/border2.png"));
	m_gameData->imgPlayerBorder = m_gameData->front->loadImage(m_baseAssetDir + "Data/border.png");
	m_gameData->imgSpice = m_gameData->front->loadImage(m_baseAssetDir + "Data/spice.png");

	// Other
	m_gameData->imgScore = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/score.png"));
	m_gameData->imgAllClear = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/allclear.png"));
	m_gameData->imgLose = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/lose.png"));
	m_gameData->imgWin = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/win.png"));
	m_gameData->imgFeverGauge = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/fgauge.png"));
	m_gameData->imgSeconds = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserBackgrounds + m_gameData->gUserSettings.backgroundDirPath + std::string("/fcounter.png"));
	m_gameData->imgCharHolder = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/charHolder.png");
	m_gameData->imgNameHolder = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/nameHolder.png");
	m_gameData->imgBlack = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/black.png");
	m_gameData->imgDropSet = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/dropset.png");
	m_gameData->imgChain = m_gameData->front->loadImage(m_baseAssetDir + std::string("User/Backgrounds/") + m_gameData->gUserSettings.backgroundDirPath + std::string("/chain.png"));
	m_gameData->imgCheckMark = m_gameData->front->loadImage(m_baseAssetDir + "Data/checkmark.png");
	m_gameData->imgPlayerCharSelect = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/charSelect.png");

	for (int i = 0; i < kNumCharacters; ++i) {
		m_gameData->imgCharField[i] = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserCharacter + m_game->m_settings->characterSetup[static_cast<PuyoCharacter>(i)] + "/field.png");
		m_gameData->imgCharSelect[i] = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserCharacter + m_game->m_settings->characterSetup[static_cast<PuyoCharacter>(i)] + "/select.png");
		m_gameData->imgCharName[i] = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserCharacter + m_game->m_settings->characterSetup[static_cast<PuyoCharacter>(i)] + "/name.png");
		m_gameData->imgSelect[i] = m_gameData->front->loadImage(m_baseAssetDir + kFolderUserCharacter + m_game->m_settings->characterSetup[static_cast<PuyoCharacter>(i)] + "/select.png");
	}

	m_gameData->imgPlayerNumber = m_gameData->front->loadImage(m_baseAssetDir + "Data/CharSelect/playernumber.png");

}

void GameRenderer::loadAudio() const
{
	Sounds& snd = m_gameData->snd;
	setBuffer(snd.chain[0], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain1.ogg"))));
	setBuffer(snd.chain[1], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain2.ogg"))));
	setBuffer(snd.chain[2], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain3.ogg"))));
	setBuffer(snd.chain[3], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain4.ogg"))));
	setBuffer(snd.chain[4], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain5.ogg"))));
	setBuffer(snd.chain[5], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain6.ogg"))));
	setBuffer(snd.chain[6], (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/chain7.ogg"))));
	setBuffer(snd.allClearDrop, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/allclear.ogg"))));
	setBuffer(snd.drop, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/drop.ogg"))));
	setBuffer(snd.fever, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/fever.ogg"))));
	setBuffer(snd.feverLight, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/feverlight.ogg"))));
	setBuffer(snd.feverTimeCount, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/fevertimecount.ogg"))));
	setBuffer(snd.feverTimeEnd, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/fevertimeend.ogg"))));
	setBuffer(snd.go, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/go.ogg"))));
	setBuffer(snd.heavy, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/heavy.ogg"))));
	setBuffer(snd.hit, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/hit.ogg"))));
	setBuffer(snd.lose, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/lose.ogg"))));
	setBuffer(snd.move, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/move.ogg"))));
	setBuffer(snd.nuisanceHitL, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/nuisance_hitL.ogg"))));
	setBuffer(snd.nuisanceHitM, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/nuisance_hitM.ogg"))));
	setBuffer(snd.nuisanceHitS, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/nuisance_hitS.ogg"))));
	setBuffer(snd.nuisanceL, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/nuisanceL.ogg"))));
	setBuffer(snd.nuisanceS, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/nuisanceS.ogg"))));
	setBuffer(snd.ready, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/ready.ogg"))));
	setBuffer(snd.rotate, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/rotate.ogg"))));
	setBuffer(snd.win, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/win.ogg"))));
	setBuffer(snd.decide, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/decide.ogg"))));
	setBuffer(snd.cancel, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/cancel.ogg"))));
	setBuffer(snd.cursor, (m_gameData->front->loadSound(kFolderUserSounds + m_gameData->gUserSettings.sfxDirPath + std::string("/cursor.ogg"))));

}

} // ppvs