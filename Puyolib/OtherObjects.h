#pragma once

#include "FieldProp.h"
#include "Frontend.h"
#include "Sprite.h"
#include "global.h"

namespace ppvs {

// The particles puyos spit out after popping
class Particle {
public:
	Particle(float, float, int, const GameData*);
	~Particle();

	Particle(const Particle&) = delete;
	Particle& operator=(const Particle&) = delete;
	Particle(Particle&&) = delete;
	Particle& operator=(Particle&&) = delete;

	[[nodiscard]] int getTimer() const { return m_timer; }
	void draw(FeRenderTarget*);
	void play();
    [[nodiscard]] bool shouldDestroy() const;

private:
	float m_posX, m_posY, m_speedX;
	float m_gravityTimer;
	float m_scale;
	int m_timer;
	Sprite m_sprite;
};

// Puyos that are "thrown" out
class ParticleThrow {
public:
	ParticleThrow(float, float, int, const GameData*);
	~ParticleThrow();

	ParticleThrow(const ParticleThrow&) = delete;
	ParticleThrow& operator=(const ParticleThrow&) = delete;
	ParticleThrow(ParticleThrow&&) = delete;
	ParticleThrow& operator=(ParticleThrow&&) = delete;

	[[nodiscard]] int getTimer() const { return m_timer; }
	void draw(FeRenderTarget*);
	void play();
    [[nodiscard]] bool shouldDestroy() const;

private:
	float m_posX = 0.f, m_posY = 0.f, m_speedX = 0.f, m_rotateSpeed = 0.f;
	float m_gravityTimer = 0.f;
	float m_scale = 1.f, m_rotate = 0.;
	int m_timer = 0;
	Sprite m_sprite {};
};

// Object that shows the word XX chain
class ChainWord {
public:
	explicit ChainWord(const GameData* data);
	~ChainWord();

	ChainWord(const ChainWord&) = delete;
	ChainWord& operator=(const ChainWord&) = delete;
	ChainWord(ChainWord&&) = delete;
	ChainWord& operator=(ChainWord&&) = delete;

	[[nodiscard]] int getTimer() const { return m_timer; }
	void setScale(const float scale) { m_scale = scale; }
	void draw(FeRenderTarget* rw);
	void showAt(float x, float y, int n);
	void move();

private:
	float m_posX = 0.f, m_posY = 0.f, m_scale = 1.f;
	int m_timer = 100, m_number = 0;
	bool m_visible = false;
	Sprite m_spriteN1 {}, m_spriteN2 {}, m_spriteChain {};
};

// Object that shows the word +XX seconds
class SecondsObject {
public:
	explicit SecondsObject(const GameData*);
	~SecondsObject();

	SecondsObject(const SecondsObject&) = delete;
	SecondsObject& operator=(const SecondsObject&) = delete;
	SecondsObject(SecondsObject&&) = delete;
	SecondsObject& operator=(SecondsObject&&) = delete;

	[[nodiscard]] int getTimer() const { return m_timer; }
	void setScale(const float scale) { m_scale = scale; }
	void draw(FeRenderTarget* rw);
	void showAt(float x, float y, int n);
	void move();

private:
	float m_posX = 0.f, m_posY = 0.f, m_scale = 1.f;
	int m_timer = 100, m_number = 0;
	bool m_visible = false;
	Sprite m_spriteN1 {}, m_spriteN2 {}, m_spritePlus {};
};

// Light that represents garbage
class LightEffect {
public:
	LightEffect(const GameData* data, const PosVectorFloat& startPv, const PosVectorFloat& middlePv, const PosVectorFloat& endPv);
	~LightEffect();

	LightEffect(const LightEffect&) = delete;
	LightEffect& operator=(const LightEffect&) = delete;
	LightEffect(LightEffect&&) = delete;
	LightEffect& operator=(LightEffect&&) = delete;

	[[nodiscard]] float getTimer() const { return m_timer; }
	void setTimer(float timer);
	void setStart(const PosVectorFloat& pv) { m_start = pv; }
	void setEnd(const PosVectorFloat& pv) { m_end = pv; }
	void setMiddle(const PosVectorFloat& pv) { m_middle = pv; }
	void draw(FeRenderTarget* rw);

private:
	bool m_visible = false;
	float m_timer = 0.f;
	PosVectorFloat m_end {}, m_start {}, m_middle {};
	Sprite m_sprite {};
	Sprite m_tail {};
	Sprite m_hitLight {};
};

// Light that represents a count for the fever gauge
class FeverLight {
public:
    explicit FeverLight(const GameData* data);
	~FeverLight();

	FeverLight(const FeverLight&) = delete;
	FeverLight& operator=(const FeverLight&) = delete;
	FeverLight(FeverLight&&) = delete;
	FeverLight& operator=(FeverLight&&) = delete;

	[[nodiscard]] float getTimer() const { return m_timer; }
	void setTimer(float timer);
	void init(const PosVectorFloat&, const PosVectorFloat&, const PosVectorFloat&);
	void draw(FeRenderTarget* rw);

private:
	bool m_visible = false;
	PosVectorFloat m_end {}, m_start {}, m_middle {};
	float m_timer = 0.f;
	float m_speed = 0.f;
	Sprite m_sprite {};
	Sprite m_hitLight {};
};

// The 6 nuisance puyos above the field
class NuisanceTray {
public:
	NuisanceTray();
	~NuisanceTray();

	NuisanceTray(const NuisanceTray&) = delete;
	NuisanceTray& operator=(const NuisanceTray&) = delete;
	NuisanceTray(NuisanceTray&&) = delete;
	NuisanceTray& operator=(NuisanceTray&&) = delete;

	void init(GameData*);
	GameData* m_data = nullptr;
	void align(float x, float y, float scale);
	void setVisible(bool);
	void update(int);
	void play();
	void draw();
	void setImage(int sprite, int image);
	void setDarken(const bool d) { m_darken = d; }

private:
	float m_timer = 0.f;
	float m_animationTimer[6] {};
	float m_globalScale = 0.f;
	float m_offsetX = 0.f;
	float m_offsetY = 0.f;
	Sprite m_sprite[6] {};
	Sprite m_back {};
	bool m_darken = false;
};

// Score
class ScoreCounter {
public:
	ScoreCounter();
	~ScoreCounter();

	ScoreCounter(const ScoreCounter&) = delete;
	ScoreCounter& operator=(const ScoreCounter&) = delete;
	ScoreCounter(ScoreCounter&&) = delete;
	ScoreCounter& operator=(ScoreCounter&&) = delete;

	GameData* m_data = nullptr;
	void init(GameData*, float x, float y, float scale);
	void setCounter(int);
	void setPointBonus(int, int);
	void draw();

private:
	int m_timer = 0;
	int m_score = 0;
	int m_point = 0;
	int m_bonus = 0;
	Sprite m_sprite[9] {};

	void setImage(int spr, int score);
};

}
