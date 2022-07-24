#pragma once

#include "global.h"
#include "fieldProp.h"
#include "sprite.h"
#include "dropPattern.h"
#include <math.h>
namespace ppvs
{

class player;

class shadow
{
public:
	shadow();
	~shadow();
	gameData* data;
	void init(gameData* data);
	void draw(frendertarget* target);
	void setPosition(float xx, float yy) { m_sprite.setPosition(xx, yy); }
	void setColor(int i) { m_color = i; setSprite(); }
	void setVisible(bool b) { m_sprite.setVisible(b); }
private:
	void setSprite();
	sprite m_sprite;
	int m_color;
	bool m_init;
};

class movePuyo
{
private:
	bool m_visible;
	player* m_player;
	movePuyoType m_type;
	int m_spawnX, m_spawnY;
	int m_color1, m_color2;
	int m_bigColor;
	shadow m_shadow1, m_shadow2, m_shadow3, m_shadow4;
	posVectorInt m_shadowPos[4];
	posVectorInt m_pos[4];
	posVectorFloat m_posReal[2];
	float m_posXEye1real, m_posYEye1real, m_posXEye2real, m_posYEye2real;
	unsigned int m_rotation;
	float m_movePuyoAngle;
	float m_sprite1Angle, m_sprite2Angle;
	int m_flip;
	bool m_transpose;
	int m_fallCounter, m_rotateCounter, m_flipCounter;
	float m_dropCounter, m_holdCounter;
	sprite m_sprite1;
	sprite m_sprite2;
	sprite m_spriteEye1;
	sprite m_spriteEye2;
	sprite m_quick1;
	sprite m_quick2;
	float m_qscale;

	float subRectX1, subRectY1, subRectWidth1, subRectHeight1;
	float subRectX2, subRectY2, subRectWidth2, subRectHeight2;
	bool triplet_rot;
	bool initCalled;

public:
	movePuyo();
	~movePuyo();
	gameData* data;
	void init(gameData* globalp);
	// Asking values
	int getColor1() { return m_color1; }
	int getColor2() { return m_color2; }
	int getColorBig() { return m_bigColor; }
	int getPosX1() { return m_pos[0].x; }
	int getPosY1() { return m_pos[0].y; }
	int getPosX2() { return m_pos[1].x; }
	int getPosY2() { return m_pos[1].y; }
	int getPosX3() { return m_pos[2].x; }
	int getPosY3() { return m_pos[2].y; }
	int getPosX4() { return m_pos[3].x; }
	int getPosY4() { return m_pos[3].y; }
	bool getTranspose() { return m_transpose; }
	int getRotateCounter() { return m_rotateCounter; }
	int getFlipCounter() { return m_flipCounter; }
	int getFallCounter() { return m_fallCounter; }
	movePuyoType getType() { return m_type; }
	int getRotation() { return m_rotation; }
	float getdropc() { return m_dropCounter; }
	posVectorInt getSpawnPoint();
	void setVisible(bool);

	// Set
	void setColorBig(int i) { m_bigColor = i; }
	void setPosX1(int i) { m_pos[0].x = i; }
	void setPosY1(int i) { m_pos[0].y = i; }
	void setPosX2(int i) { m_pos[1].x = i; }
	void setPosY2(int i) { m_pos[1].y = i; }
	void setPosX3(int i) { m_pos[2].x = i; }
	void setPosY3(int i) { m_pos[2].y = i; }
	void setPosX4(int i) { m_pos[3].x = i; }
	void setPosY4(int i) { m_pos[3].y = i; }
	void setRotation(int i) { m_rotation = i; }
	void setRotateCounter(int f) { m_rotateCounter = f; }
	void setFlipCounter(int f) { m_flipCounter = f; }
	void setFallCounter(int f) { m_fallCounter = f; }



	// Related to puyo movement
	void prepare(movePuyoType type, player* player, int color1, int color2); // Phase 0
	void move();
	void setSprite(); // Phase 10
	void moveSpriteX(); // Phase 10
	void setSpriteX(); // Phase 10
	void setSpriteY(); // Phase 10
	void setSpriteAngle(); // Phase 10
	void setRotation(); // Phase 10
	void setType(movePuyoType);
	bool isAnyTouching(direction);
	void placePuyos(); // Phase 10 end
	void placeShadow();

	// Draw
	void draw();
	void drawQuick();
};

}
