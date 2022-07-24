#pragma once

#include "global.h"
#include "sprite.h"
#include "fieldProp.h"
#include "frontend.h"
#include <math.h>

namespace ppvs
{

class feverCounter
{
public:
	feverCounter(gameData* data);
	~feverCounter();
	int seconds;
	bool fullGauge() { return (m_count == 7); }
	int getCount() { return m_realCount; }
	int getTime() { return seconds; }
	void addCount() { m_realCount++; }
	void setCount(int a) { m_realCount = a; m_count = a; }
	void setTimer(int t) { timer = t; }
	void addTime(int t);
	void setSeconds(int t) { seconds = t; }
	void update();
	void setVisible(bool b) { m_visible = b; }
	void setPositionSeconds(posVectorFloat pv);
	posVectorFloat getPositionSeconds() { return m_positionSeconds; };
	void resetPositionSeconds();

	void init(float x, float y, float scale, bool orientation, gameData* g);
	posVectorFloat getPV();
	void draw();

	bool endless;
	int maxSeconds;

private:
	void setSecondsSprite(int);
	int timer; // Do not confuse timer with seconds
	int updateTimer;
	int m_realCount;
	int m_count;
	float m_scale;
	posVectorFloat m_positionSeconds;
	bool m_init;
	bool m_orientation;
	bool m_visible;
	gameData* data;
	sprite m_sprite[7];
	sprite m_glow[7];
	sprite m_sparkle;
	sprite m_secondsSprite[2];
};

}
