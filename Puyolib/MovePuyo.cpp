#include "MovePuyo.h"
#include "../PVS_ENet/PVS_Client.h"
#include "Game.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace ppvs {

Shadow::Shadow() = default;

Shadow::~Shadow() = default;

void Shadow::init(GameData* data)
{
	if (m_init) {
		return;
	}

	m_init = true;
	m_data = data;
	m_sprite.setImage((m_data->imgPuyo));
	m_color = 0;
	m_sprite.setVisible(false);
	setSprite();
}

void Shadow::setSprite()
{
	m_sprite.setSubRect(16 * kPuyoX - kPuyoX / 2 * (m_color / 3 + 1), 7 * kPuyoY + kPuyoY / 2 * (m_color % 3), kPuyoX / 2, kPuyoY / 2);
	m_sprite.setCenter(kPuyoX / 4, kPuyoY / 4);
}

void Shadow::draw(FeRenderTarget* target)
{
	m_sprite.draw(target);
}

void Shadow::hotReload()
{
	m_sprite.setImage((m_data->imgPuyo));
	m_sprite.setSubRect(16 * kPuyoX - kPuyoX / 2 * (m_color / 3 + 1), 7 * kPuyoY + kPuyoY / 2 * (m_color % 3), kPuyoX / 2, kPuyoY / 2);
	m_sprite.setCenter(kPuyoX / 4, kPuyoY / 4);
}

MovePuyo::MovePuyo()
{
	m_pos[0].x = 0;
	m_pos[0].y = 0;
	m_pos[1].x = 0;
	m_pos[1].y = 0;
	m_pos[2].x = 0;
	m_pos[2].y = 0;
	m_pos[3].x = 0;
	m_pos[3].y = 0;
	m_initCalled = false;
	m_data = nullptr;

	m_visible = false;
	m_player = nullptr;
	m_type = MovePuyoType::DOUBLET;
	m_spawnX = 0;
	m_spawnY = 0;
	m_color1 = 0;
	m_color2 = 0;
	m_bigColor = 0;
	m_shadowPos[0].x = 0;
	m_shadowPos[0].y = 0;
	m_shadowPos[1].x = 0;
	m_shadowPos[1].y = 0;
	m_shadowPos[2].x = 0;
	m_shadowPos[2].y = 0;
	m_shadowPos[3].x = 0;
	m_shadowPos[3].y = 0;
	m_posReal[0].x = 0;
	m_posReal[0].y = 0;
	m_posReal[1].x = 0;
	m_posReal[1].y = 0;
	m_posXEye1Real = 0;
	m_posYEye1Real = 0;
	m_posXEye2Real = 0;
	m_posYEye2Real = 0;
	m_rotation = 0;
	m_movePuyoAngle = 0;
	m_sprite1Angle = 0;
	m_sprite2Angle = 0;
	m_flip = 0;
	m_transpose = false;
	m_fallCounter = 0;
	m_rotateCounter = 0;
	m_flipCounter = 0;
	m_dropCounter = 0;
	m_holdCounter = 0;
	m_subRectX1 = 0;
	m_subRectY1 = 0;
	m_subRectWidth1 = 0;
	m_subRectHeight1 = 0;
	m_subRectX2 = 0;
	m_subRectY2 = 0;
	m_subRectWidth2 = 0;
	m_subRectHeight2 = 0;
	m_tripletRot = false;
	m_qScale = 1;
}

MovePuyo::~MovePuyo() = default;

void MovePuyo::init(GameData* data)
{
	m_data = data;

	// Set sprites
	m_sprite1.setImage(m_data->imgPuyo);
	m_sprite2.setImage(m_data->imgPuyo);
	m_spriteEye1.setImage(m_data->imgPuyo);
	m_spriteEye2.setImage(m_data->imgPuyo);
	m_visible = false;

	// Set sprite
	m_sprite1.setSubRect(0, kPuyoY * m_color1, kPuyoX, kPuyoY);
	m_sprite2.setSubRect(0, kPuyoY * m_color2, kPuyoX, kPuyoY);
	m_spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * (m_color1 % 2), 12 * kPuyoY + kPuyoY * ((m_color1 / 2) % 2), kPuyoX, kPuyoY);
	m_spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * (m_color2 % 2), 12 * kPuyoY + kPuyoY * ((m_color2 / 2) % 2), kPuyoX, kPuyoY);
	m_sprite1.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_sprite2.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_spriteEye1.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_spriteEye2.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_quick1.setImage(m_data->imgPuyo);
	m_quick2.setImage(m_data->imgPuyo);
	m_quick1.setSubRect(4 * kPuyoX, 13 * kPuyoY, kPuyoX, kPuyoY);
	m_quick2.setSubRect(4 * kPuyoX, 13 * kPuyoY, kPuyoX, kPuyoY);
	m_quick1.setTransparency(0);
	m_quick2.setTransparency(0);

	// Call init to initialize default values
	m_type = MovePuyoType::DOUBLET;
	m_pos[0].x = 0;
	m_pos[0].y = 0;
	m_pos[1].x = 0;
	m_pos[1].y = 0;
	m_pos[2].x = 0;
	m_pos[2].y = 0;
	m_pos[3].x = 0;
	m_pos[3].y = 0;
	m_rotation = 0;
	m_rotateCounter = 0;
	m_spawnX = 0;
	m_spawnY = 0;
	m_color1 = 0;
	m_color2 = 0;
	m_bigColor = 0;
	m_posReal[0].x = 0;
	m_posReal[0].y = 0;
	m_posReal[1].x = 0;
	m_posReal[1].y = 0;
	m_movePuyoAngle = 0;
	m_sprite1Angle = 0;
	m_sprite2Angle = 0;
	m_flip = 0;
	m_fallCounter = 0;
	m_dropCounter = 0;
	m_holdCounter = 0;
	m_rotateCounter = 0;
	m_flipCounter = 0;
	m_qScale = 1;
}

PosVectorInt MovePuyo::getSpawnPoint()
{
	PosVectorInt spawn;
	spawn.x = m_spawnX;
	spawn.y = m_spawnY;
	return spawn;
}

void MovePuyo::setVisible(bool b)
{
	m_visible = b;
}


