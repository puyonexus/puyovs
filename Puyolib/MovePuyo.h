#pragma once

#include "DropPattern.h"
#include "FieldProp.h"
#include "Sprite.h"
#include "global.h"

namespace ppvs {

class Player;

class Shadow {
public:
	Shadow();
	~Shadow();

	Shadow(const Shadow&) = delete;
	Shadow& operator=(const Shadow&) = delete;
	Shadow(Shadow&&) = delete;
	Shadow& operator=(Shadow&&) = delete;

	GameData* m_data = nullptr;
	void init(GameData* data);
	void draw(FeRenderTarget* target);
	void setPosition(const float xx, const float yy) { m_sprite.setPosition(xx, yy); }
	void setColor(const int i)
	{
		m_color = i;
		setSprite();
	}
	void setVisible(const bool b) { m_sprite.setVisible(b); }
	void hotReload();

private:
	void setSprite();
	Sprite m_sprite;
	int m_color = 0;
	bool m_init = false;
};

class MovePuyo {
	bool m_visible;
	Player* m_player;
	MovePuyoType m_type;
	int m_spawnX, m_spawnY;
	int m_color1, m_color2;
	int m_bigColor;
	Shadow m_shadow1, m_shadow2, m_shadow3, m_shadow4;
	PosVectorInt m_shadowPos[4] {};
	PosVectorInt m_pos[4] {};
	PosVectorFloat m_posReal[2] {};
	float m_posXEye1Real, m_posYEye1Real, m_posXEye2Real, m_posYEye2Real;
	unsigned int m_rotation;
	float m_movePuyoAngle;
	float m_sprite1Angle, m_sprite2Angle;
	int m_flip;
	bool m_transpose;
	int m_fallCounter, m_rotateCounter, m_flipCounter;
	float m_dropCounter, m_holdCounter;
	Sprite m_sprite1;
	Sprite m_sprite2;
	Sprite m_spriteEye1;
	Sprite m_spriteEye2;
	Sprite m_quick1;
	Sprite m_quick2;
	float m_qScale;

	float m_subRectX1, m_subRectY1, m_subRectWidth1, m_subRectHeight1;
	float m_subRectX2, m_subRectY2, m_subRectWidth2, m_subRectHeight2;
	bool m_tripletRot;
	bool m_initCalled;

public:
	MovePuyo();
	~MovePuyo();

	MovePuyo(const MovePuyo&) = delete;
	MovePuyo& operator=(const MovePuyo&) = delete;
	MovePuyo(MovePuyo&&) = delete;
	MovePuyo& operator=(MovePuyo&&) = delete;

	GameData* m_data;
	void init(GameData* data);

	// Asking values
    [[nodiscard]] int getColor1() const { return m_color1; }
    [[nodiscard]] int getColor2() const { return m_color2; }
    [[nodiscard]] int getColorBig() const { return m_bigColor; }
    [[nodiscard]] int getPosX1() const { return m_pos[0].x; }
    [[nodiscard]] int getPosY1() const { return m_pos[0].y; }
    [[nodiscard]] int getPosX2() const { return m_pos[1].x; }
    [[nodiscard]] int getPosY2() const { return m_pos[1].y; }
    [[nodiscard]] int getPosX3() const { return m_pos[2].x; }
    [[nodiscard]] int getPosY3() const { return m_pos[2].y; }
    [[nodiscard]] int getPosX4() const { return m_pos[3].x; }
    [[nodiscard]] int getPosY4() const { return m_pos[3].y; }
    [[nodiscard]] bool getTranspose() const { return m_transpose; }
    [[nodiscard]] int getRotateCounter() const { return m_rotateCounter; }
    [[nodiscard]] int getFlipCounter() const { return m_flipCounter; }
    [[nodiscard]] int getFallCounter() const { return m_fallCounter; }
    [[nodiscard]] MovePuyoType getType() const { return m_type; }
    [[nodiscard]] unsigned int getRotation() const { return m_rotation; }
    [[nodiscard]] float getDropCounter() const { return m_dropCounter; }
	PosVectorInt getSpawnPoint();
	void setVisible(bool);

	// Set
	void setColorBig(const int i) { m_bigColor = i; }
	void setPosX1(const int i) { m_pos[0].x = i; }
	void setPosY1(const int i) { m_pos[0].y = i; }
	void setPosX2(const int i) { m_pos[1].x = i; }
	void setPosY2(const int i) { m_pos[1].y = i; }
	void setPosX3(const int i) { m_pos[2].x = i; }
	void setPosY3(const int i) { m_pos[2].y = i; }
	void setPosX4(const int i) { m_pos[3].x = i; }
	void setPosY4(const int i) { m_pos[3].y = i; }
	void setRotation(const int i) { m_rotation = i; }
	void setRotateCounter(const int f) { m_rotateCounter = f; }
	void setFlipCounter(const int f) { m_flipCounter = f; }
	void setFallCounter(const int f) { m_fallCounter = f; }

	// Related to puyo movement
	void prepare(MovePuyoType type, Player* player, int color1, int color2); // Phase 0
	void move();
	void setSprite(); // Phase 10
	void moveSpriteX(); // Phase 10
	void setSpriteX(); // Phase 10
	void setSpriteY(); // Phase 10
	void setSpriteAngle(); // Phase 10
	void setRotation(); // Phase 10
	void setType(MovePuyoType);
	bool isAnyTouching(Direction) const;
	void placePuyos(); // Phase 10 end
	void placeShadow();

	// Draw
	void draw();
	void drawQuick();
	void hotReload();
};

}
