#pragma once

#include "FieldProp.h"
#include "Sprite.h"
#include "global.h"

namespace ppvs {

class FeverCounter {
public:
	explicit FeverCounter(const GameData* gameData);

	[[nodiscard]] bool fullGauge() const { return (m_count == 7); }
	[[nodiscard]] int getCount() const { return m_realCount; }
	[[nodiscard]] int getTime() const { return m_seconds; }
	void addCount() { m_realCount++; }
	void setCount(const int a)
	{
		m_realCount = a;
		m_count = a;
	}
	void setTimer(const int t) { m_timer = t; }
	void addTime(int t);
	void setSeconds(const int t) { m_seconds = t; }
	void update();
	void setVisible(const bool b) { m_visible = b; }
	void setPositionSeconds(PosVectorFloat pv);
	[[nodiscard]] PosVectorFloat getPositionSeconds() const { return m_positionSeconds; }
	void resetPositionSeconds();

	void init(float x, float y, float scale, bool orientation, GameData* g);
	[[nodiscard]] PosVectorFloat getPos() const;
	void draw();

	bool m_endless = false;
	int m_seconds = 60 * 15;
	int m_maxSeconds = 30;

private:
    void setSecondsSprite(int);

    int m_timer = 0; // Do not confuse timer with seconds
	int m_updateTimer = 0;
	int m_count = 0;
	int m_realCount = 0;
	float m_scale = 0.f;
	PosVectorFloat m_positionSeconds = { 0, 0 };
	bool m_init = false;
	bool m_orientation = false;
	bool m_visible = false;
	GameData* m_data = nullptr;
	Sprite m_sprite[7];
	Sprite m_glow[7];
	Sprite m_sparkle;
	Sprite m_secondsSprite[2];
};

}