// Get move puyo ready to be dropped
void MovePuyo::prepare(MovePuyoType type, Player* player, int color1, int color2)
{
	m_player = player;
	m_shadow1.m_data = m_data;
	m_shadow1.init(player->m_data);
	m_shadow2.m_data = m_data;
	m_shadow2.init(player->m_data);
	m_shadow3.m_data = m_data;
	m_shadow3.init(player->m_data);
	m_shadow4.m_data = m_data;
	m_shadow4.init(player->m_data);

	// Triplet and quadruplet start in rotation=1 mode
	if (type == MovePuyoType::DOUBLET || type == MovePuyoType::BIG) {
		m_rotation = 0;
	} else if (type == MovePuyoType::TRIPLET || type == MovePuyoType::TRIPLET_R || type == MovePuyoType::QUADRUPLET) {
		m_rotation = 1;
	}

	// Set to triplet if tripletR
	if (type == MovePuyoType::TRIPLET_R) {
		setType(MovePuyoType::TRIPLET);
		m_transpose = true;
	} else {
		setType(type);
		m_transpose = false;
	}
	// TEMPORARY (pass this through paramaters?)
	// Define a spawn point (x index) and top of the field
	FieldProp prop = m_player->m_activeField->getProperties();
	m_spawnX = (prop.gridX - 1) / 2;
	m_spawnY = prop.gridY - 4;

	// Reset values
	m_movePuyoAngle = 0;
	m_dropCounter = 0;
	m_fallCounter = 0;
	m_flipCounter = 0;
	m_rotateCounter = 0;

	// Set color
	m_color1 = color1;
	m_color2 = color2;
	m_bigColor = color1;
	setSprite();

	// Set spawn point
	m_pos[0].x = m_spawnX;
	m_pos[0].y = m_spawnY;

	// Set initial position of doublet
	setSpriteX();
	setSpriteY();

	m_initCalled = true;
}

// Set sprites of movePuyos
void MovePuyo::setSprite()
{
	if (m_type == MovePuyoType::DOUBLET) {
		m_subRectX1 = 0;
		m_subRectY1 = static_cast<float>(kPuyoY * m_color1);
		m_subRectWidth1 = static_cast<float>(kPuyoX);
		m_subRectHeight1 = static_cast<float>(kPuyoY);
		m_subRectX2 = 0;
		m_subRectY2 = static_cast<float>(kPuyoY * m_color2);
		m_subRectWidth2 = static_cast<float>(kPuyoX);
		m_subRectHeight2 = static_cast<float>(kPuyoY);
		m_sprite1.setSubRect(
			static_cast<int>(m_subRectX1),
			static_cast<int>(m_subRectY1),
			static_cast<int>(m_subRectWidth1),
			static_cast<int>(m_subRectHeight1));
		m_sprite2.setSubRect(
			static_cast<int>(m_subRectX2),
			static_cast<int>(m_subRectY2),
			static_cast<int>(m_subRectWidth2),
			static_cast<int>(m_subRectHeight2));
		m_sprite1.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_sprite2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setVisible(false);
		m_spriteEye2.setVisible(false);
	} else if (m_type == MovePuyoType::TRIPLET && m_color1 != m_color2) {
		m_subRectX1 = static_cast<float>(kPuyoX * m_color1);
		m_subRectY1 = static_cast<float>(kPuyoY * 5 + 1);
		m_subRectWidth1 = static_cast<float>(kPuyoX);
		m_subRectHeight1 = static_cast<float>(kPuyoY * 2 - 1);
		m_subRectX2 = 0.f;
		m_subRectY2 = static_cast<float>(kPuyoY * m_color2);
		m_subRectWidth2 = static_cast<float>(kPuyoX);
		m_subRectHeight2 = static_cast<float>(kPuyoY);
		m_sprite1.setSubRect(
			static_cast<int>(m_subRectX1),
			static_cast<int>(m_subRectY1),
			static_cast<int>(m_subRectWidth1),
			static_cast<int>(m_subRectHeight1));
		m_sprite2.setSubRect(
			static_cast<int>(m_subRectX2),
			static_cast<int>(m_subRectY2),
			static_cast<int>(m_subRectWidth2),
			static_cast<int>(m_subRectHeight2));
		m_sprite1.setCenter(kPuyoX / 2, kPuyoY + kPuyoY / 2);
		m_sprite2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color1 / 2)), 12 * kPuyoY + kPuyoY * (m_color1 % 2), kPuyoX, kPuyoY);
		m_spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color2 / 2)), 12 * kPuyoY + kPuyoY * (m_color2 % 2), kPuyoX, kPuyoY);
		m_spriteEye1.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(false);
	} else if (m_type == MovePuyoType::TRIPLET && m_color1 == m_color2) {
		m_subRectX1 = static_cast<float>(5 * kPuyoX + 2 * kPuyoX * m_color1);
		m_subRectY1 = static_cast<float>(kPuyoY * 5 + 1);
		m_subRectWidth1 = static_cast<float>(kPuyoX * 2);
		m_subRectHeight1 = static_cast<float>(kPuyoY * 2 - 1);
		m_subRectX2 = 0.f;
		m_subRectY2 = static_cast<float>(kPuyoY * m_color2);
		m_subRectWidth2 = static_cast<float>(kPuyoX);
		m_subRectHeight2 = static_cast<float>(kPuyoY);
		m_sprite1.setSubRect(
			static_cast<int>(m_subRectX1),
			static_cast<int>(m_subRectY1),
			static_cast<int>(m_subRectWidth1),
			static_cast<int>(m_subRectHeight1));
		m_sprite2.setSubRect(
			static_cast<int>(m_subRectX2),
			static_cast<int>(m_subRectY2),
			static_cast<int>(m_subRectWidth2),
			static_cast<int>(m_subRectHeight2));
		m_sprite1.setCenter(kPuyoX / 2, kPuyoY + kPuyoY / 2);
		m_sprite2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(false);
		m_spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color1 / 2)), 12 * kPuyoY + kPuyoY * (m_color1 % 2), kPuyoX, kPuyoY);
		m_spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color2 / 2)), 12 * kPuyoY + kPuyoY * (m_color2 % 2), kPuyoX, kPuyoY);
		m_spriteEye1.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(false);
	} else if (m_type == MovePuyoType::QUADRUPLET) {
		m_subRectX1 = 10.f * static_cast<float>(kPuyoX) + (static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f) * m_color1;
		m_subRectY1 = static_cast<float>(kPuyoY) * 13.f;
		m_subRectWidth1 = static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f;
		m_subRectHeight1 = static_cast<float>(kPuyoY) * 2.f;
		m_subRectX2 = 10.f * static_cast<float>(kPuyoX) + (static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f) * m_color2;
		m_subRectY2 = static_cast<float>(kPuyoY) * 13.f;
		m_subRectWidth2 = static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f;
		m_subRectHeight2 = static_cast<float>(kPuyoY) * 2.f;
		m_sprite1.setSubRect(
			static_cast<int>(m_subRectX1),
			static_cast<int>(m_subRectY1),
			static_cast<int>(m_subRectWidth1),
			static_cast<int>(m_subRectHeight1));
		m_sprite2.setSubRect(
			static_cast<int>(m_subRectX2),
			static_cast<int>(m_subRectY2),
			static_cast<int>(m_subRectWidth2),
			static_cast<int>(m_subRectHeight2));
		m_sprite1.setCenter(m_data->quadrupletCenter, kPuyoY);
		m_sprite2.setCenter(m_data->quadrupletCenter, kPuyoY);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color1 / 2)), 12 * kPuyoY + kPuyoY * (m_color1 % 2), kPuyoX, kPuyoY);
		m_spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((m_color2 / 2)), 12 * kPuyoY + kPuyoY * (m_color2 % 2), kPuyoX, kPuyoY);
		m_spriteEye1.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye2.setCenter(kPuyoX / 2, kPuyoY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(true);
	} else if (m_type == MovePuyoType::BIG) {
		m_subRectX1 = static_cast<float>(2 * kPuyoX + 2 * kPuyoX * m_bigColor);
		m_subRectY1 = static_cast<float>(kPuyoY * 7);
		m_subRectWidth1 = static_cast<float>(kPuyoX * 2);
		m_subRectHeight1 = static_cast<float>(kPuyoY * 2);
		m_subRectX2 = 0;
		m_subRectY2 = 0;
		m_subRectWidth2 = 0;
		m_subRectHeight2 = 0;
		m_sprite1.setSubRect(
			static_cast<int>(m_subRectX1),
			static_cast<int>(m_subRectY1),
			static_cast<int>(m_subRectWidth1),
			static_cast<int>(m_subRectHeight1));
		m_sprite2.setSubRect(
			static_cast<int>(m_subRectX2),
			static_cast<int>(m_subRectY2),
			static_cast<int>(m_subRectWidth2),
			static_cast<int>(m_subRectHeight2));
		m_sprite1.setCenter(kPuyoX / 2, kPuyoY + kPuyoY / 2);
		m_sprite2.setCenter(kPuyoX / 2, kPuyoY + kPuyoY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(false);
		m_spriteEye1.setVisible(false);
		m_spriteEye2.setVisible(false);
	}
}

