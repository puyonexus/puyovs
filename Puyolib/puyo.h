#pragma once

#include "global.h"
#include "fieldProp.h"
#include "sprite.h"
#include <math.h>

namespace ppvs
{
class field;

// puyo should be a base class to derive the following classes:
// coloredPuyo, nuisancePuyo, and possibly other ones like for example: pointPuyos, iceCubes, bombPuyo, starPuyo, etc.

enum puyoType
{
	NOPUYO,
	COLORPUYO,
	NUISANCEPUYO,
	HARDPUYO
};

class puyo
{
public:
	puyo(int indexX, int indexY, int color, field* f, float spriteX, float spriteY, gameData* globalp);
	puyo(const puyo& self); // Copy constructor
	virtual ~puyo();
	virtual puyo* clone();

	// Get and set
	int GetindexX()const { return m_posX; } // Returns indexed position
	void SetindexX(int val) { m_posX = val; }
	int GetindexY()const { return m_posY; }
	void SetindexY(int val) { m_posY = val; }
	float GetSpriteX() { return m_spriteX; } // Returns global position
	void SetSpriteX(float val) { m_spriteX = val; }
	float GetSpriteY() { return m_spriteY; }
	void SetSpriteY(float val) { m_spriteY = val; }
	float GetScaleX() { return m_scaleX; }
	void SetScaleX(float val) { m_scaleX = val; }
	float GetScaleY() { return m_scaleY; }
	void SetScaleY(float val) { m_scaleY = val; }
	float GetTargetY() { return m_targetY; }
	void SetTargetY(float val) { m_targetY = val; }
	float GetAccelY() { return m_accelY; }
	void SetAccelY(float val) { m_accelY = val; }
	void AddAccelY(float val);
	void setFallDelay(int val) { fallDelay = static_cast<float>(val); }
	puyoType getType()const { return m_type; }

	// Virtual functions
	virtual void draw(frendertarget* target);
	virtual int getColor()const;
	virtual void pop();
	virtual void landProper();
	virtual bool destroyPuyo();
	virtual void neighbourPop(field* f, bool v);
	virtual void updateSprite();

	// Set animation
	void setLink(direction);
	void unsetLink(direction);
	void bounce();
	void setFallTarget(int target);

	// Public variables
	gameData* data;
	float fallDelay;
	int fallFlag;
	bool bounceFlag0; // Flag 0
	int bounceFlag, searchBounce;
	float bounceTimer;
	float bounceMultiplier;
	float bounceY;
	int bottomY;
	bool glow;
	bool hard;
	bool dropable;
	bool lastNuisance;
	bool mark;
	bool destroy;
	bool temporary;

protected:
	int m_posX, m_posY; // Position in the field
	float m_spriteX, m_spriteY, m_spriteYsc; // Position on the screen (MMF legacy)
	float m_scaleX, m_scaleY, m_scaleYcor; // Standard scaling
	float m_scaleXd, m_scaleYd; // Scaling factor for destruction
	float m_accelY, m_targetY; // Acceleration factor
	puyoType m_type;
	int m_color;
	bool m_linkDown, m_linkUp, m_linkRight, m_linkLeft;
	field* m_field;
	sprite m_sprite;
	int m_destroyTimer;
};

class colorPuyo : public puyo
{
public:
	colorPuyo(int indexX, int indexY, int color, field*, float spriteXreal, float spriteYreal, gameData* globalp);
	colorPuyo(const colorPuyo& self);
	~colorPuyo() {}
	colorPuyo* clone();

	void updateSprite();
	void draw(frendertarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(field* f, bool v);
};

class nuisancePuyo : public puyo
{
public:
	nuisancePuyo(int indexX, int indexY, int color, field*, float spriteXreal, float spriteYreal, gameData* globalp);
	nuisancePuyo(const nuisancePuyo& self);
	~nuisancePuyo() {}
	nuisancePuyo* clone();

	void updateSprite();
	void draw(frendertarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(field* f, bool v);
};

class hardPuyo : public puyo
{
public:
	hardPuyo(int indexX, int indexY, int color, field*, float spriteXreal, float spriteYreal, gameData* globalp);
	hardPuyo(const hardPuyo& self);
	~hardPuyo() {}
	hardPuyo* clone();

	void updateSprite();
	void draw(frendertarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(field* f, bool v);

	int life;
	int vlife;
};

}
