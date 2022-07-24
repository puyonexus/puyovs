#pragma once

#include "Sprite.h"
#include "global.h"
#include "FieldProp.h"
#include "Frontend.h"
#include <math.h>

namespace ppvs
{

// The particles puyos spit out after popping
class Particle
{
public:
	Particle(float, float, int, GameData*);
	~Particle();
	int getTimer() { return m_timer; }
	void draw(FeRenderTarget*);
	void play();
	bool destroy();

private:
	float m_posX, m_posY, m_speedX;
	float gravityTimer;
	float m_scale;
	int m_timer;
	Sprite m_sprite;
};

// Puyos that are "thrown" out
class ParticleThrow
{
public:
	ParticleThrow(float, float, int, GameData*);
	~ParticleThrow();
	int getTimer() { return m_timer; }
	void draw(FeRenderTarget*);
	void play();
	bool destroy();

private:
	float m_posX, m_posY, m_speedX, m_rotateSpeed;
	float gravityTimer;
	float m_scale, m_rotate;
	int m_timer;
	Sprite m_sprite;
};

// Object that shows the word XX chain
class ChainWord
{
public:
	ChainWord(GameData* gamedata);
	~ChainWord();
	int getTimer() { return m_timer; }
	void setScale(float scale) { m_scale = scale; }
	void draw(FeRenderTarget* rw);
	void showAt(float x, float y, int n);
	void move();

private:
	float m_posX, m_posY, m_scale;
	int m_timer, m_number;
	bool m_visible;
	Sprite m_spriteN1, m_spriteN2, m_spriteChain;
};

// Object that shows the word +XX seconds
class SecondsObject
{
public:
	SecondsObject(GameData*);
	~SecondsObject();
	int getTimer() { return m_timer; }
	void setScale(float scale) { m_scale = scale; }
	void draw(FeRenderTarget* rw);
	void showAt(float x, float y, int n);
	void move();

private:
	float m_posX, m_posY, m_scale;
	int m_timer, m_number;
	bool m_visible;
	Sprite m_spriteN1, m_spriteN2, m_spritePlus;
};

// Light that represents garbage
class LightEffect
{
public:
	LightEffect(GameData* gamedata, PosVectorFloat& startpv, PosVectorFloat& middlepv, PosVectorFloat& endpv);
	~LightEffect();
	float getTimer() { return m_timer; }
	void setTimer(float timer);
	void setStart(PosVectorFloat& pv) { m_start = pv; }
	void setEnd(PosVectorFloat& pv) { m_end = pv; }
	void setMiddle(PosVectorFloat& pv) { m_middle = pv; }
	void draw(FeRenderTarget* rw);

private:
	bool m_visible;
	float m_timer; float m_traileffect;
	PosVectorFloat m_end, m_start, m_middle;
	Sprite m_sprite;
	Sprite m_tail;
	Sprite m_hitlight;
};

// Light that represents a count for the fevergauge
class FeverLight
{
public:
	FeverLight(GameData* gamedata);
	~FeverLight();
	float getTimer() { return m_timer; }
	void setTimer(float timer);
	void init(PosVectorFloat&, PosVectorFloat&, PosVectorFloat&);
	void draw(FeRenderTarget* rw);

private:
	bool m_visible;
	PosVectorFloat m_end, m_start, m_middle;
	float m_timer;
	float m_speed;
	Sprite m_sprite;
	Sprite m_hitLight;
};

// The 6 nuisance puyos above the field
class NuisanceTray
{
public:
	NuisanceTray();
	~NuisanceTray();
	void init(GameData*);
	GameData* gamedata;
	void align(float x, float y, float scale);
	void setVisible(bool);
	void update(int);
	void play();
	void draw();
	void setImage(int sprite, const int image);
	void setDarken(bool d) { darken = d; }

private:
	float m_timer;
	float m_animationTimer[6];
	float m_scale[6];
	float m_globalScale;
	float offsetX;
	float offsetY;
	Sprite m_sprite[6];
	Sprite m_back;
	bool darken;
};

// Score
class ScoreCounter
{
public:
	ScoreCounter();
	~ScoreCounter();
	GameData* gamedata;
	void init(GameData*, float x, float y, float scale);
	void setCounter(int);
	void setPointBonus(int, int);
	void draw();

private:
	int timer;
	int score;
	int point;
	int bonus;
	Sprite m_sprite[9];
	void setImage(int spr, int score);
};

}