// Moves sprite in the x direction
void MovePuyo::moveSpriteX()
{
	const int offsetX = static_cast<int>(m_player->m_activeField->getProperties().offsetX);
	const int gridSizeX = m_player->m_activeField->getProperties().gridWidth;

	// Actual movement
	if (m_posReal[0].x < offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2)
		m_posReal[0].x += static_cast<float>(gridSizeX) / 2.f;
	else if (m_posReal[0].x > offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2)
		m_posReal[0].x -= static_cast<float>(gridSizeX) / 2.f;

	// Snap
	if (m_posReal[0].x < offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2 && m_posReal[0].x > offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2 - gridSizeX / 2.f)
		m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2);
	else if (m_posReal[0].x > offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2 && m_posReal[0].x < offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2 + gridSizeX / 2.f)
		m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2);

	// Controller movement
	if (m_player->m_currentPhase == Phase::MOVE) {
		if (m_player->m_controls.m_right == 1 && !isAnyTouching(RIGHT)) {
			m_pos[0].x += 1;
			m_data->snd.move.play(m_data);
		}
		if (m_player->m_controls.m_left == 1 && !isAnyTouching(LEFT)) {
			m_pos[0].x -= 1;
			m_data->snd.move.play(m_data);
		}
		if (m_player->m_controls.m_right > 8 && m_player->m_controls.m_right % 2 == 0 && m_player->m_controls.m_left == 0 && !isAnyTouching(RIGHT)) {
			m_pos[0].x += 1;
			m_data->snd.move.play(m_data);
		}
		if (m_player->m_controls.m_left > 8 && m_player->m_controls.m_left % 2 == 0 && m_player->m_controls.m_right == 0 && !isAnyTouching(LEFT)) {
			m_pos[0].x -= 1;
			m_data->snd.move.play(m_data);
		}
		setRotation();
	}
}

// Sets sprite in the x direction
void MovePuyo::setSpriteX()
{
	int offsetX = static_cast<int>(m_player->m_activeField->getProperties().offsetX);
	int gridSizeX = m_player->m_activeField->getProperties().gridWidth;
	m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + kPuyoX / 2);
}

// Sets sprite in the y direction
void MovePuyo::setSpriteY()
{
	FieldProp prop = m_player->m_activeField->getProperties();

	// MovePuyosY
	// Move puyos down: relative to field
	if (m_player->m_currentPhase == Phase::MOVE) {
		// There is now a complication that the origin is now the center of the puyo -> problem when PUYOY and gridsize are unequal
		// So we must correct it by +PUYOY/2-(PUYOY-gridSizeY)
		m_posReal[0].y = prop.offsetY + m_spawnY * prop.gridHeight - m_pos[0].y * prop.gridHeight - (100 - m_fallCounter) / 100.f * prop.gridHeight + kPuyoY / 2 - (kPuyoY - prop.gridHeight);

		if (m_player->m_controls.m_up == 1 && m_player->m_currentGame->m_currentRuleSet->m_quickDrop) {
			if (m_player->m_currentGame->m_connected && m_player->getPlayerType() == HUMAN) {
				// Send
				// q|posx1|posy1|posx2|posy2
				char str[100];
				sprintf(str, "q|%i|%i|%i|%i",
					m_pos[0].x, m_pos[0].y,
					m_pos[1].x, m_pos[1].y);
				m_player->m_currentGame->m_network->sendToChannel(CHANNEL_GAME, str, m_player->m_currentGame->m_channelName.c_str());
			}
			setRotation();
			int gridSizeX = prop.gridWidth;
			m_quick1.setPosition(m_pos[0].x * static_cast<float>(gridSizeX), m_posReal[0].y - prop.offsetY - kPuyoY / 2 + (kPuyoY - prop.gridHeight));
			m_quick2.setPosition(m_pos[1].x * static_cast<float>(gridSizeX), m_posReal[1].y - prop.offsetY - kPuyoY / 2 + (kPuyoY - prop.gridHeight));
			m_quick1.setTransparency(1);
			m_quick2.setTransparency(1);
			m_quick1.setSubRect(4 * kPuyoX + kPuyoX * m_color1, 13 * kPuyoY, kPuyoX, static_cast<int>(static_cast<float>(kPuyoY) * 1.5f));
			m_quick2.setSubRect(4 * kPuyoX + kPuyoX * m_color2, 13 * kPuyoY, kPuyoX, static_cast<int>(static_cast<float>(kPuyoY) * 1.5f));
			m_dropCounter = 100;
			m_holdCounter = 1;
			m_qScale = 1;
			// Drop puyo
			for (int i = 0; i < prop.gridY; i++) {
				if (!isAnyTouching(BELOW)) {
					m_pos[0].y--;
					m_pos[1].y--;
					m_pos[2].y--;
					m_pos[3].y--;
					m_qScale += 1;
				}
			}
		}

		// Add to fallcounter
		if (m_fallCounter < 100)
			m_fallCounter += static_cast<int>(m_player->m_dropSpeed);

		// Add score here
		if (m_player->m_controls.m_down > 0 && m_fallCounter >= 100 && !isAnyTouching(BELOW)) {
			m_player->m_scoreVal += 1;
			m_player->m_dropBonus += 1;
		}

		// Reset fallcounter
		if (m_fallCounter >= 100 && !isAnyTouching(BELOW)) {
			m_fallCounter -= 100;
			m_pos[0].y -= 1;
			m_pos[1].y -= 1;
		}
	}

	// Controller movement
	if (m_player->m_currentPhase == Phase::MOVE) {
		// Move puyo down
		if (m_player->m_controls.m_down > 0 && m_player->m_controls.m_left == 0 && m_player->m_controls.m_right == 0 && m_fallCounter < 100)
			m_fallCounter += m_player->m_currentGame->m_currentRuleSet->m_fastDrop;
		if (m_player->m_controls.m_down > 0 && m_player->m_controls.m_left == 0 && m_player->m_controls.m_right > 0 && isAnyTouching(RIGHT) && m_fallCounter < 100)
			m_fallCounter += m_player->m_currentGame->m_currentRuleSet->m_fastDrop;
		if (m_player->m_controls.m_down > 0 && m_player->m_controls.m_left > 0 && m_player->m_controls.m_right == 0 && isAnyTouching(LEFT) && m_fallCounter < 100)
			m_fallCounter += m_player->m_currentGame->m_currentRuleSet->m_fastDrop;

		// Last bit
		if (m_player->m_controls.m_down > 0 && m_fallCounter >= 100 && isAnyTouching(BELOW) && m_holdCounter > 0)
			m_dropCounter = 100;

		if (m_player->m_controls.m_down > 0 && m_fallCounter >= 100 && isAnyTouching(BELOW)) {
			m_fallCounter = 100;
			m_player->m_scoreVal += 1;
			m_player->m_dropBonus += 1;
			m_holdCounter = 1;
		}
	}

	// Holdcounter
	if (m_player->m_controls.m_down == 0)
		m_holdCounter = 0;

	if (m_player->m_currentPhase != Phase::MOVE) {
		m_holdCounter = 0;
	}
}

