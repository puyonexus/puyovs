#include <algorithm>
#include "movePuyo.h"
#include "player.h"
#include "game.h"
#include "../PVS_ENet/PVS_Client.h"

using namespace std;

namespace ppvs
{

shadow::shadow()
{
	m_init = false;
}

shadow::~shadow() = default;

void shadow::init(gameData * gamedata)
{
	if (m_init)
		return;
	m_init = true;
	data = gamedata;
	m_sprite.setImage((data->imgPuyo));
	m_color = 0;
	m_sprite.setVisible(false);
	setSprite();

}

void shadow::setSprite()
{
	m_sprite.setSubRect(16 * PUYOX - PUYOX / 2 * (m_color / 3 + 1), 7 * PUYOY + PUYOY / 2 * (m_color % 3), PUYOX / 2, PUYOY / 2);
	m_sprite.setCenter(PUYOX / 4, PUYOY / 4);
}

void shadow::draw(frendertarget * target)
{
	m_sprite.draw(target);
}

/************************
*Movepuyo implementation
************************/

movePuyo::movePuyo()
{
	m_pos[0].x = 0; m_pos[0].y = 0;
	m_pos[1].x = 0; m_pos[1].y = 0;
	m_pos[2].x = 0; m_pos[2].y = 0;
	m_pos[3].x = 0; m_pos[3].y = 0;
	initCalled = false;
	data = nullptr;

	m_visible = false;
	m_player = nullptr;
	m_type = DOUBLET;
	m_spawnX = 0; m_spawnY = 0;
	m_color1 = 0; m_color2 = 0;
	m_bigColor = 0;
	m_shadowPos[0].x = 0; m_shadowPos[0].y = 0;
	m_shadowPos[1].x = 0; m_shadowPos[1].y = 0;
	m_shadowPos[2].x = 0; m_shadowPos[2].y = 0;
	m_shadowPos[3].x = 0; m_shadowPos[3].y = 0;
	m_posReal[0].x = 0; m_posReal[0].y = 0;
	m_posReal[1].x = 0; m_posReal[1].y = 0;
	m_posXEye1real = 0; m_posYEye1real = 0; m_posXEye2real = 0; m_posYEye2real = 0;
	m_rotation = 0;
	m_movePuyoAngle = 0;
	m_sprite1Angle = 0; m_sprite2Angle = 0;
	m_flip = 0;
	m_transpose = false;
	m_fallCounter = 0; m_rotateCounter = 0; m_flipCounter = 0;
	m_dropCounter = 0; m_holdCounter = 0;
	subRectX1 = 0; subRectY1 = 0; subRectWidth1 = 0; subRectHeight1 = 0;
	subRectX2 = 0; subRectY2 = 0; subRectWidth2 = 0; subRectHeight2 = 0;
	triplet_rot = false;
	m_qscale = 1;
}

movePuyo::~movePuyo()
{
	//dtor
}

void movePuyo::init(gameData * globalp)
{
	data = globalp;
	//set sprites
	m_sprite1.setImage(data->imgPuyo);
	m_sprite2.setImage(data->imgPuyo);
	m_spriteEye1.setImage(data->imgPuyo);
	m_spriteEye2.setImage(data->imgPuyo);
	m_visible = false;

	//Set sprite
	m_sprite1.setSubRect(0, PUYOY * m_color1, PUYOX, PUYOY);
	m_sprite2.setSubRect(0, PUYOY * m_color2, PUYOX, PUYOY);
	m_spriteEye1.setSubRect(PUYOX + 2 * PUYOX * (m_color1 % 2), 12 * PUYOY + PUYOY * ((m_color1 / 2) % 2), PUYOX, PUYOY);
	m_spriteEye2.setSubRect(PUYOX + 2 * PUYOX * (m_color2 % 2), 12 * PUYOY + PUYOY * ((m_color2 / 2) % 2), PUYOX, PUYOY);
	m_sprite1.setCenter(PUYOX / 2, PUYOY / 2);
	m_sprite2.setCenter(PUYOX / 2, PUYOY / 2);
	m_spriteEye1.setCenter(PUYOX / 2, PUYOY / 2);
	m_spriteEye2.setCenter(PUYOX / 2, PUYOY / 2);
	m_quick1.setImage(data->imgPuyo);
	m_quick2.setImage(data->imgPuyo);
	m_quick1.setSubRect(4 * PUYOX, 13 * PUYOY, PUYOX, PUYOY);
	m_quick2.setSubRect(4 * PUYOX, 13 * PUYOY, PUYOX, PUYOY);
	m_quick1.setTransparency(0);
	m_quick2.setTransparency(0);

	//Call init to initialize default values
	m_type = DOUBLET;
	m_pos[0].x = 0; m_pos[0].y = 0; m_pos[1].x = 0; m_pos[1].y = 0;
	m_pos[2].x = 0; m_pos[2].y = 0; m_pos[3].x = 0; m_pos[3].y = 0;
	m_rotation = 0; m_rotateCounter = 0;
	m_spawnX = 0; m_spawnY = 0;
	m_color1 = 0; m_color2 = 0; m_bigColor = 0;
	m_posReal[0].x = 0; m_posReal[0].y = 0; m_posReal[1].x = 0; m_posReal[1].y = 0;
	m_movePuyoAngle = 0; m_sprite1Angle = 0; m_sprite2Angle = 0;
	m_flip = 0; m_fallCounter = 0; m_dropCounter = 0; m_holdCounter = 0; m_rotateCounter = 0; m_flipCounter = 0;
	m_qscale = 1;
}

posVectorInt movePuyo::getSpawnPoint()
{
	posVectorInt spawn;
	spawn.x = m_spawnX;
	spawn.y = m_spawnY;
	return spawn;
}

void movePuyo::setVisible(bool b)
{
	m_visible = b;
}

void movePuyo::prepare(movePuyoType mpt, player * thisplayer, int color1, int color2)
{//Get movepuyo ready to be dropped
	m_player = thisplayer;
	m_shadow1.data = data; m_shadow1.init(thisplayer->data);
	m_shadow2.data = data; m_shadow2.init(thisplayer->data);
	m_shadow3.data = data; m_shadow3.init(thisplayer->data);
	m_shadow4.data = data; m_shadow4.init(thisplayer->data);
	//triplet and quadruplet start in rotation=1 mode
	if (mpt == DOUBLET || mpt == BIG)
		m_rotation = 0;
	else if (mpt == TRIPLET || mpt == TRIPLETR || mpt == QUADRUPLET)
		m_rotation = 1;
	//set to triplet if tripletR
	if (mpt == TRIPLETR)
	{
		setType(TRIPLET);
		m_transpose = true;
	}
	else
	{
		setType(mpt);
		m_transpose = false;
	}
	//TEMPORARY (pass this through paramaters?)
	//define a spawn point (x index) and top of the field
	fieldProp prop = m_player->activeField->getProperties();
	m_spawnX = (prop.gridX - 1) / 2;
	m_spawnY = prop.gridY - 4;
	//reset values
	m_movePuyoAngle = 0;
	m_dropCounter = 0;
	m_fallCounter = 0;
	m_flipCounter = 0;
	m_rotateCounter = 0;
	//set color
	m_color1 = color1;
	m_color2 = color2;
	m_bigColor = color1;
	setSprite();
	//set spawn point
	m_pos[0].x = m_spawnX;
	m_pos[0].y = m_spawnY;

	//set initial position of doublet
	setSpriteX();
	setSpriteY();

	initCalled = true;
}

// Set sprites of movePuyos
void movePuyo::setSprite()
{
	if (m_type == DOUBLET)
	{
		subRectX1 = 0; subRectY1 = static_cast<float>(PUYOY * m_color1);
		subRectWidth1 = static_cast<float>(PUYOX); subRectHeight1 = static_cast<float>(PUYOY);
		subRectX2 = 0; subRectY2 = static_cast<float>(PUYOY * m_color2);
		subRectWidth2 = static_cast<float>(PUYOX); subRectHeight2 = static_cast<float>(PUYOY);
		m_sprite1.setSubRect(
			static_cast<int>(subRectX1),
			static_cast<int>(subRectY1),
			static_cast<int>(subRectWidth1), 
			static_cast<int>(subRectHeight1)
		);
		m_sprite2.setSubRect(
			static_cast<int>(subRectX2),
			static_cast<int>(subRectY2),
			static_cast<int>(subRectWidth2),
			static_cast<int>(subRectHeight2)
		);
		m_sprite1.setCenter(PUYOX / 2, PUYOY / 2);
		m_sprite2.setCenter(PUYOX / 2, PUYOY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setVisible(false);
		m_spriteEye2.setVisible(false);
	}
	else if (m_type == TRIPLET && m_color1 != m_color2)
	{
		subRectX1 = static_cast<float>(PUYOX * m_color1); subRectY1 = static_cast<float>(PUYOY * 5 + 1);
		subRectWidth1 = static_cast<float>(PUYOX); subRectHeight1 = static_cast<float>(PUYOY * 2 - 1);
		subRectX2 = 0.f; subRectY2 = static_cast<float>(PUYOY * m_color2);
		subRectWidth2 = static_cast<float>(PUYOX); subRectHeight2 = static_cast<float>(PUYOY);
		m_sprite1.setSubRect(
			static_cast<int>(subRectX1),
			static_cast<int>(subRectY1),
			static_cast<int>(subRectWidth1),
			static_cast<int>(subRectHeight1)
		);
		m_sprite2.setSubRect(
			static_cast<int>(subRectX2),
			static_cast<int>(subRectY2),
			static_cast<int>(subRectWidth2),
			static_cast<int>(subRectHeight2)
		);
		m_sprite1.setCenter(PUYOX / 2, PUYOY + PUYOY / 2);
		m_sprite2.setCenter(PUYOX / 2, PUYOY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setSubRect(PUYOX + 2 * PUYOX * ((m_color1 / 2)), 12 * PUYOY + PUYOY * (m_color1 % 2), PUYOX, PUYOY);
		m_spriteEye2.setSubRect(PUYOX + 2 * PUYOX * ((m_color2 / 2)), 12 * PUYOY + PUYOY * (m_color2 % 2), PUYOX, PUYOY);
		m_spriteEye1.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye2.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(false);
	}
	else if (m_type == TRIPLET && m_color1 == m_color2)
	{
		subRectX1 = static_cast<float>(5 * PUYOX + 2 * PUYOX * m_color1); subRectY1 = static_cast<float>(PUYOY * 5 + 1);
		subRectWidth1 = static_cast<float>(PUYOX * 2); subRectHeight1 = static_cast<float>(PUYOY * 2 - 1);
		subRectX2 = 0.f; subRectY2 = static_cast<float>(PUYOY * m_color2);
		subRectWidth2 = static_cast<float>(PUYOX); subRectHeight2 = static_cast<float>(PUYOY);
		m_sprite1.setSubRect(
			static_cast<int>(subRectX1),
			static_cast<int>(subRectY1),
			static_cast<int>(subRectWidth1),
			static_cast<int>(subRectHeight1)
		);
		m_sprite2.setSubRect(
			static_cast<int>(subRectX2),
			static_cast<int>(subRectY2),
			static_cast<int>(subRectWidth2),
			static_cast<int>(subRectHeight2)
		);
		m_sprite1.setCenter(PUYOX / 2, PUYOY + PUYOY / 2);
		m_sprite2.setCenter(PUYOX / 2, PUYOY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(false);
		m_spriteEye1.setSubRect(PUYOX + 2 * PUYOX * ((m_color1 / 2)), 12 * PUYOY + PUYOY * (m_color1 % 2), PUYOX, PUYOY);
		m_spriteEye2.setSubRect(PUYOX + 2 * PUYOX * ((m_color2 / 2)), 12 * PUYOY + PUYOY * (m_color2 % 2), PUYOX, PUYOY);
		m_spriteEye1.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye2.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(false);
	}
	else if (m_type == QUADRUPLET)
	{
		subRectX1 = 10.f * static_cast<float>(PUYOX) + (static_cast<float>(PUYOX) + static_cast<float>(PUYOX) / 6.4f) * m_color1; subRectY1 = static_cast<float>(PUYOY) * 13.f;
		subRectWidth1 = static_cast<float>(PUYOX) + static_cast<float>(PUYOX) / 6.4f; subRectHeight1 = static_cast<float>(PUYOY) * 2.f;
		subRectX2 = 10.f * static_cast<float>(PUYOX) + (static_cast<float>(PUYOX) + static_cast<float>(PUYOX) / 6.4f) * m_color2; subRectY2 = static_cast<float>(PUYOY) * 13.f;
		subRectWidth2 = static_cast<float>(PUYOX) + static_cast<float>(PUYOX) / 6.4f; subRectHeight2 = static_cast<float>(PUYOY) * 2.f;
		m_sprite1.setSubRect(
			static_cast<int>(subRectX1),
			static_cast<int>(subRectY1),
			static_cast<int>(subRectWidth1),
			static_cast<int>(subRectHeight1)
		);
		m_sprite2.setSubRect(
			static_cast<int>(subRectX2),
			static_cast<int>(subRectY2),
			static_cast<int>(subRectWidth2),
			static_cast<int>(subRectHeight2)
		);
		m_sprite1.setCenter(data->PUYOXCENTER, PUYOY);
		m_sprite2.setCenter(data->PUYOXCENTER, PUYOY);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(true);
		m_spriteEye1.setSubRect(PUYOX + 2 * PUYOX * ((m_color1 / 2)), 12 * PUYOY + PUYOY * (m_color1 % 2), PUYOX, PUYOY);
		m_spriteEye2.setSubRect(PUYOX + 2 * PUYOX * ((m_color2 / 2)), 12 * PUYOY + PUYOY * (m_color2 % 2), PUYOX, PUYOY);
		m_spriteEye1.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye2.setCenter(PUYOX / 2, PUYOY / 2);
		m_spriteEye1.setVisible(true);
		m_spriteEye2.setVisible(true);
	}
	else if (m_type == BIG)
	{
		subRectX1 = static_cast<float>(2 * PUYOX + 2 * PUYOX * m_bigColor); subRectY1 = static_cast<float>(PUYOY * 7);
		subRectWidth1 = static_cast<float>(PUYOX * 2); subRectHeight1 = static_cast<float>(PUYOY * 2);
		subRectX2 = 0; subRectY2 = 0;
		subRectWidth2 = 0; subRectHeight2 = 0;
		m_sprite1.setSubRect(
			static_cast<int>(subRectX1),
			static_cast<int>(subRectY1),
			static_cast<int>(subRectWidth1),
			static_cast<int>(subRectHeight1)
		);
		m_sprite2.setSubRect(
			static_cast<int>(subRectX2),
			static_cast<int>(subRectY2),
			static_cast<int>(subRectWidth2),
			static_cast<int>(subRectHeight2)
		);
		m_sprite1.setCenter(PUYOX / 2, PUYOY + PUYOY / 2);
		m_sprite2.setCenter(PUYOX / 2, PUYOY + PUYOY / 2);
		m_sprite1.setVisible(true);
		m_sprite2.setVisible(false);
		m_spriteEye1.setVisible(false);
		m_spriteEye2.setVisible(false);
	}
}

void movePuyo::moveSpriteX()
{//MOVES sprite in the x direction
	int offsetX = static_cast<int>(m_player->activeField->getProperties().offsetX);
	int gridSizeX = m_player->activeField->getProperties().gridWidth;

	//Actual movement
	if (m_posReal[0].x < offsetX + m_pos[0].x * gridSizeX + PUYOX / 2)
		m_posReal[0].x += float(gridSizeX / 2.f);
	else if (m_posReal[0].x > offsetX + m_pos[0].x * gridSizeX + PUYOX / 2)
		m_posReal[0].x -= (float)gridSizeX / 2.f;

	//Snap
	if (m_posReal[0].x<offsetX + m_pos[0].x * gridSizeX + PUYOX / 2 && m_posReal[0].x>offsetX + m_pos[0].x * gridSizeX + PUYOX / 2 - gridSizeX / 2.f)
		m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + PUYOX / 2);
	else if (m_posReal[0].x > offsetX + m_pos[0].x * gridSizeX + PUYOX / 2 && m_posReal[0].x < offsetX + m_pos[0].x * gridSizeX + PUYOX / 2 + gridSizeX / 2.f)
		m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + PUYOX / 2);

	//Controller movement
	if (m_player->currentphase == MOVE)
	{
		//if (m_type==DOUBLET)
		{
			//debugstring=to_string(m_pos[0].x+1);
			if (m_player->controls.Right == 1 && !isAnyTouching(RIGHT))
			{
				m_pos[0].x += 1;
				data->snd.move.Play(data);
			}
			if (m_player->controls.Left == 1 && !isAnyTouching(LEFT))
			{
				m_pos[0].x -= 1;
				data->snd.move.Play(data);
			}
			if (m_player->controls.Right > 8 && m_player->controls.Right % 2 == 0 && m_player->controls.Left == 0 && !isAnyTouching(RIGHT))
			{
				m_pos[0].x += 1;
				data->snd.move.Play(data);
			}
			if (m_player->controls.Left > 8 && m_player->controls.Left % 2 == 0 && m_player->controls.Right == 0 && !isAnyTouching(LEFT))
			{
				m_pos[0].x -= 1;
				data->snd.move.Play(data);
			}
		}
		setRotation();
	}
}

void movePuyo::setSpriteX()
{//sets sprite in the x direction
	int offsetX = static_cast<int>(m_player->activeField->getProperties().offsetX);
	int gridSizeX = m_player->activeField->getProperties().gridWidth;
	m_posReal[0].x = static_cast<float>(offsetX + m_pos[0].x * gridSizeX + PUYOX / 2);

}

void movePuyo::setSpriteY()
{//sets sprite in the y direction
	fieldProp prop = m_player->activeField->getProperties();

	//MovePuyosY
	//Move puyos down: relative to field
	if (m_player->currentphase == MOVE)
	{
		//there is now a complication that the origin is now the center of the puyo -> problem when PUYOY and gridsize are unequal
		//so we must correct it by +PUYOY/2-(PUYOY-gridSizeY)
		m_posReal[0].y = prop.offsetY + m_spawnY * prop.gridHeight - m_pos[0].y * prop.gridHeight - (100 - m_fallCounter) / 100.f * prop.gridHeight + PUYOY / 2 - (PUYOY - prop.gridHeight);

		if (m_player->controls.Up == 1 && m_player->currentgame->currentruleset->quickDrop)
		{
			if (m_player->currentgame->connected && m_player->getPlayerType() == HUMAN)
			{
				//send
				//q|posx1|posy1|posx2|posy2
				char str[100];
				sprintf(str, "q|%i|%i|%i|%i",
					m_pos[0].x, m_pos[0].y,
					m_pos[1].x, m_pos[1].y);
				m_player->currentgame->network->sendToChannel(CHANNEL_GAME, str, m_player->currentgame->channelName.c_str());
			}
			setRotation();
			int gridSizeX = prop.gridWidth;
			m_quick1.setPosition(m_pos[0].x * static_cast<float>(gridSizeX), m_posReal[0].y - prop.offsetY - PUYOY / 2 + (PUYOY - prop.gridHeight));
			m_quick2.setPosition(m_pos[1].x * static_cast<float>(gridSizeX), m_posReal[1].y - prop.offsetY - PUYOY / 2 + (PUYOY - prop.gridHeight));
			//m_quick1.setPosition(m_pos[0].x*gridSizeX,m_posReal[0].y-prop.offsetY);
			//m_quick2.setPosition(m_pos[1].x*gridSizeX,m_posReal[0].y-prop.offsetY);
			m_quick1.setTransparency(1);
			m_quick2.setTransparency(1);
			m_quick1.setSubRect(4 * PUYOX + PUYOX * m_color1, 13 * PUYOY, PUYOX, static_cast<int>(static_cast<float>(PUYOY) * 1.5f));
			m_quick2.setSubRect(4 * PUYOX + PUYOX * m_color2, 13 * PUYOY, PUYOX, static_cast<int>(static_cast<float>(PUYOY) * 1.5f));
			m_dropCounter = 100;
			m_holdCounter = 1;
			m_qscale = 1;
			//drop puyo
			for (int i = 0; i < prop.gridY; i++)
			{
				if (!isAnyTouching(BELOW))
				{
					m_pos[0].y--;
					m_pos[1].y--;
					m_pos[2].y--;
					m_pos[3].y--;
					m_qscale += 1;
				}
			}
		}

		//add to fallcounter
		if (m_fallCounter < 100)
			m_fallCounter += static_cast<int>(m_player->dropspeed);

		//add score here
		if (m_player->controls.Down > 0 &&
			m_fallCounter >= 100 &&
			!isAnyTouching(BELOW))
		{
			m_player->scoreVal += 1;
			m_player->dropBonus += 1;
		}

		//reset fallcounter
		if (m_fallCounter >= 100 && !isAnyTouching(BELOW))
		{
			m_fallCounter -= 100;
			m_pos[0].y -= 1;
			m_pos[1].y -= 1;
		}
	}

	//Controller movement
	if (m_player->currentphase == MOVE)
	{
		//Move puyo down
		if (m_player->controls.Down > 0 &&
			m_player->controls.Left == 0 &&
			m_player->controls.Right == 0 &&
			m_fallCounter < 100)
			m_fallCounter += m_player->currentgame->currentruleset->fastDrop;
		if (m_player->controls.Down > 0 &&
			m_player->controls.Left == 0 &&
			m_player->controls.Right > 0 &&
			isAnyTouching(RIGHT) &&
			m_fallCounter < 100)
			m_fallCounter += m_player->currentgame->currentruleset->fastDrop;
		if (m_player->controls.Down > 0 &&
			m_player->controls.Left > 0 &&
			m_player->controls.Right == 0 &&
			isAnyTouching(LEFT) &&
			m_fallCounter < 100)
			m_fallCounter += m_player->currentgame->currentruleset->fastDrop;

		//Last bit
		if (m_player->controls.Down > 0 &&
			m_fallCounter >= 100 &&
			isAnyTouching(BELOW) &&
			m_holdCounter > 0)
			m_dropCounter = 100;

		if (m_player->controls.Down > 0 &&
			m_fallCounter >= 100 &&
			isAnyTouching(BELOW))
		{
			m_fallCounter = 100;
			m_player->scoreVal += 1;
			m_player->dropBonus += 1;
			m_holdCounter = 1;
		}
	}

	//Holdcounter
	if (m_player->controls.Down == 0)
		m_holdCounter = 0;

	if (m_player->currentphase != MOVE)
	{
		m_holdCounter = 0;
	}
}

void movePuyo::setSpriteAngle()
{//sets sprite and puyo angle
	//int offsetX=m_player->activeField->getProperties().offsetX;
	int gridSizeX = m_player->activeField->getProperties().gridWidth;
	int gridSizeY = m_player->activeField->getProperties().gridHeight;
	//int offsetY=m_player->activeField->getProperties().offsetY;

	//Actual movement
	if (m_type == DOUBLET)
	{
		//Doublet does not rotate puyo sprites
		m_sprite1Angle = 0;
		m_sprite2Angle = 0;
		//Doublet: puyo2 rotates around puyo1
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin(m_movePuyoAngle * PI / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos(m_movePuyoAngle * PI / 180);
	}
	if (m_type == TRIPLET && (m_transpose == false || m_color1 == m_color2))
	{
		//sprite angle
		m_sprite1Angle = m_movePuyoAngle - 90;
		m_sprite2Angle = 0;
		//sprite pos
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin(m_movePuyoAngle * PI / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos(m_movePuyoAngle * PI / 180);
	}
	if (m_type == TRIPLET && m_color1 != m_color2 && m_transpose == true)
	{
		//sprite angle
		m_sprite1Angle = m_movePuyoAngle;
		m_sprite2Angle = 0;
		//sprite pos
		m_posReal[1].x = m_posReal[0].x + gridSizeX * sin((m_movePuyoAngle - 90) * PI / 180);
		m_posReal[1].y = m_posReal[0].y - gridSizeY * cos((m_movePuyoAngle - 90) * PI / 180);
	}
	if (m_type == QUADRUPLET)
	{
		//Quadruplet: the two pieces stay in place and rotate around the center
		//sprite angle
		m_sprite1Angle = m_movePuyoAngle + 90;
		m_sprite2Angle = m_movePuyoAngle + 180 + 90;
		//sprite pos
		m_posReal[1].x = m_posReal[0].x;
		m_posReal[1].y = m_posReal[0].y;
	}
	if (m_type == BIG)
	{
		m_sprite1Angle = 0;
		m_sprite2Angle = 0;
	}

	//Controller movement
	if (m_player->currentphase == MOVE)
	{
		//(This movement is incomplete for nazo puyo: it doesn't check for collisions upwards)
		if (m_type == DOUBLET)
		{ //ROTATION
			//Clockwise
			int hor = 0, ver = 0;
			switch (m_rotation)
			{
			case 0: hor = 1; ver = 0; break;
			case 1: hor = 0; ver = -1; break;
			case 2: hor = -1; ver = 0; break;
			case 3: hor = 0; ver = 1; break;
			}
			if (m_player->controls.A == 1 && m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver))
			{
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->controls.A += 1;
				//play sound
				data->snd.rotate.Play(data);
			}

			//Counter Clockwise
			switch (m_rotation)
			{
			case 0: hor = -1; ver = 0; break;
			case 1: hor = 0; ver = 1; break;
			case 2: hor = 1; ver = 0; break;
			case 3: hor = 0; ver = -1; break;
			}
			if (m_player->controls.B == 1 && m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver))
			{
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->controls.B += 1;
				//play sound
				data->snd.rotate.Play(data);
			}
		}

		else if (m_type == TRIPLET)
		{ //ROTATION
			//Clockwise
			int hor = 0, ver = 0;
			switch (m_rotation)
			{
			case 0: hor = 1; ver = 0; break;
			case 1: hor = 0; ver = -1; break;
			case 2: hor = -1; ver = 0; break;
			case 3: hor = 0; ver = 1; break;
			}
			if (m_player->controls.A == 1 && m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver))
			{
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->controls.A += 1;
				//play sound
				data->snd.rotate.Play(data);
			}

			//Counter Clockwise
			switch (m_rotation)
			{
			case 1: hor = -1; ver = 0; break;
			case 2: hor = 0; ver = 1; break;
			case 3: hor = 1; ver = 0; break;
			case 0: hor = 0; ver = -1; break;
			}
			if (m_player->controls.B == 1 && m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver))
			{
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->controls.B += 1;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		else if (m_type == QUADRUPLET)
		{
			if (m_player->controls.A == 1)
			{
				m_rotation++;
				m_rotation %= 4;
				m_rotateCounter = -8;
				m_player->controls.A += 1;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1)
			{
				m_rotation--;
				m_rotation %= 4;
				m_rotateCounter = 8;
				m_player->controls.B += 1;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		else if (m_type == BIG)
		{
			if (m_player->controls.A == 1)
			{
				m_bigColor++;
				m_bigColor %= m_player->colors;
				setSprite();
				m_player->controls.A += 1;
			}
			if (m_player->controls.B == 1)
			{
				m_bigColor--;
				if (m_bigColor < 0)
					m_bigColor += m_player->colors;
				m_bigColor %= m_player->colors;
				setSprite();
				m_player->controls.B += 1;

			}
		}

		if (m_type == DOUBLET)
		{ //7Rule
			if (m_player->controls.A == 1 &&
				m_rotation == 0 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y + 1) &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_pos[0].y += 1;
				m_rotation = 1;
				m_rotateCounter = -8;
				m_player->controls.A += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 0 &&
				m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y + 1) &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_pos[0].y += 1;
				m_rotation = 3;
				m_rotateCounter = 8;
				m_player->controls.B += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == TRIPLET)
		{ //7Rule
			if (m_player->controls.A == 1 &&
				m_rotation == 0 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y + 1))
			{
				m_pos[0].y += 1;
				m_rotation = 1;
				m_rotateCounter = -8;
				m_player->controls.A += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 1 &&
				m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y + 1))
			{
				m_pos[0].y += 1;
				m_rotation = 0;
				m_rotateCounter = 8;
				m_player->controls.B += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}

		if (m_type == DOUBLET)
		{ //Wall kick
			//right wall
			if (m_player->controls.A == 1 &&
				m_rotation == 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.A += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = -8;
				m_rotation = 1;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 2 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.B += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = 8;
				m_rotation = 1;
				//play sound
				data->snd.rotate.Play(data);
			}
			//left wall
			if (m_player->controls.A == 1 &&
				m_rotation == 2 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.A += 1;
				m_pos[0].x += 1;
				m_rotateCounter = -8;
				m_rotation = 3;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 0 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.B += 1;
				m_pos[0].x += 1;
				m_rotateCounter = 8;
				m_rotation = 3;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == DOUBLET || m_type == TRIPLET)
		{ //Ground kick
			if (m_player->controls.A == 1 &&
				m_rotation == 1 &&
				m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y + 1) &&
				!m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_player->controls.A += 1;
				m_pos[0].y += 1;
				m_rotateCounter = -8;
				m_rotation = 2;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 3 &&
				m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y + 1) &&
				!m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_player->controls.B += 1;
				m_pos[0].y += 1;
				m_rotateCounter = 8;
				m_rotation = 2;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == TRIPLET)
		{//Ground kick
			if (m_player->controls.B == 1 &&
				m_rotation == 0 &&
				m_player->activeField->isEmpty(m_pos[2].x, m_pos[2].y + 1) &&
				!m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_player->controls.B += 1;
				m_pos[0].y += 1;
				m_rotateCounter = 8;
				m_rotation = 3;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == DOUBLET)
		{ //Flip
			if (m_player->controls.A == 1 &&
				m_flip == 0 &&
				m_flipCounter > 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_flipCounter = -8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_flip == 0 &&
				m_flipCounter < 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_flipCounter = 8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == DOUBLET)
		{ //Flip & Ground Kick
			if (m_player->controls.A == 1 &&
				m_flip == 0 &&
				m_rotation == 0 &&
				m_flipCounter > 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_flipCounter = -8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				m_pos[0].y += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_flip == 0 &&
				m_rotation == 0 &&
				m_flipCounter < 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x, m_pos[0].y - 1))
			{
				m_flipCounter = 8;
				m_flip = 1;
				m_rotation = (int)abs(int(m_rotation - 2));
				m_pos[0].y += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == TRIPLET)
		{ //"Flip" & Ground Kick
			if (m_player->controls.A == 1 &&
				m_rotation == 2 &&
				!m_player->activeField->isEmpty(m_pos[1].x + 1, m_pos[1].y) &&
				!m_player->activeField->isEmpty(m_pos[1].x - 1, m_pos[1].y) &&
				!m_player->activeField->isEmpty(m_pos[2].x, m_pos[2].y - 1))
			{
				m_rotateCounter = -8;
				m_rotation = 3;
				m_pos[0].y += 1;
				m_pos[0].x += 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 3 &&
				!m_player->activeField->isEmpty(m_pos[2].x + 1, m_pos[2].y) &&
				!m_player->activeField->isEmpty(m_pos[2].x - 1, m_pos[2].y) &&
				!m_player->activeField->isEmpty(m_pos[1].x, m_pos[1].y - 1))
			{
				m_rotateCounter = 8;
				m_rotation = 2;
				m_pos[0].y += 1;
				m_pos[0].x -= 1;
				m_fallCounter = 60;
				m_dropCounter += 10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		if (m_type == TRIPLET)
		{ //Wall kick
			//right wall
			if (m_player->controls.A == 1 &&
				m_rotation == 0 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.A += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = -8;
				m_rotation = 1;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 3 &&
				!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[2].x + 1, m_pos[2].y) &&
				m_player->activeField->isEmpty(m_pos[1].x, m_pos[1].y - 1))
			{
				m_player->controls.B += 1;
				m_pos[0].x -= 1;
				m_rotateCounter = 8;
				m_rotation = 2;
				//play sound
				data->snd.rotate.Play(data);
			}
			//left wall
			if (m_player->controls.A == 1 &&
				m_rotation == 2 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[2].x, m_pos[2].y - 1))
			{
				m_player->controls.A += 1;
				m_pos[0].x += 1;
				m_rotateCounter = -8;
				m_rotation = 3;
				//play sound
				data->snd.rotate.Play(data);
			}
			if (m_player->controls.B == 1 &&
				m_rotation == 1 &&
				m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			{
				m_player->controls.B += 1;
				m_pos[0].x += 1;
				m_rotateCounter = 8;
				m_rotation = 0;
				//play sound
				data->snd.rotate.Play(data);
			}
		}
		/*
		if (m_type==DOUBLET)
		{ //Ground & wall kick
			if (m_player->controls.A==1 &&
				m_rotation==1 &&
				m_player->activeField->isEmpty(m_pos[0].x+1,m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[0].x-1,m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[2].x,m_pos[2].y-1))
			{
				m_player->controls.A+=1;
				m_pos[0].x+=1;
				m_rotateCounter=-8;
				m_rotation=2;
				m_pos[0].y+=1;
				m_fallCounter=60;
				m_dropCounter+=10;
				//play sound
				data->snd.rotate.Play(data);
			}

			if (m_player->controls.B==1 &&
				m_rotation==2 &&
				!m_player->activeField->isEmpty(m_pos[0].x+1,m_pos[0].y) &&
				m_player->activeField->isEmpty(m_pos[0].x-1,m_pos[0].y) &&
				!m_player->activeField->isEmpty(m_pos[2].x,m_pos[2].y-1))
			{
				m_player->controls.B+=1;
				m_pos[0].x-=1;
				m_rotateCounter=8;
				m_rotation=1;
				m_pos[0].y+=1;
				m_fallCounter=60;
				m_dropCounter+=10;
				//play sound
				data->snd.rotate.Play(data);
			}
		}*/
	}

	setRotation();

	//edit timer
	if (m_rotateCounter > 0)
		m_rotateCounter -= 1;
	if (m_rotateCounter < 0)
		m_rotateCounter += 1;

	//edit fliptimer
	if (m_flipCounter > 0)
		m_flipCounter -= 1;
	if (m_flipCounter < 0)
		m_flipCounter += 1;
	if (m_flipCounter == 0)
		m_flip = 0;

	//Rotate Puyos
	//RotateCounter is for normal rotations, FlipTimer is for fast flipping (Variable Flip determines if used or not)
	//MovepUyoAngle=Rotation( "Ctr_Variables_P1" )*90+RotateCounter( "Ctr_Timers_P1" )/8.0*90+FlipCounter( "Ctr_Timers_P1" )/8.0*180*Flip( "Ctr_Variables_P1" )
	m_movePuyoAngle = static_cast<float>(m_rotation * 90) + static_cast<float>(m_rotateCounter) / 8.0f * 90 + static_cast<float>(m_flipCounter) / 8.0f * 180 * static_cast<float>(m_flip);
	
	//Trigger flip
	if (m_type == DOUBLET && m_dropCounter < 90)
	{
		if (m_player->controls.A == 1 &&
			m_flip == 0 &&
			m_flipCounter <= 0 &&
			!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
			!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			m_flipCounter = 20;
		if (m_player->controls.B == 1 &&
			m_flip == 0 &&
			m_flipCounter >= 0 &&
			!m_player->activeField->isEmpty(m_pos[0].x + 1, m_pos[0].y) &&
			!m_player->activeField->isEmpty(m_pos[0].x - 1, m_pos[0].y))
			m_flipCounter = -20;
	}
}

void movePuyo::setRotation()
{
	if (m_type == DOUBLET || m_type == TRIPLET)
	{
		//Set variables
		if (m_rotation == 0)
		{
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y + 1;
		}
		else if (m_rotation == 1)
		{
			m_pos[1].x = m_pos[0].x + 1;
			m_pos[1].y = m_pos[0].y;
		}
		else if (m_rotation == 2)
		{
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y - 1;
		}
		else if (m_rotation == 3)
		{
			m_pos[1].x = m_pos[0].x - 1;
			m_pos[1].y = m_pos[0].y;
		}
	}
	if (m_type == TRIPLET)
	{
		//Set variables
		if (m_rotation == 1)
		{
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y + 1;
		}
		else if (m_rotation == 2)
		{
			m_pos[2].x = m_pos[0].x + 1;
			m_pos[2].y = m_pos[0].y;
		}
		else if (m_rotation == 3)
		{
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y - 1;
		}
		else if (m_rotation == 0)
		{
			m_pos[2].x = m_pos[0].x - 1;
			m_pos[2].y = m_pos[0].y;
		}

	}
	if (m_type == QUADRUPLET || m_type == BIG)
	{//quadruplet and big simply stay in pos==1 state
		m_pos[1].x = m_pos[0].x + 1;
		m_pos[1].y = m_pos[0].y;
		m_pos[2].x = m_pos[0].x;
		m_pos[2].y = m_pos[0].y + 1;
		m_pos[3].x = m_pos[0].x + 1;
		m_pos[3].y = m_pos[0].y + 1;
	}
}

void movePuyo::move()
{
	if (m_player->currentphase == MOVE && m_player->getPlayerType() == ONLINE
		&& !m_player->messages.empty() && m_player->messages.front()[0] == 'm')
	{
		int t = 0;
		int x1, x2, x3, x4, y1, y2, y3, y4;
		int color, rotation;
		int rotcounter, fallcounter, flipcounter;
		int scoreval, trns, down;

		//0["m"]1[timestamp]2[posx1]3[posy1]4[posx2]5[posy2]6[posx3]7[posy3]8[posx4]9[posy4]10[bigcolor]
		//11[rotation]12[rotecounter]13[fallcounter]14[flipcounter]15[scoreVal]16[turns]17[button down]
		//sscanf(currentMessageStr.c_str(),"m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%f|%f|%f|%i|%i|%i",
		sscanf(m_player->messages.front().c_str(), "m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
			&t
			, &x1, &y1
			, &x2, &y2
			, &x3, &y3
			, &x4, &y4
			, &color, &rotation
			, &rotcounter, &fallcounter, &flipcounter
			, &scoreval, &trns, &down);
		if (m_player->turns == trns)
		{
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
			m_player->scoreVal = scoreval;
			m_player->controls.Down = down;
		}

		//m_player->currentMessageStr="";
		m_player->messages.pop_front();
		setSprite();
	}
	if (m_player->currentphase == MOVE && m_player->getPlayerType() == ONLINE
		&& !m_player->messages.empty() && m_player->messages.front()[0] == 'q')
	{
		fieldProp prop = m_player->activeField->getProperties();
		int x1, x2, y1, y2;
		sscanf(m_player->messages.front().c_str(), "q|%i|%i|%i|%i",
			&x1, &y1
			, &x2, &y2);

		m_pos[0].x = x1;
		m_pos[0].y = y1;
		m_pos[1].x = x2;
		m_pos[1].y = y2;
		m_posReal[0].y = prop.offsetY + m_spawnY * prop.gridHeight - m_pos[0].y * prop.gridHeight - (100 - m_fallCounter) / 100.f * prop.gridHeight + PUYOY / 2 - (PUYOY - prop.gridHeight);

		setRotation();
		int gridSizeX = prop.gridWidth;
		m_quick1.setPosition(static_cast<float>(m_pos[0].x * gridSizeX), m_posReal[0].y - prop.offsetY - static_cast<float>(PUYOY / 2) + static_cast<float>(PUYOY - prop.gridHeight));
		m_quick2.setPosition(static_cast<float>(m_pos[1].x * gridSizeX), m_posReal[1].y - prop.offsetY - static_cast<float>(PUYOY / 2) + static_cast<float>(PUYOY - prop.gridHeight));
		//m_quick1.setPosition(m_pos[0].x*gridSizeX,m_posReal[0].y-prop.offsetY);
		//m_quick2.setPosition(m_pos[1].x*gridSizeX,m_posReal[0].y-prop.offsetY);
		m_quick1.setTransparency(1);
		m_quick2.setTransparency(1);
		m_quick1.setSubRect(4 * PUYOX + PUYOX * m_color1, 13 * PUYOY, PUYOX, static_cast<int>(PUYOY * 1.5));
		m_quick2.setSubRect(4 * PUYOX + PUYOX * m_color2, 13 * PUYOY, PUYOX, static_cast<int>(PUYOY * 1.5));
		m_dropCounter = 100;
		m_holdCounter = 1;
		m_qscale = 1;
		//drop puyo
		for (int i = 0; i < prop.gridY; i++)
		{
			if (!isAnyTouching(BELOW))
			{
				m_pos[0].y--;
				m_pos[1].y--;
				m_pos[2].y--;
				m_pos[3].y--;
				m_qscale += 1;
			}
		}

		m_player->messages.pop_front();
	}

	//Set visible during move phase
	if (m_player->currentphase == MOVE)
		m_visible = true;

	moveSpriteX();
	setSpriteY();
	setSpriteAngle();
	if (m_player->currentphase == MOVE)
		placePuyos();
	placeShadow();

	//fade out quick drop
	m_quick1.setTransparency(max(m_quick1.getTransparency() - 0.1f, 0.0f));
	m_quick2.setTransparency(max(m_quick2.getTransparency() - 0.1f, 0.0f));

	//send move message
	/*
	"m;"+Str$(GameTimer( "Ctr_Constants" ))
	+";"+Str$(PosX1( "Ctr_Variables_P1" ))+";"+
	Str$(PosY1( "Ctr_Variables_P1" ))+";"
	+Str$(Rotation( "Ctr_Variables_P1" ))+";"+
	Str$(PosX2( "Ctr_Variables_P1" ))
	+";"+Str$(PosY2( "Ctr_Variables_P1" ))
	+";"+Str$(FallCounter( "Ctr_Timers_P1" ))
	+";"+Str$(ScoreVal( "Ctr_Variables_P1" ))
	+";"+Str$(RotateCounter( "Ctr_Timers_P1" ))
	+";"+Str$(Turns( "Ctr_Variables_P1" ))
	+";"+Str$(Button_Down( "Control-Man_P1" ))
	*/

	//create string
	//0["m"]1[timestamp]2[posx1]3[posy1]4[posx2]5[posy2]6[posx3]7[posy3]8[posx4]9[posy4]10[bigcolor]
	//11[rotation]12[rotecounter]13[fallcounter]14[flipcounter]15[scoreVal]16[turns]17[button down]
	//send on keypress
	if (m_player->currentphase == MOVE && m_player->currentgame->connected
		&& m_player->getPlayerType() == HUMAN && m_player->currentgame->players.size() <= 10)
	{
		char str[100];
		sprintf(str, "m|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|%i",
			data->matchTimer,
			m_pos[0].x, m_pos[0].y,
			m_pos[1].x, m_pos[1].y,
			m_pos[2].x, m_pos[2].y,
			m_pos[3].x, m_pos[3].y,
			m_bigColor,
			m_rotation,
			(int)m_rotateCounter, (int)m_fallCounter, (int)m_flipCounter,
			m_player->scoreVal, m_player->turns, (int)m_player->controls.Down > 0);

		//pressing
		if (m_player->controls.A == 2 || m_player->controls.B == 2 || m_player->controls.Left == 1 || m_player->controls.Right == 1
			//holding
			|| (m_player->controls.Right > 8 && m_player->controls.Right % 2 == 0 && m_player->controls.Left == 0)
			|| (m_player->controls.Left > 8 && m_player->controls.Left % 2 == 0 && m_player->controls.Right == 0)
			//press or release down
			|| (m_player->controls.Down == 1 && m_player->controls.DelayDown == false)
			|| (m_player->controls.Down == 0 && m_player->controls.DelayDown == true)
			//send on start of move phase
			|| initCalled
			)
		{
			m_player->currentgame->network->sendToChannel(CHANNEL_GAME, str, m_player->currentgame->channelName.c_str());
			if (initCalled)
				initCalled = false;
		}
	}

}

void movePuyo::setType(movePuyoType mpt)
{
	m_type = mpt;
	//Unset the other variables
	if (mpt == DOUBLET)
	{
		m_pos[2].x = -10;
		m_pos[2].y = -10;
		m_pos[3].x = -10;
		m_pos[3].y = -10;
	}
	if (mpt == TRIPLET)
	{
		m_pos[3].x = -10;
		m_pos[3].y = -10;
	}
}

bool movePuyo::isAnyTouching(direction dir)
{//is the puyopair obstructed in some direction?
	int hor, ver;
	if (dir == BELOW)
	{
		hor = 0;
		ver = -1;
	}
	else if (dir == LEFT)
	{
		hor = -1;
		ver = 0;
	}
	else if (dir == RIGHT)
	{
		hor = 1;
		ver = 0;
	}
	if (!m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + ver) ||
		!m_player->activeField->isEmpty(m_pos[1].x + hor, m_pos[1].y + ver) ||
		!m_player->activeField->isEmpty(m_pos[2].x + hor, m_pos[2].y + ver) ||
		!m_player->activeField->isEmpty(m_pos[3].x + hor, m_pos[3].y + ver))
		return true;

	if (dir == LEFT || dir == RIGHT)
	{//floating object
		if ((!m_player->activeField->isEmpty(m_pos[0].x + hor, m_pos[0].y + 1) ||
			!m_player->activeField->isEmpty(m_pos[1].x + hor, m_pos[1].y + 1) ||
			!m_player->activeField->isEmpty(m_pos[2].x + hor, m_pos[2].y + 1) ||
			!m_player->activeField->isEmpty(m_pos[3].x + hor, m_pos[3].y + 1)) && m_fallCounter < 90)
			return true;
	}
	return false;
}

void movePuyo::placePuyos()
{
	if (m_fallCounter > 90 &&
		isAnyTouching(BELOW))
		m_dropCounter += 1;
	if (m_dropCounter > 90 && m_player->currentgame->settings->recording != PVS_REPLAYING)
	{
		//End phase 10
		if (m_player->currentphase == MOVE && m_player->getPlayerType() != ONLINE)
		{
			m_player->turns += 1;
			m_player->endPhase();
		}
	}
	else if (m_player->currentgame->settings->recording == PVS_REPLAYING)
	{
		//do same thing as online
		//receive place message
		if (!m_player->messages.empty() && m_player->messages.front()[0] == 'p')
		{
			m_player->turns += 1;
			m_player->endPhase();
		}
	}
	//online
	if (m_player->currentphase == MOVE && m_player->getPlayerType() == ONLINE)
	{
		//receive place message
		if (!m_player->messages.empty() && m_player->messages.front()[0] == 'p')
		{
			m_player->turns += 1;
			m_player->endPhase();
			//ignore rotation and flipping
			//m_rotateCounter=0;
			//m_flipCounter=0;
		}
		//do not clear message yet
	}

}

void movePuyo::placeShadow()
{//Set shadow
	if (m_player->currentphase != MOVE)
	{
		m_shadow1.setVisible(false);
		m_shadow2.setVisible(false);
		m_shadow3.setVisible(false);
		m_shadow4.setVisible(false);
		return;
	}
	else if (m_dropCounter > 90)
		return;
	//projection downwards
	int i = 0;
	fieldProp prop = m_player->activeField->getProperties();
	float height = static_cast<float>(prop.gridHeight);
	float width = static_cast<float>(prop.gridWidth);
	int maxheight = prop.gridY;
	bool moveUp[4];

	//Compare colors: which are on the bottom, which are on top?
	for (int k = 0; k < 4; k++)
	{
		moveUp[k] = false;
		for (int j = 1; j < 5; j++)
		{//compare pos with rest
			if (m_pos[k].y > m_pos[(k + j) % 4].y && m_pos[k].x == m_pos[(k + j) % 4].x)
				moveUp[k] = true;
		}
	}

	//project every color downwards
	for (int k = 0; k < 4; k++)
	{
		i = 0;
		while (m_player->activeField->isEmpty(m_pos[k].x, m_pos[k].y - i) && i < maxheight)
		{
			m_shadowPos[k].x = m_pos[k].x;
			m_shadowPos[k].y = m_pos[k].y - i;

			i++;
		}
		if (moveUp[k])
			m_shadowPos[k].y += 1;
	}
	float xcorrection = -m_pos[0].x * 2.0f + static_cast<float>(prop.gridX / 2);
	m_shadow1.setPosition(m_shadowPos[0].x * width + PUYOX / 2 + xcorrection, (maxheight - 4 - m_shadowPos[0].y) * height + PUYOY / 2);
	m_shadow2.setPosition(m_shadowPos[1].x * width + PUYOX / 2 + xcorrection, (maxheight - 4 - m_shadowPos[1].y) * height + PUYOY / 2);
	m_shadow3.setPosition(m_shadowPos[2].x * width + PUYOX / 2 + xcorrection, (maxheight - 4 - m_shadowPos[2].y) * height + PUYOY / 2);
	m_shadow4.setPosition(m_shadowPos[3].x * width + PUYOX / 2 + xcorrection, (maxheight - 4 - m_shadowPos[3].y) * height + PUYOY / 2);

	/*if (m_type==TRIPLET && m_transpose)
	{
		m_shadow2.setPosition(m_shadowPos[2].x*width+PUYOX/2,(maxheight-4-m_shadowPos[2].y)*height+PUYOY/2);
		m_shadow3.setPosition(m_shadowPos[1].x*width+PUYOX/2,(maxheight-4-m_shadowPos[1].y)*height+PUYOY/2);
	}*/

	//set colors
	int colors[4];
	colors[0] = m_color1;
	colors[1] = m_color2;
	colors[2] = m_color1;
	colors[3] = m_color2;
	if (m_type == TRIPLET && m_transpose)
	{
		colors[1] = m_color1;
		colors[2] = m_color2;
	}
	else if (m_type == QUADRUPLET)
	{
		if (m_rotation == 0)
		{
			colors[0] = m_color2;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color1;
		}
		else if (m_rotation == 1)
		{
			colors[0] = m_color2;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color1;
		}
		else if (m_rotation == 2)
		{
			colors[0] = m_color1;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color2;
		}
		else if (m_rotation == 3)
		{
			colors[0] = m_color1;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color2;
		}
	}
	else if (m_type == BIG)
	{
		colors[0] = m_bigColor;
		colors[1] = m_bigColor;
		colors[2] = m_bigColor;
		colors[3] = m_bigColor;
	}
	m_shadow1.setColor(colors[0]);
	m_shadow2.setColor(colors[1]);
	m_shadow3.setColor(colors[2]);
	m_shadow4.setColor(colors[3]);


	//set color and visibility
	if (m_type == DOUBLET)
	{
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(false);
		m_shadow4.setVisible(false);
	}
	else if (m_type == TRIPLET)
	{
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(true);
		m_shadow4.setVisible(false);
	}
	else if (m_type == QUADRUPLET || m_type == BIG)
	{
		m_shadow1.setVisible(true);
		m_shadow2.setVisible(true);
		m_shadow3.setVisible(true);
		m_shadow4.setVisible(true);
	}

	//trigger glow
	int n = 2;
	if (m_type == TRIPLET || m_type == TRIPLETR)
	{
		n = 3;
		m_shadowPos[3].x = -1; m_shadowPos[3].y = -1;
	}
	else if (m_type == QUADRUPLET || m_type == BIG)
	{
		n = 4;
	}
	else
	{
		m_shadowPos[2].x = -1; m_shadowPos[2].y = -1;
		m_shadowPos[3].x = -1; m_shadowPos[3].y = -1;
	}
	if (m_player->getPlayerType() != ONLINE)
		m_player->activeField->triggerGlow(m_shadowPos, n, colors);
}

void movePuyo::draw()
{//Draw movepuyo on screen
	drawQuick();

	if (!m_visible)
		return;

	fieldProp prop = m_player->activeField->getProperties();

	//Now using sfml's renderimages. Note: many variables have become useless
	//set position
	float posX1 = (m_posReal[0].x - prop.offsetX);
	float posY1 = (m_posReal[0].y - prop.offsetY);
	float posX2 = (m_posReal[1].x - prop.offsetX);
	float posY2 = (m_posReal[1].y - prop.offsetY);

	float posX_Qcor = 0, posY_Qcor = 0;
	if (m_type == QUADRUPLET)
	{//set correction factor for quadruplet puyo
		posX_Qcor = static_cast<float>(PUYOX / 2);
		posY_Qcor = static_cast<float>(-PUYOY / 2) + static_cast<float>(PUYOY) * 0.18f;
	}

	float xcorrection = -m_pos[0].x * 2.f + static_cast<float>(prop.gridX / 2);

	//Eye position
	if (m_type != QUADRUPLET)
	{
		m_posXEye1real = posX1 + PUYOY * sin(m_sprite1Angle * PI / 180);
		m_posYEye1real = posY1 - PUYOY * cos(m_sprite1Angle * PI / 180);
		m_posXEye2real = posX1 + PUYOY * sin(m_sprite2Angle * PI / 180);
		m_posYEye2real = posY1 - PUYOY * cos(m_sprite2Angle * PI / 180);
	}
	else
	{
		m_posXEye1real = posX1 + PUYOY * 0.4f * sin((m_sprite1Angle - 30) * PI / 180);
		m_posYEye1real = posY1 - PUYOY * 0.4f * cos((m_sprite1Angle - 30) * PI / 180);
		m_posXEye2real = posX1 + PUYOY * 0.4f * sin((m_sprite2Angle - 30) * PI / 180);
		m_posYEye2real = posY1 - PUYOY * 0.4f * cos((m_sprite2Angle - 30) * PI / 180);
	}

	m_sprite1.setPosition(posX1 + xcorrection + posX_Qcor, posY1 + posY_Qcor); //Added correction factor because width is 31, not 32
	m_sprite2.setPosition(posX2 + xcorrection + posX_Qcor, posY2 + posY_Qcor);
	m_spriteEye1.setPosition(m_posXEye1real + xcorrection + posX_Qcor, m_posYEye1real + posY_Qcor);
	m_spriteEye2.setPosition(m_posXEye2real + xcorrection + posX_Qcor, m_posYEye2real + posY_Qcor);

	//Set rotation of sprite
	m_sprite1.setRotation(-m_sprite1Angle);
	m_sprite2.setRotation(-m_sprite2Angle);


	m_shadow1.draw(data->front);
	m_shadow2.draw(data->front);
	m_shadow3.draw(data->front);
	m_shadow4.draw(data->front);

	if (m_type == DOUBLET)
	{
		m_sprite1.draw(data->front);
		m_sprite2.draw(data->front);
		//m_sprite1.draw();
		//m_sprite2.draw();

	}
	else if (m_type == TRIPLET)
	{
		if (m_color1 != m_color2)
		{
			m_sprite1.draw(data->front);
			m_sprite2.draw(data->front);
			m_spriteEye1.draw(data->front);
		}
		else
		{
			m_sprite1.draw(data->front);
			m_spriteEye1.draw(data->front);
		}
	}
	else if (m_type == QUADRUPLET)
	{
		m_sprite1.draw(data->front);
		m_sprite2.draw(data->front);
		m_spriteEye1.draw(data->front);
		m_spriteEye2.draw(data->front);

	}
	else if (m_type == BIG)
	{
		m_sprite1.draw(data->front);
	}
}

void movePuyo::drawQuick()
{//Draw quickdrop
	//get scale from field
	fieldProp prop = m_player->activeField->getProperties();

	float scaleX = prop.scaleX;
	float scaleY = prop.scaleY;

	float corr = prop.gridHeight / (PUYOY * 1.0f);

	m_quick1.setScaleX(scaleX);
	m_quick1.setScaleY(scaleY * m_qscale * corr / 1.5f);
	m_quick2.setScaleX(scaleX);
	m_quick2.setScaleY(scaleY * m_qscale * corr / 1.5f);
	m_quick1.draw(data->front);
	m_quick2.draw(data->front);
}

}
