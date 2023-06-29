// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3
#pragma once

#include "Player.h"
namespace ppvs {

struct Voices {
	Sound chain[12], damage1, damage2, fever, feverSuccess, feverFail, lose, win, choose;
};

class PlayerRenderer {
public:
	explicit PlayerRenderer(Player* player);
	~PlayerRenderer();

	void initPlayer(FieldProp properties, int playerNum, const int totalPlayers);
	void initVoices();
	void killCharacterAnimations();
	void reset();



	void drawPlayer();
	void drawPlayerEffect();


	// API
	void updateCharacterArt(bool show);

	void readyGoAnimation();
	void drawFieldBack(PosVectorFloat position, float rotation);
	void drawFieldFeverBack(PosVectorFloat position, float rotation);
	void drawAllClear(PosVectorFloat position, float scaleX, float scaleY, float rotation);
	void drawCross(FeRenderTarget* r);
	void drawLose();
	void drawWin();
	void drawColorMenu();

	// Parent
	Player* m_player;

	// Public parameters

	GameData* m_gameData;
	Animation m_characterAnimation;
	ChainWord* m_chainWord = nullptr;
	Controller m_controls;
	FeverCounter m_feverGauge;
	FieldProp m_properties {};
	Game* m_game = nullptr;
	GameData* m_data = nullptr;
	MovePuyo m_movePuyo;
	Voices m_characterVoiceBank;

	// Garbage variables
	NuisanceTray m_normalTray, m_feverTray;

	// Color select
	int m_colorMenuTimer = 0;
	int m_menuHeight = 0;
	int m_spiceSelect = 0;
	Sprite m_colorMenuBorder[9];
	Sprite m_spice[5];

	Sprite m_overlaySprite; // this is actually used by the online functionality
	Sprite m_charHolderSprite;
	Sprite m_currentCharacterSprite;
	int m_showCharacterTimer = 0;
	Sprite m_readyToPlay;
	Sprite m_rematchIcon;
	int m_rematchIconTimer = 0;

	FeFont* m_statusFont = nullptr;
	FeText* m_statusText = nullptr;
	std::string m_lastText;
	void setStatusText(const char* utf8);

private:
	std::string getCharacterSkinName();
	void setDropSetSprite(int x, int y, PuyoCharacter pc);
	FeSound* loadAudio(std::string& folder, const char* sound);
	void setFieldImage(PuyoCharacter character);
	Sprite m_dropSet[16];

	std::vector<LightEffect*> m_lightEffect;
	std::deque<int> m_nextList;
	std::vector<SecondsObject*> m_secondsObj;

	float m_globalScale = 0.f; // Global scale for certain objects (for example sprite animations)

	PlayerType m_type;
	ScoreCounter m_scoreCounter;
	FeverLight m_feverLight;
	PuyoCharacter m_character;
	NextPuyo m_nextPuyo;
	Field m_fieldNormal;
	Field m_fieldFever;
	Field m_fieldTemp;
	Sprite m_fieldSprite;
	Sprite m_fieldFeverSprite;
	Sprite m_allClearSprite;
	Sprite m_crossSprite;
	Sprite m_winSprite;
	Sprite m_loseSprite;
	Sprite m_borderSprite;
	float m_nextPuyoOffsetX = 0.f;
	float m_nextPuyoOffsetY = 0.f;
	float m_nextPuyoScale = 0.f;

	// FIXME: apparently there's a garbage rendering bug
};

}