// Sets sprite and puyo angle
void MovePuyo::setSpriteAngle()
{
	int gridSizeX = m_player->m_activeField->getProperties().gridWidth;
	int gridSizeY = m_player->m_activeField->getProperties().gridHeight;

	// Actual movement
	if (m_type == MovePuyoType::DOUBLET) {
		// Doublet does not rotate puyo sprites
		m_sprite1Angle = 0;
		m_sprite2Angle = 0;
		// Doublet: puyo2 rotates around puyo1
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin(m_movePuyoAngle * kPiF / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos(m_movePuyoAngle * kPiF / 180);
	}
	if (m_type == MovePuyoType::TRIPLET && (m_transpose == false || m_color1 == m_color2)) {
		// Sprite angle
		m_sprite1Angle = m_movePuyoAngle - 90;
		m_sprite2Angle = 0;
		// Sprite pos
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin(m_movePuyoAngle * kPiF / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos(m_movePuyoAngle * kPiF / 180);
	}
	if (m_type == MovePuyoType::TRIPLET && m_color1 != m_color2 && m_transpose == true) {
		// Sprite angle
		m_sprite1Angle = m_movePuyoAngle;
		m_sprite2Angle = 0;
		// Sprite pos
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin((m_movePuyoAngle - 90) * kPiF / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos((m_movePuyoAngle - 90) * kPiF / 180);
	}
	if (m_type == MovePuyoType::QUADRUPLET) {
		// Quadruplet: the two pieces stay in place and rotate around the center
		// Sprite angle
		m_sprite1Angle = m_movePuyoAngle + 90;
		m_sprite2Angle = m_movePuyoAngle + 180 + 90;
		// Sprite pos
		m_posReal[1].x = m_posReal[0].x;
		m_posReal[1].y = m_posReal[0].y;
	}
	if (m_type == MovePuyoType::BIG) {
		m_sprite1Angle = 0;
		m_sprite2Angle = 0;
	}

	// Controller movement
	if (m_player->m_currentPhase == Phase::MOVE) {
		// (This movement is incomplete for nazo puyo: it doesn't check for collisions upwards)
		if (m_type == MovePuyoType::DOUBLET) {
			// ROTATION
			// Clockwise
			int hor = 0, ver = 0;
			switch (m_rotation) {
			case 0:
				hor = 1;
				ver = 0;
				break;
			case 1:
				hor = 0;
				ver = -1;
				break;
			case 2:
				hor = -1;
				ver = 0;
				break;
			case 3:
				hor = 0;
				ver = 1;
				break;
			default:
				break;
			}

			if (m_player->m_controls.m_a == 1 && m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver)) {
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->m_controls.m_a += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}

			// Counter Clockwise
			switch (m_rotation) {
			case 0:
				hor = -1;
				ver = 0;
				break;
			case 1:
				hor = 0;
				ver = 1;
				break;
			case 2:
				hor = 1;
				ver = 0;
				break;
			case 3:
				hor = 0;
				ver = -1;
				break;
			default:
				break;
			}
			if (m_player->m_controls.m_b == 1 && m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver)) {
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->m_controls.m_b += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}

		else if (m_type == MovePuyoType::TRIPLET) {
			// ROTATION
			// Clockwise
			int hor = 0, ver = 0;
			switch (m_rotation) {
			case 0:
				hor = 1;
				ver = 0;
				break;
			case 1:
				hor = 0;
				ver = -1;
				break;
			case 2:
				hor = -1;
				ver = 0;
				break;
			case 3:
				hor = 0;
				ver = 1;
				break;
			default:
				break;
			}
			if (m_player->m_controls.m_a == 1 && m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver)) {
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->m_controls.m_a += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}

			// Counter Clockwise
			switch (m_rotation) {
			case 1:
				hor = -1;
				ver = 0;
				break;
			case 2:
				hor = 0;
				ver = 1;
				break;
			case 3:
				hor = 1;
				ver = 0;
				break;
			case 0:
				hor = 0;
				ver = -1;
				break;
			default:
				break;
			}
			if (m_player->m_controls.m_b == 1 && m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver)) {
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->m_controls.m_b += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		} else if (m_type == MovePuyoType::QUADRUPLET) {
			if (m_player->m_controls.m_a == 1) {
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->m_controls.m_a += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1) {
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->m_controls.m_b += 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		} else if (m_type == MovePuyoType::BIG) {
			if (m_player->m_controls.m_a == 1) {
				m_bigColor++;
				m_bigColor %= m_player->m_colors;
				setSprite();
				m_player->m_controls.m_a += 1;
			}
			if (m_player->m_controls.m_b == 1) {
				m_bigColor--;
				if (m_bigColor < 0)
					m_bigColor += m_player->m_colors;
				m_bigColor %= m_player->m_colors;
				setSprite();
				m_player->m_controls.m_b += 1;
			}
		}

		if (m_type == MovePuyoType::DOUBLET) {
			// 7Rule
			if (m_player->m_controls.m_a == 1 && m_rotation == 0 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y + 1) && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_pos[0].y += 1;
				m_rotation = 1;
				m_rotateCounter = -8;
				m_player->m_controls.m_a += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 0 && m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y + 1) && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_pos[0].y += 1;
				m_rotation = 3;
				m_rotateCounter = 8;
				m_player->m_controls.m_b += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::TRIPLET) {
			// 7Rule
			if (m_player->m_controls.m_a == 1 && m_rotation == 0 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y + 1)) {
				m_pos[0].y += 1;
				m_rotation = 1;
				m_rotateCounter = -8;
				m_player->m_controls.m_a += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 1 && m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y + 1)) {
				m_pos[0].y += 1;
				m_rotation = 0;
				m_rotateCounter = 8;
				m_player->m_controls.m_b += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}

		if (m_type == MovePuyoType::DOUBLET) {
			// Wall kick
			// Right wall
			if (m_player->m_controls.m_a == 1 && m_rotation == 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_a += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = -8;
				m_rotation = 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 2 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = 8;
				m_rotation = 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}

			// Left wall
			if (m_player->m_controls.m_a == 1 && m_rotation == 2 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_a += 1;
				m_pos[0].x += 1;
				m_rotateCounter = -8;
				m_rotation = 3;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 0 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].x += 1;
				m_rotateCounter = 8;
				m_rotation = 3;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::DOUBLET || m_type == MovePuyoType::TRIPLET) {
			// Ground kick
			if (m_player->m_controls.m_a == 1 && m_rotation == 1 && m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y + 1) && !m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_player->m_controls.m_a += 1;
				m_pos[0].y += 1;
				m_rotateCounter = -8;
				m_rotation = 2;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 3 && m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y + 1) && !m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].y += 1;
				m_rotateCounter = 8;
				m_rotation = 2;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::TRIPLET) {
			// Ground kick
			if (m_player->m_controls.m_b == 1 && m_rotation == 0 && m_player->m_activeField->isEmpty(m_pos[2].x, m_pos[2].y + 1) && !m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].y += 1;
				m_rotateCounter = 8;
				m_rotation = 3;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::DOUBLET) {
			// Flip
			if (m_player->m_controls.m_a == 1 && m_flip == 0 && m_flipCounter > 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_flipCounter = -8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_flip == 0 && m_flipCounter < 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_flipCounter = 8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::DOUBLET) {
			// Flip & Ground Kick
			if (m_player->m_controls.m_a == 1 && m_flip == 0 && m_rotation == 0 && m_flipCounter > 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_flipCounter = -8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				m_pos[0].y += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_flip == 0 && m_rotation == 0 && m_flipCounter < 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1)) {
				m_flipCounter = 8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				m_pos[0].y += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::TRIPLET) {
			// "Flip" & Ground Kick
			if (m_player->m_controls.m_a == 1 && m_rotation == 2 && !m_player->m_activeField->isEmpty(m_pos[1].x + 1, m_pos[1].y) && !m_player->m_activeField->isEmpty(m_pos[1].x - 1, m_pos[1].y) && !m_player->m_activeField->isEmpty(m_pos[2].x, m_pos[2].y - 1)) {
				m_rotateCounter = -8;
				m_rotation = 3;
				m_pos[0].y += 1;
				m_pos[0].x += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 3 && !m_player->m_activeField->isEmpty(m_pos[2].x + 1, m_pos[2].y) && !m_player->m_activeField->isEmpty(m_pos[2].x - 1, m_pos[2].y) && !m_player->m_activeField->isEmpty(m_pos[1].x, m_pos[1].y - 1)) {
				m_rotateCounter = 8;
				m_rotation = 2;
				m_pos[0].y += 1;
				m_pos[0].x -= 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
		if (m_type == MovePuyoType::TRIPLET) {
			// Wall kick
			// Right wall
			if (m_player->m_controls.m_a == 1 && m_rotation == 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_a += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = -8;
				m_rotation = 1;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 3 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[2].x + 1, m_pos[2].y) && m_player->m_activeField->isEmpty(m_pos[1].x, m_pos[1].y - 1)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = 8;
				m_rotation = 2;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			// Left wall
			if (m_player->m_controls.m_a == 1 && m_rotation == 2 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) && m_player->m_activeField->isEmpty(m_pos[2].x, m_pos[2].y - 1)) {
				m_player->m_controls.m_a += 1;
				m_pos[0].x += 1;
				m_rotateCounter = -8;
				m_rotation = 3;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
			if (m_player->m_controls.m_b == 1 && m_rotation == 1 && m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y)) {
				m_player->m_controls.m_b += 1;
				m_pos[0].x += 1;
				m_rotateCounter = 8;
				m_rotation = 0;
				// Play sound
				m_data->snd.rotate.play(m_data);
			}
		}
	}

	setRotation();

	// Edit timer
	if (m_rotateCounter > 0)
		m_rotateCounter -= 1;
	if (m_rotateCounter < 0)
		m_rotateCounter += 1;

	// Edit fliptimer
	if (m_flipCounter > 0)
		m_flipCounter -= 1;
	if (m_flipCounter < 0)
		m_flipCounter += 1;
	if (m_flipCounter == 0)
		m_flip = 0;

	// Rotate Puyos
	// RotateCounter is for normal rotations, FlipTimer is for fast flipping (Variable Flip determines if used or not)
	// MovepUyoAngle=Rotation( "Ctr_Variables_P1" )*90+RotateCounter( "Ctr_Timers_P1" )/8.0*90+FlipCounter( "Ctr_Timers_P1" )/8.0*180*Flip( "Ctr_Variables_P1" )
	m_movePuyoAngle = static_cast<float>(m_rotation * 90) + static_cast<float>(m_rotateCounter) / 8.0f * 90 + static_cast<float>(m_flipCounter) / 8.0f * 180 * static_cast<float>(m_flip);

	// Trigger flip
	if (m_type == MovePuyoType::DOUBLET && m_dropCounter < 90) {
		if (m_player->m_controls.m_a == 1 && m_flip == 0 && m_flipCounter <= 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			m_flipCounter = 20;
		if (m_player->m_controls.m_b == 1 && m_flip == 0 && m_flipCounter >= 0 && !m_player->m_activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) && !m_player->m_activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			m_flipCounter = -20;
	}
}

void MovePuyo::setRotation()
{
	if (m_type == MovePuyoType::DOUBLET || m_type == MovePuyoType::TRIPLET) {
		// Set variables
		if (m_rotation == 0) {
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y + 1;
		} else if (m_rotation == 1) {
			m_pos[1].x = m_pos[0].x + 1;
			m_pos[1].y = m_pos[0].y;
		} else if (m_rotation == 2) {
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y - 1;
		} else if (m_rotation == 3) {
			m_pos[1].x = m_pos[0].x - 1;
			m_pos[1].y = m_pos[0].y;
		}
	}
	if (m_type == MovePuyoType::TRIPLET) {
		// Set variables
		if (m_rotation == 1) {
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y + 1;
		} else if (m_rotation == 2) {
			m_pos[2].x = m_pos[0].x + 1;
			m_pos[2].y = m_pos[0].y;
		} else if (m_rotation == 3) {
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y - 1;
		} else if (m_rotation == 0) {
			m_pos[2].x = m_pos[0].x - 1;
			m_pos[2].y = m_pos[0].y;
		}
	}
	if (m_type == MovePuyoType::QUADRUPLET || m_type == MovePuyoType::BIG) {
		// Quadruplet and big simply stay in pos==1 state
		m_pos[1].x = m_pos[0].x + 1;
		m_pos[1].y = m_pos[0].y;
		m_pos[2].x = m_pos[0].x;
		m_pos[2].y = m_pos[0].y + 1;
		m_pos[3].x = m_pos[0].x + 1;
		m_pos[3].y = m_pos[0].y + 1;
	}
}

void MovePuyo::move()
{
	if (m_player->m_currentPhase == Phase::MOVE && m_player->getPlayerType() == ONLINE
		&& !m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'm') {
		int t = 0;
		int x1, x2, x3, x4, y1, y2, y3, y4;
		int color, rotation;
		int rotcounter, fallcounter, flipcounter;
		int scoreval, trns, down;

		// 0["m"]1[timestamp]2[posx1]3[posy1]4[posx2]5[posy2]6[posx3]7[posy3]8[posx4]9[posy4]10[bigcolor]
		// 11[rotation]12[rotecounter]13[fallcounter]14[flipcounter]15[scoreVal]16[turns]17[button down]
		// sscanf(currentMessageStr.c_str(),"m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%f|%f|%f|%i|%i|%i",
		sscanf(m_player->m_messages.front().c_str(), "m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
			&t, &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4, &color, &rotation, &rotcounter, &fallcounter, &flipcounter, &scoreval, &trns, &down);
		if (m_player->m_turns == trns) {
			setPosX1(x1);
			setPosY1(y1);
			setPosX2(x2);
			setPosY2(y2);
			setPosX3(x3);
			setPosY3(y3);
			setPosX4(x4);
			setPosY4(y4);
			setColorBig(color);
			setRotation(rotation);
			setRotateCounter(rotcounter);
			setFallCounter(fallcounter);
			setFlipCounter(flipcounter);
			m_player->m_scoreVal = scoreval;
			m_player->m_controls.m_down = down;
		}

		m_player->m_messages.pop_front();
		setSprite();
	}
	if (m_player->m_currentPhase == Phase::MOVE && m_player->getPlayerType() == ONLINE
		&& !m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'q') {
		FieldProp prop = m_player->m_activeField->getProperties();
		int x1, x2, y1, y2;
		sscanf(m_player->m_messages.front().c_str(), "q|%i|%i|%i|%i",
			&x1, &y1, &x2, &y2);

		m_pos[0].x = x1;
		m_pos[0].y = y1;
		m_pos[1].x = x2;
		m_pos[1].y = y2;
		m_posReal[0].y = prop.offsetY + m_spawnY * prop.gridHeight - m_pos[0].y * prop.gridHeight - (100 - m_fallCounter) / 100.f * prop.gridHeight + kPuyoY / 2 - (kPuyoY - prop.gridHeight);

		setRotation();
		int gridSizeX = prop.gridWidth;
		m_quick1.setPosition(static_cast<float>(m_pos[0].x * gridSizeX), m_posReal[0].y - prop.offsetY - static_cast<float>(kPuyoY / 2) + static_cast<float>(kPuyoY - prop.gridHeight));
		m_quick2.setPosition(static_cast<float>(m_pos[1].x * gridSizeX), m_posReal[1].y - prop.offsetY - static_cast<float>(kPuyoY / 2) + static_cast<float>(kPuyoY - prop.gridHeight));
		m_quick1.setTransparency(1);
		m_quick2.setTransparency(1);
		m_quick1.setSubRect(4 * kPuyoX + kPuyoX * m_color1, 13 * kPuyoY, kPuyoX, static_cast<int>(kPuyoY * 1.5));
		m_quick2.setSubRect(4 * kPuyoX + kPuyoX * m_color2, 13 * kPuyoY, kPuyoX, static_cast<int>(kPuyoY * 1.5));
		m_dropCounter = 100;
		m_holdCounter = 1;
		m_qScale = 1;

		// Drop puyo
		for (int i = 0; i < prop.gridY; i++) {
			if (!isAnyTouching(BELOW)) {
				m_pos[0].y--;
				m_pos[1].y--;
				m_pos[2].y--;
				m_pos[3].y--;
				m_qScale += 1;
			}
		}

		m_player->m_messages.pop_front();
	}

	// Set visible during move phase
	if (m_player->m_currentPhase == Phase::MOVE)
		m_visible = true;

	moveSpriteX();
	setSpriteY();
	setSpriteAngle();
	if (m_player->m_currentPhase == Phase::MOVE)
		placePuyos();
	placeShadow();

	// Fade out quick drop
	m_quick1.setTransparency(max(m_quick1.getTransparency() - 0.1f, 0.0f));
	m_quick2.setTransparency(max(m_quick2.getTransparency() - 0.1f, 0.0f));

	// Send move message
	// Create string
	// 0["m"]1[timestamp]2[posx1]3[posy1]4[posx2]5[posy2]6[posx3]7[posy3]8[posx4]9[posy4]10[bigcolor]
	// 11[rotation]12[rotecounter]13[fallcounter]14[flipcounter]15[scoreVal]16[turns]17[button down]
	// Send on keypress
	if (m_player->m_currentPhase == Phase::MOVE && m_player->m_currentGame->m_connected
		&& m_player->getPlayerType() == HUMAN && m_player->m_currentGame->m_players.size() <= 10) {
		char str[100];
		sprintf(str, "m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
			m_data->matchTimer,
			m_pos[0].x, m_pos[0].y,
			m_pos[1].x, m_pos[1].y,
			m_pos[2].x, m_pos[2].y,
			m_pos[3].x, m_pos[3].y,
			m_bigColor,
			m_rotation,
			(int)m_rotateCounter, (int)m_fallCounter, (int)m_flipCounter,
			m_player->m_scoreVal, m_player->m_turns, (int)m_player->m_controls.m_down > 0);

		// Pressing
		if (m_player->m_controls.m_a == 2 || m_player->m_controls.m_b == 2 || m_player->m_controls.m_left == 1 || m_player->m_controls.m_right == 1
			// Holding
			|| (m_player->m_controls.m_right > 8 && m_player->m_controls.m_right % 2 == 0 && m_player->m_controls.m_left == 0)
			|| (m_player->m_controls.m_left > 8 && m_player->m_controls.m_left % 2 == 0 && m_player->m_controls.m_right == 0)
			// Press or release down
			|| (m_player->m_controls.m_down == 1 && m_player->m_controls.m_delayDown == false)
			|| (m_player->m_controls.m_down == 0 && m_player->m_controls.m_delayDown == true)
			// Send on start of move phase
			|| m_initCalled) {
			m_player->m_currentGame->m_network->sendToChannel(CHANNEL_GAME, str, m_player->m_currentGame->m_channelName.c_str());
			if (m_initCalled) {
				m_initCalled = false;
			}
		}
	}
}

void MovePuyo::setType(MovePuyoType mpt)
{
	m_type = mpt;
	// Unset the other variables
	if (mpt == MovePuyoType::DOUBLET) {
		m_pos[2].x = -10;
		m_pos[2].y = -10;
		m_pos[3].x = -10;
		m_pos[3].y = -10;
	}
	if (mpt == MovePuyoType::TRIPLET) {
		m_pos[3].x = -10;
		m_pos[3].y = -10;
	}
}

// Is the puyopair obstructed in some direction?
bool MovePuyo::isAnyTouching(const Direction dir) const
{
	int hor = 0, ver = 0;
	if (dir == BELOW) {
		hor = 0;
		ver = -1;
	} else if (dir == LEFT) {
		hor = -1;
		ver = 0;
	} else if (dir == RIGHT) {
		hor = 1;
		ver = 0;
	}
	if (!m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver) || !m_player->m_activeField->isEmpty(m_pos[1].x + hor, m_pos[1].y + ver) || !m_player->m_activeField->isEmpty(m_pos[2].x + hor, m_pos[2].y + ver) || !m_player->m_activeField->isEmpty(m_pos[3].x + hor, m_pos[3].y + ver))
		return true;

	if (dir == LEFT || dir == RIGHT) {
		// Floating object
		if ((!m_player->m_activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + 1) || !m_player->m_activeField->isEmpty(m_pos[1].x + hor, m_pos[1].y + 1) || !m_player->m_activeField->isEmpty(m_pos[2].x + hor, m_pos[2].y + 1) || !m_player->m_activeField->isEmpty(m_pos[3].x + hor, m_pos[3].y + 1)) && m_fallCounter < 90)
			return true;
	}
	return false;
}

