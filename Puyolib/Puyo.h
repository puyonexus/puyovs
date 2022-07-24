#pragma once

#include "global.h"
#include "FieldProp.h"
#include "Sprite.h"
#include <math.h>

namespace ppvs
{

class Field;

// Puyo should be a base class to derive the following classes:
// ColorPuyo, NuisancePuyo, and possibly other ones like for example: PointPuyo, IceCube, BombPuyo, StarPuyo, etc.

enum PuyoType
{
	NOPUYO,
	COLORPUYO,
	NUISANCEPUYO,
	HARDPUYO
};

class Puyo
{
public:
	Puyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* globalp);
	Puyo(const Puyo& self); // Copy constructor
	virtual ~Puyo();
	virtual Puyo* clone();

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
	PuyoType getType()const { return m_type; }

	// Virtual functions
	virtual void draw(FeRenderTarget* target);
	virtual int getColor()const;
	virtual void pop();
	virtual void landProper();
	virtual bool destroyPuyo();
	virtual void neighbourPop(Field* f, bool v);
	virtual void updateSprite();

	// Set animation
	void setLink(direction);
	void unsetLink(direction);
	void bounce();
	void setFallTarget(int target);

	// Public variables
	GameData* data;
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
	PuyoType m_type;
	int m_color;
	bool m_linkDown, m_linkUp, m_linkRight, m_linkLeft;
	Field* m_field;
	Sprite m_sprite;
	int m_destroyTimer;
};

class ColorPuyo : public Puyo
{
public:
	ColorPuyo(int indexX, int indexY, int color, Field*, float spriteXreal, float spriteYreal, GameData* globalp);
	ColorPuyo(const ColorPuyo& self);
	~ColorPuyo() {}
	ColorPuyo* clone();

	void updateSprite();
	void draw(FeRenderTarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(Field* f, bool v);
};

class NuisancePuyo : public Puyo
{
public:
	NuisancePuyo(int indexX, int indexY, int color, Field*, float spriteXreal, float spriteYreal, GameData* globalp);
	NuisancePuyo(const NuisancePuyo& self);
	~NuisancePuyo() {}
	NuisancePuyo* clone();

	void updateSprite();
	void draw(FeRenderTarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(Field* f, bool v);
};

class HardPuyo : public Puyo
{
public:
	HardPuyo(int indexX, int indexY, int color, Field*, float spriteXreal, float spriteYreal, GameData* globalp);
	HardPuyo(const HardPuyo& self);
	~HardPuyo() {}
	HardPuyo* clone();

	void updateSprite();
	void draw(FeRenderTarget* target);
	int getColor() const;
	void pop();
	void landProper();
	bool destroyPuyo();
	void neighbourPop(Field* f, bool v);

	int life;
	int vlife;
};

}
