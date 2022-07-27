#pragma once

#include "FieldProp.h"
#include "Sprite.h"
#include "global.h"

namespace ppvs {

class Field;

// Puyo should be a base class to derive the following classes:
// ColorPuyo, NuisancePuyo, and possibly other ones like for example: PointPuyo, IceCube, BombPuyo, StarPuyo, etc.

enum PuyoType {
	NOPUYO,
	COLORPUYO,
	NUISANCEPUYO,
	HARDPUYO
};

class Puyo {
public:
	Puyo(int indexX, int indexY, int color, Field* f, float spriteX, float spriteY, GameData* data);
	Puyo(const Puyo& self); // Copy constructor
	virtual ~Puyo();
	virtual Puyo* clone();

	// Get and set
	[[nodiscard]] int posX() const { return m_posX; } // Returns indexed position
	void setPosX(const int val) { m_posX = val; }
	[[nodiscard]] int posY() const { return m_posY; }
	void setPosY(const int val) { m_posY = val; }
	[[nodiscard]] float spriteX() const { return m_spriteX; } // Returns global position
	void setSpriteX(const float val) { m_spriteX = val; }
	[[nodiscard]] float spriteY() const { return m_spriteY; }
	void setSpriteY(const float val) { m_spriteY = val; }
	[[nodiscard]] float scaleX() const { return m_scaleX; }
	void setScaleX(const float val) { m_scaleX = val; }
	[[nodiscard]] float scaleY() const { return m_scaleY; }
	void setScaleY(const float val) { m_scaleY = val; }
	[[nodiscard]] float targetY() const { return m_targetY; }
	void setTargetY(const float val) { m_targetY = val; }
	[[nodiscard]] float accelerationY() const { return m_accelerationY; }
	void setAccelerationY(const float val) { m_accelerationY = val; }
	void addAccelerationY(float val);
	void setFallDelay(const int val) { m_fallDelay = static_cast<float>(val); }
	[[nodiscard]] PuyoType getType() const { return m_type; }

	// Virtual functions
	virtual void draw(FeRenderTarget* target);
	[[nodiscard]] virtual int getColor() const;
	virtual void pop();
	virtual void landProper();
	virtual bool destroyPuyo();
	virtual void neighborPop(Field* f, bool v);
	virtual void updateSprite();

	// Set animation
	void setLink(Direction);
	void unsetLink(Direction);
	void bounce();
	void setFallTarget(int target);

	// Public variables
	GameData* m_data = nullptr;
	float m_fallDelay = 0.f;
	int m_fallFlag = 0;
	bool m_bounceFlag0 = false; // Flag 0
	int m_bounceFlag = 0, m_searchBounce = 0;
	float m_bounceTimer = 0.f;
	float m_bounceMultiplier = 0.f;
	float m_bounceY = 0.f;
	int m_bottomY = 0;
	bool m_glow = false;
	bool m_hard = false;
	bool m_droppable = false;
	bool m_lastNuisance = false;
	bool m_mark = false;
	bool m_destroy = false;
	bool m_temporary = false;

protected:
	int m_posX = 0, m_posY = 0; // Position in the field
	float m_spriteX = 0.f, m_spriteY = 0.f, m_spriteYsc = 0.f; // Position on the screen (MMF legacy)
	float m_scaleX = 0.f, m_scaleY = 0.f, m_scaleYcor = 0.f; // Standard scaling
	float m_scaleXd = 0.f, m_scaleYd = 0.f; // Scaling factor for destruction
	float m_accelerationY = 0.f, m_targetY = 0.f; // Acceleration factor
	PuyoType m_type = NOPUYO;
	int m_color = 0;
	bool m_linkDown = false, m_linkUp = false, m_linkRight = false, m_linkLeft = false;
	Field* m_field = nullptr;
	Sprite m_sprite {};
	int m_destroyTimer = 0;
};

class ColorPuyo final : public Puyo {
public:
	ColorPuyo(int indexX, int indexY, int color, Field*, float spriteX, float spriteY, GameData* data);
	ColorPuyo(const ColorPuyo& self);
	~ColorPuyo() override = default;
	ColorPuyo* clone() override;

	void updateSprite() override;
	void draw(FeRenderTarget* target) override;
	[[nodiscard]] int getColor() const override;
	void pop() override;
	void landProper() override;
	bool destroyPuyo() override;
	void neighborPop(Field* f, bool v) override;
};

class NuisancePuyo final : public Puyo {
public:
	NuisancePuyo(int indexX, int indexY, int color, Field*, float spriteX, float spriteY, GameData* data);
	NuisancePuyo(const NuisancePuyo& self);
	~NuisancePuyo() override = default;
	NuisancePuyo* clone() override;

	void updateSprite() override;
	void draw(FeRenderTarget* target) override;
	[[nodiscard]] int getColor() const override;
	void pop() override;
	void landProper() override;
	bool destroyPuyo() override;
	void neighborPop(Field* f, bool v) override;
};

class HardPuyo final : public Puyo {
public:
	HardPuyo(int indexX, int indexY, int color, Field*, float spriteX, float spriteY, GameData* data);
	HardPuyo(const HardPuyo& self);
	~HardPuyo() override = default;
	HardPuyo* clone() override;

	void updateSprite() override;
	void draw(FeRenderTarget* target) override;
	[[nodiscard]] int getColor() const override;
	void pop() override;
	void landProper() override;
	bool destroyPuyo() override;
	void neighborPop(Field* f, bool v) override;

	int m_life;
	int m_vlife;
};

}