void MovePuyo::placePuyos()
{
	if (m_fallCounter > 90 && isAnyTouching(BELOW)) {
		m_dropCounter += 1;
	}
	if (m_dropCounter > 90 && m_player->m_currentGame->m_settings->recording != RecordState::REPLAYING) {
		// End phase 10
		if (m_player->m_currentPhase == Phase::MOVE && m_player->getPlayerType() != ONLINE) {
			m_player->m_turns += 1;
			m_player->endPhase();
		}
	} else if (m_player->m_currentGame->m_settings->recording == RecordState::REPLAYING) {
		// Do same thing as online
		// Receive place message
		if (!m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'p') {
			m_player->m_turns += 1;
			m_player->endPhase();
		}
	}
	// Online
	if (m_player->m_currentPhase == Phase::MOVE && m_player->getPlayerType() == ONLINE) {
		// Receive place message
		if (!m_player->m_messages.empty() && m_player->m_messages.front()[0] == 'p') {
			m_player->m_turns += 1;
			m_player->endPhase();
			// Ignore rotation and flipping
		}
		// Do not clear message yet
	}
}

// Set shadow
void MovePuyo::placeShadow()
{
	if (m_player->m_currentPhase != Phase::MOVE) {
		m_shadow1.setVisible(false);
		m_shadow2.setVisible(false);
		m_shadow3.setVisible(false);
		m_shadow4.setVisible(false);
		return;
	}
	if (m_dropCounter > 90) {
		return;
	}
	const FieldProp prop = m_player->m_activeField->getProperties();
	const auto height = static_cast<float>(prop.gridHeight);
	const auto width = static_cast<float>(prop.gridWidth);
	const int maxHeight = prop.gridY;
	bool moveUp[4];

	// Compare colors: which are on the bottom, which are on top?
	for (int k = 0; k < 4; k++) {
		moveUp[k] = false;
		for (int j = 1; j < 5; j++) {
			// Compare pos with rest
			if (m_pos[k].y > m_pos[(k + j) % 4].y && m_pos[k].x == m_pos[(k + j) % 4].x)
				moveUp[k] = true;
		}
	}

	// Project every color downwards
	for (int k = 0; k < 4; k++) {
		int i = 0;
		while (m_player->m_activeField->isEmpty(m_pos[k].x, m_pos[k].y - i) && i < maxHeight) {
			m_shadowPos[k].x = m_pos[k].x;
			m_shadowPos[k].y = m_pos[k].y - i;

			i++;
		}
		if (moveUp[k]) {
			m_shadowPos[k].y += 1;
		}
	}
	const float xCorrection = -m_pos[0].x * 2.0f + static_cast<float>(prop.gridX / 2);
	m_shadow1.setPosition(m_shadowPos[0].x * width + kPuyoX / 2 + xCorrection, (maxHeight - 4 - m_shadowPos[0].y) * height + kPuyoY / 2);
	m_shadow2.setPosition(m_shadowPos[1].x * width + kPuyoX / 2 + xCorrection, (maxHeight - 4 - m_shadowPos[1].y) * height + kPuyoY / 2);
	m_shadow3.setPosition(m_shadowPos[2].x * width + kPuyoX / 2 + xCorrection, (maxHeight - 4 - m_shadowPos[2].y) * height + kPuyoY / 2);
	m_shadow4.setPosition(m_shadowPos[3].x * width + kPuyoX / 2 + xCorrection, (maxHeight - 4 - m_shadowPos[3].y) * height + kPuyoY / 2);

	// Set colors
	int colors[4];
	colors[0] = m_color1;
	colors[1] = m_color2;
	colors[2] = m_color1;
	colors[3] = m_color2;
	if (m_type == MovePuyoType::TRIPLET && m_transpose) {
		colors[1] = m_color1;
		colors[2] = m_color2;
	} else if (m_type == MovePuyoType::QUADRUPLET) {
		if (m_rotation == 0) {
			colors[0] = m_color2;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color1;
		} else if (m_rotation == 1) {
			colors[0] = m_color2;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color1;
		} else if (m_rotation == 2) {
			colors[0] = m_color1;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color2;
		} else if (m_rotation == 3) {
			colors[0] = m_color1;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color2;
		}
	} else if (m_type == MovePuyoType::BIG) {
		colors[0] = m_bigColor;
		colors[1] = m_bigColor;
		colors[2] = m_bigColor;
		colors[3] = m_bigColor;
	}
	m_shadow1.setColor(colors[0]);
	m_shadow2.setColor(colors[1]);
	m_shadow3.setColor(colors[2]);
	m_shadow4.setColor(colors[3]);

	// Set color and visibility
	if (m_type == MovePuyoType::DOUBLET) {
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(false);
		m_shadow4.setVisible(false);
	} else if (m_type == MovePuyoType::TRIPLET) {
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(true);
		m_shadow4.setVisible(false);
	} else if (m_type == MovePuyoType::QUADRUPLET || m_type == MovePuyoType::BIG) {
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(true);
		m_shadow4.setVisible(true);
	}

	// Trigger glow
	int n = 2;
	if (m_type == MovePuyoType::TRIPLET || m_type == MovePuyoType::TRIPLET_R) {
		n = 3;
		m_shadowPos[3].x = -1;
		m_shadowPos[3].y = -1;
	} else if (m_type == MovePuyoType::QUADRUPLET || m_type == MovePuyoType::BIG) {
		n = 4;
	} else {
		m_shadowPos[2].x = -1;
		m_shadowPos[2].y = -1;
		m_shadowPos[3].x = -1;
		m_shadowPos[3].y = -1;
	}
	if (m_player->getPlayerType() != ONLINE) {
		m_player->m_activeField->triggerGlow(m_shadowPos, n, colors);
	}
}

