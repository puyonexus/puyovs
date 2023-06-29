// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#include "PlayerRenderer.h"
#include "Game.h"
namespace ppvs {

PlayerRenderer::PlayerRenderer(Player* player)
	: m_player(player)
	, m_gameData(player->m_data)
	, m_game(player->m_currentGame)
	, m_feverGauge(player->m_currentGame->m_gameRenderer->m_gameData)
	, m_feverLight(player->m_currentGame->m_gameRenderer->m_gameData)
{
	m_controls.init(m_player->getPlayerNum(), m_player->getPlayerType(), m_player->m_currentGame->m_settings->recording);
	m_movePuyo.init(m_gameData);

	// Trays
	m_normalTray.init(m_gameData);
	m_feverTray.init(m_gameData);

	// Make field Sprite fit
	m_fieldSprite.setSize(m_fieldNormal.getFieldSize());
	m_fieldFeverSprite.setImage(m_gameData->imgFieldFever);
	m_fieldFeverSprite.setSize(m_fieldNormal.getFieldSize());

	// Spawn ChainWord
	m_chainWord = new ChainWord(m_gameData);
	m_chainWord->setScale(m_globalScale);

	// Initialize movePuyo
	m_movePuyo.prepare(MovePuyoType::DOUBLET, m_player, 0, 0);
}

PlayerRenderer::~PlayerRenderer()
{
	delete m_chainWord;
	while (!m_lightEffect.empty()) {
		delete m_lightEffect.back();
		m_lightEffect.pop_back();
	}

	while (!m_secondsObj.empty()) {
		delete m_secondsObj.back();
		m_secondsObj.pop_back();
	}
	delete m_statusText;
	delete m_statusFont;
}

void PlayerRenderer::initPlayer(FieldProp boardProperties, int playerNum, const int totalPlayers)
{
	m_properties = boardProperties;

	// Initialize garbage trays
	m_normalTray.align(m_properties.offsetX, m_properties.offsetY - m_properties.gridWidth * m_globalScale, m_globalScale);
	m_feverTray.align(m_properties.offsetX, m_properties.offsetY - m_properties.gridWidth * m_globalScale, m_globalScale);
	m_feverTray.setVisible(false);

	m_scoreCounter.init(m_gameData, m_properties.offsetX, m_player->m_activeField->getBottomCoordinates().y + static_cast<float>(kPuyoY) / 4.f * m_globalScale, m_globalScale);

	m_allClearSprite.setImage(m_gameData->imgAllClear);
	m_allClearSprite.setCenter();

	// Killing cross sprite
	m_crossSprite.setImage(m_gameData->imgPuyo);
	// FIXME: make independent on magic values
	m_crossSprite.setSubRect(7 * kPuyoX, 12 * kPuyoY, kPuyoX, kPuyoY);
	m_crossSprite.setCenter(0, 0);

	m_winSprite.setImage(m_gameData->imgWin);
	m_winSprite.setCenter();
	m_loseSprite.setImage(m_gameData->imgLose);
	m_loseSprite.setCenter();

	// Draw border sprite
	m_borderSprite.setCenter();
	m_borderSprite.setPosition(m_properties.centerX, m_properties.centerY / 2);
	m_borderSprite.setScale(1);

	// set field origin to the bottom of the sprite
	m_fieldSprite.setCenterBottom();
	m_fieldFeverSprite.setCenterBottom();

	// Menu objects
	// FIXME: implement a more robust menu system
	for (int i = 0; i < 9; i++) {
		m_colorMenuBorder[i].setImage(m_gameData->imgPlayerBorder);
		m_colorMenuBorder[i].setVisible(false);
		// FIXME: read sprite size and use that instead
		m_colorMenuBorder[i].setSubRect(i % 3 * 24, i / 3 * 24, 24, 24);
	}

	for (int j = 0; j < 5; j++) {
		m_spice[j].setImage(m_gameData->imgSpice);
		// FIXME: read sprite size and use that instead
		m_spice[j].setSubRect(0, j * 50, 138, 50);
		m_spice[j].setVisible(false);
		m_spice[j].setCenter();
	}

	// Do we really need this?
	m_overlaySprite.setImage(nullptr);
	m_overlaySprite.setTransparency(0.5f);
	m_overlaySprite.setScale(2 * 192, 336);
	m_overlaySprite.setColor(0, 0, 0);
	m_overlaySprite.setPosition(-192.f / 2.f, -336.f / 4.f);

	m_charHolderSprite.setImage(m_data->imgCharHolder);
	m_charHolderSprite.setCenter();
	m_charHolderSprite.setPosition(
		m_properties.offsetX + static_cast<float>(m_properties.gridWidth * m_properties.gridX) / 2 * m_globalScale,
		m_properties.offsetY + static_cast<float>(m_properties.gridHeight * m_properties.gridY) / 2 * m_globalScale);
	m_charHolderSprite.setVisible(false);
	m_charHolderSprite.setScale(m_globalScale);
	m_currentCharacterSprite.setVisible(true);

	// Dropsets
	for (auto& i : m_dropSet) {
		i.setVisible(true);
	}
	m_showCharacterTimer = 0;

	// Rematch icon
	m_rematchIcon.setImage(m_data->imgCheckMark);
	m_rematchIcon.setCenter();
	m_rematchIcon.setPosition(
		m_properties.offsetX + static_cast<float>(m_properties.gridWidth * m_properties.gridX) / 2 * m_globalScale,
		m_properties.offsetY + static_cast<float>(m_properties.gridHeight) * (static_cast<float>(m_properties.gridY) / 2 + 3) * m_globalScale);
	m_rematchIcon.setVisible(false);
	m_rematchIcon.setScale(m_globalScale);
	m_rematchIconTimer = 0;

	// Set text
	// Text messages on screen
	m_statusFont = m_data->front->loadFont("Arial", 14);
	m_statusText = nullptr;

	// Broadcast our state if not replaying
	if (m_game->m_settings->recording != RecordState::REPLAYING) {
		setStatusText(m_game->m_translatableStrings.waitingForPlayer.c_str());
	}
}

void PlayerRenderer::reset()
{
	m_fieldSprite.setVisible(true);
	m_fieldFeverSprite.setVisible(false);
	m_fieldNormal.setTransformScale(1);
	m_fieldFever.setTransformScale(1);

	m_movePuyo.prepare(MovePuyoType::DOUBLET, m_player, 0, 0);
	m_movePuyo.setVisible(false);

	m_player->m_activeField = &m_fieldNormal;
	m_normalTray.align(m_player->m_activeField->getProperties().offsetX, m_player->m_activeField->getProperties().offsetY - (32) * m_globalScale, m_globalScale);
	m_normalTray.setDarken(false);
	m_normalTray.update(0);
	m_feverTray.update(0);
}

void PlayerRenderer::updateCharacterArt(bool show)
{
	initVoices();
	const PosVectorFloat charOffset(
		m_properties.centerX * 1,
		m_properties.centerY / 2.0f * 1);
	if (m_game->isBigMatch()) {
		// Big match --> no animations / move packets
		m_characterAnimation.init(m_data, charOffset,1,"");
	} else {
		m_characterAnimation.init(m_data, charOffset, 1, m_game->m_baseAssetDir + kFolderUserCharacter + getCharacterSkinName() + "/Animation/");
	}
	if (m_game->m_settings->useCharacterField) {
		// TODO: import setFieldImage from Player.cpp:573
		setFieldImage(m_character);
	}

	if (show) {
		m_currentCharacterSprite.setImage(m_data->front->loadImage((m_game->m_baseAssetDir + kFolderUserCharacter + getCharacterSkinName() + "/icon.png")));
		m_currentCharacterSprite.setCenter();
		m_currentCharacterSprite.setPosition(m_charHolderSprite.getPosition() + PosVectorFloat(1, 1) - PosVectorFloat(2, 4)); // Correct for shadow
		m_currentCharacterSprite.setVisible(true);
		m_currentCharacterSprite.setScale(m_globalScale);
		m_charHolderSprite.setVisible(true);
		m_showCharacterTimer = 5 * 60;
		for (auto& i : m_dropSet) {
			i.setImage(m_data->front->loadImage("Data/CharSelect/dropset.png"));
		}
		setDropSetSprite(static_cast<int>(m_currentCharacterSprite.getX()), static_cast<int>(m_currentCharacterSprite.getY() + 60.f * m_globalScale), m_character);

	}
}

void PlayerRenderer::setFieldImage(PuyoCharacter character) {
	FeImage* im = m_data->imgCharField[static_cast<unsigned char>(character)];
	if (im && !im->error()) {
		m_fieldSprite.setImage(im);
	}
	else {
		m_fieldSprite.setImage(m_player->getPlayerNum()==1 ? m_data->imgField1 : m_data->imgField2);
	}
	if (m_nextPuyo.getOrientation() < 0) {
		m_fieldSprite.setFlipX(true);
	}
}

/* Stops all character animations */
void PlayerRenderer::killCharacterAnimations()
{
	m_characterAnimation.prepareAnimation("");
}

void PlayerRenderer::readyGoAnimation()
{
	m_game->m_readyGoObj.prepareAnimation("readygo");
}

std::string PlayerRenderer::getCharacterSkinName()
{
	return m_game->m_settings->characterSetup[m_player->getCharacter()];
}

FeSound* PlayerRenderer::loadAudio(std::string& folder, const char* sound)
{
	return m_game->m_gameRenderer->frontend->loadSound(folder + sound);
}

void PlayerRenderer::initVoices()
{
	std::string folder = kFolderUserCharacter + getCharacterSkinName() + std::string("/Voice/");
	m_characterVoiceBank.chain[0].setBuffer(loadAudio(folder, "chain1.wav"));
	m_characterVoiceBank.chain[1].setBuffer(loadAudio(folder, "chain2.wav"));
	m_characterVoiceBank.chain[2].setBuffer(loadAudio(folder, "chain3.wav"));
	m_characterVoiceBank.chain[3].setBuffer(loadAudio(folder, "chain4.wav"));
	m_characterVoiceBank.chain[4].setBuffer(loadAudio(folder, "chain5.wav"));
	m_characterVoiceBank.chain[5].setBuffer(loadAudio(folder, "spell1.wav"));
	m_characterVoiceBank.chain[6].setBuffer(loadAudio(folder, "spell2.wav"));
	m_characterVoiceBank.chain[7].setBuffer(loadAudio(folder, "spell3.wav"));
	m_characterVoiceBank.chain[8].setBuffer(loadAudio(folder, "spell4.wav"));
	m_characterVoiceBank.chain[9].setBuffer(loadAudio(folder, "spell5.wav"));
	m_characterVoiceBank.chain[10].setBuffer(loadAudio(folder, "counter.wav"));
	m_characterVoiceBank.chain[11].setBuffer(loadAudio(folder, "counterspell.wav"));
	m_characterVoiceBank.damage1.setBuffer(loadAudio(folder, "damage1.wav"));
	m_characterVoiceBank.damage2.setBuffer(loadAudio(folder, "damage2.wav"));
	m_characterVoiceBank.choose.setBuffer(loadAudio(folder, "choose.wav"));
	m_characterVoiceBank.fever.setBuffer(loadAudio(folder, "fever.wav"));
	m_characterVoiceBank.feverSuccess.setBuffer(loadAudio(folder, "feversuccess.wav"));
	m_characterVoiceBank.feverFail.setBuffer(loadAudio(folder, "feverfail.wav"));
	m_characterVoiceBank.lose.setBuffer(loadAudio(folder, "lose.wav"));
	m_characterVoiceBank.win.setBuffer(loadAudio(folder, "win.wav"));
}

}