// Draw move puyo on screen
void MovePuyo::draw()
{
	drawQuick();

	if (!m_visible) {
		return;
	}

	FieldProp prop = m_player->m_activeField->getProperties();

	// Now using sfml's renderimages. Note: many variables have become useless
	// Set position
	float posX1 = (m_posReal[0].x - prop.offsetX);
	float posY1 = (m_posReal[0].y - prop.offsetY);
	float posX2 = (m_posReal[1].x - prop.offsetX);
	float posY2 = (m_posReal[1].y - prop.offsetY);

	float posX_Qcor = 0, posY_Qcor = 0;
	if (m_type == MovePuyoType::QUADRUPLET) {
		// Set correction factor for quadruplet puyo
		posX_Qcor = static_cast<float>(kPuyoX / 2);
		posY_Qcor = static_cast<float>(-kPuyoY / 2) + static_cast<float>(kPuyoY) * 0.18f;
	}

	float xcorrection = -m_pos[0].x * 2.f + static_cast<float>(prop.gridX / 2);

	// Eye position
	if (m_type != MovePuyoType::QUADRUPLET) {
		m_posXEye1Real = posX1 + kPuyoY * sin(m_sprite1Angle * kPiF / 180);
		m_posYEye1Real = posY1 - kPuyoY * cos(m_sprite1Angle * kPiF / 180);
		m_posXEye2Real = posX1 + kPuyoY * sin(m_sprite2Angle * kPiF / 180);
		m_posYEye2Real = posY1 - kPuyoY * cos(m_sprite2Angle * kPiF / 180);
	} else {
		m_posXEye1Real = posX1 + kPuyoY * 0.4f * sin((m_sprite1Angle - 30) * kPiF / 180);
		m_posYEye1Real = posY1 - kPuyoY * 0.4f * cos((m_sprite1Angle - 30) * kPiF / 180);
		m_posXEye2Real = posX1 + kPuyoY * 0.4f * sin((m_sprite2Angle - 30) * kPiF / 180);
		m_posYEye2Real = posY1 - kPuyoY * 0.4f * cos((m_sprite2Angle - 30) * kPiF / 180);
	}

	m_sprite1.setPosition(posX1 + xcorrection + posX_Qcor, posY1 + posY_Qcor); // Added correction factor because width is 31, not 32
	m_sprite2.setPosition(posX2 + xcorrection + posX_Qcor, posY2 + posY_Qcor);
	m_spriteEye1.setPosition(m_posXEye1Real + xcorrection + posX_Qcor, m_posYEye1Real + posY_Qcor);
	m_spriteEye2.setPosition(m_posXEye2Real + xcorrection + posX_Qcor, m_posYEye2Real + posY_Qcor);

	// Set rotation of sprite
	m_sprite1.setRotation(-m_sprite1Angle);
	m_sprite2.setRotation(-m_sprite2Angle);

	m_shadow1.draw(m_data->front);
	m_shadow2.draw(m_data->front);
	m_shadow3.draw(m_data->front);
	m_shadow4.draw(m_data->front);

	if (m_type == MovePuyoType::DOUBLET) {
		m_sprite1.draw(m_data->front);
		m_sprite2.draw(m_data->front);

	} else if (m_type == MovePuyoType::TRIPLET) {
		if (m_color1 != m_color2) {
			m_sprite1.draw(m_data->front);
			m_sprite2.draw(m_data->front);
			m_spriteEye1.draw(m_data->front);
		} else {
			m_sprite1.draw(m_data->front);
			m_spriteEye1.draw(m_data->front);
		}
	} else if (m_type == MovePuyoType::QUADRUPLET) {
		m_sprite1.draw(m_data->front);
		m_sprite2.draw(m_data->front);
		m_spriteEye1.draw(m_data->front);
		m_spriteEye2.draw(m_data->front);

	} else if (m_type == MovePuyoType::BIG) {
		m_sprite1.draw(m_data->front);
	}
}

// Draw quickdrop
void MovePuyo::drawQuick()
{
	// Get scale from field
	const FieldProp prop = m_player->m_activeField->getProperties();

	const float scaleX = prop.scaleX;
	const float scaleY = prop.scaleY;

	const float corr = prop.gridHeight / (kPuyoY * 1.0f);

	m_quick1.setScaleX(scaleX);
	m_quick1.setScaleY(scaleY * m_qScale * corr / 1.5f);
	m_quick2.setScaleX(scaleX);
	m_quick2.setScaleY(scaleY * m_qScale * corr / 1.5f);
	m_quick1.draw(m_data->front);
	m_quick2.draw(m_data->front);
}

void MovePuyo::hotReload()
{
	m_sprite1.setImage(m_data->imgPuyo);
	m_sprite2.setImage(m_data->imgPuyo);
	m_spriteEye1.setImage(m_data->imgPuyo);
	m_spriteEye2.setImage(m_data->imgPuyo);

	// Set sprite
	m_sprite1.setSubRect(0, kPuyoY * m_color1, kPuyoX, kPuyoY);
	m_sprite2.setSubRect(0, kPuyoY * m_color2, kPuyoX, kPuyoY);
	m_spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * (m_color1 % 2), 12 * kPuyoY + kPuyoY * ((m_color1 / 2) % 2), kPuyoX, kPuyoY);
	m_spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * (m_color2 % 2), 12 * kPuyoY + kPuyoY * ((m_color2 / 2) % 2), kPuyoX, kPuyoY);
	m_sprite1.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_sprite2.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_spriteEye1.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_spriteEye2.setCenter(kPuyoX / 2, kPuyoY / 2);
	m_quick1.setImage(m_data->imgPuyo);
	m_quick2.setImage(m_data->imgPuyo);
	m_quick1.setSubRect(4 * kPuyoX, 13 * kPuyoY, kPuyoX, kPuyoY);
	m_quick2.setSubRect(4 * kPuyoX, 13 * kPuyoY, kPuyoX, kPuyoY);
	m_quick1.setTransparency(0);
	m_quick2.setTransparency(0);

	m_shadow1.hotReload();
	m_shadow2.hotReload();
	m_shadow3.hotReload();
	m_shadow4.hotReload();

}

}
